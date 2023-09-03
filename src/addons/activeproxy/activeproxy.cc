/*
 * Copyright (c) 2022 trinity-tech.io
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <uv.h>

#include <memory>
#include <algorithm>
#include <thread>
#include <cassert>

#include "activeproxy.h"
#include "connection.h"
#include "exceptions.h"
#include "utils/log.h"
#include "utils/addr.h"
#include "crypto/hex.h"

namespace elastos {
namespace carrier {
namespace activeproxy {

using Logger = elastos::carrier::Logger;

static std::shared_ptr<Logger> log;

static const uint32_t IDLE_CHECK_INTERVAL = 60000;  // 60 seconds
static const uint32_t MAX_IDLE_TIME = 300000;       // 3 minutes
static const uint32_t RE_ANNOUNCE_INTERVAL = 60 * 60 * 1000; // 1 hour
static const uint32_t HEALTH_CHECK_INTERVAL = 10000;  // 10 seconds

std::future<void> ActiveProxy::initialize(Sp<Node> node, const std::map<std::string, std::any>& configure) {
    log = Logger::get("AcriveProxy");

    if (configure.count("logLevel")) {
        logLevel = std::any_cast<std::string>(configure.at("logLevel"));
        log->setLevel(logLevel);
    }

    if (!configure.count("upstreamHost"))
        throw std::invalid_argument("Addon ActiveProxy's configure item has error: missing upstreamHost!");

    if (!configure.count("upstreamPort"))
        throw std::invalid_argument("Addon ActiveProxy's configure item has error: missing upstreamPort!");

    upstreamHost = std::any_cast<std::string>(configure.at("upstreamHost"));
    upstreamPort = (uint16_t)std::any_cast<int64_t>(configure.at("upstreamPort"));
    if (upstreamHost.empty() || upstreamPort == 0)
        throw std::invalid_argument("Addon ActiveProxy's configure item has error: empty upstreamHost or upstreamPort is not allowed");

    if (configure.count("serverPeerId")) {
        auto id = std::any_cast<std::string>(configure.at("serverPeerId"));
        auto peerId = Id(id);

        log->info("Addon ActiveProxy finding peer {} ...", id);
        auto future = node->findPeer(peerId, 8);
        auto peers = future.get();
        if (peers.empty())
            throw std::invalid_argument("Addon ActiveProxy can't find a server peer: " + id + "!");

        bool found = false;
        for (auto peer : peers) {
            serverPort = peer.getPort();
            serverId = peer.getNodeId();

            log->info("Addon ActiveProxy finding node...");
            auto future2 = node->findNode(serverId);
            auto nodes = future2.get();
            if (nodes.empty())
                throw std::invalid_argument("Addon ActiveProxy can't find node: " + serverId.toString() + "!");

            for (auto node : nodes) {
                serverHost = node->getAddress().host();

                // TODO: check the service availability
                found = true;
                break;
            }

            if (found)
                break;
        }

        if (!found)
            throw std::invalid_argument("Addon ActiveProxy can't find available service for peer: " + id + "!");
    } else if (configure.count("serverId") && configure.count("serverHost") && configure.count("serverPort")) {
        // TODO: to be remove
        std::string id = std::any_cast<std::string>(configure.at("serverId"));
        serverId = Id(id);
        serverHost = std::any_cast<std::string>(configure.at("serverHost"));
        serverPort = (uint16_t)std::any_cast<int64_t>(configure.at("serverPort"));
    } else {
        throw std::invalid_argument("Addon ActiveProxy's configure item has error: missing serverPeerId!");
    }

    if (serverHost.empty() || serverPort == 0)
        throw std::runtime_error("Addon ActiveProxy's configure item has error: empty serverHost or serverPort is not allowed");

    if (configure.count("peerPrivateKey")) {
        std::string sk = std::any_cast<std::string>(configure.at("peerPrivateKey"));
        peerKeypair = Signature::KeyPair::fromPrivateKey(Hex::decode(sk));
    }

    if (configure.count("domainName"))
        domainName = std::any_cast<std::string>(configure.at("domainName"));

    if (configure.count("maxConnections"))
        maxConnections = (uint32_t)std::any_cast<int64_t>(configure.at("maxConnections"));

    //init data
    this->node = node;

    auto addrs = SocketAddress::resolve(serverHost, serverPort);
    serverAddr = addrs[0];

    addrs = SocketAddress::resolve(upstreamHost, upstreamPort);
    upstreamAddr = addrs[0];

    serverName.reserve(serverHost.length() + 8);
    upstreamName.reserve(upstreamHost.length() + 8);

    serverName.append(serverHost).append(":").append(std::to_string(serverPort));
    upstreamName.append(upstreamHost).append(":").append(std::to_string(upstreamPort));

    //start
    startPromise = std::promise<void>();
    start();
    return startPromise.get_future();
}

std::future<void> ActiveProxy::deinitialize() {
    stopPromise = std::promise<void>();
    stop();
    return stopPromise.get_future();
}

void ActiveProxy::onStop() noexcept
{
    log->info("Addon ActiveProxy is on-stopping...");
    running = false;

    uv_idle_stop(&idleHandle);
    uv_close((uv_handle_t*)&idleHandle, nullptr);

    uv_close((uv_handle_t*)&stopHandle, nullptr);

    // close all connections
    for (const auto& c : connections) {
        c->onClosed(nullptr);
        c->close();
        c->unref();
    }

    connections.clear();
    stopPromise.set_value();
}

bool ActiveProxy::needsNewConnection() const noexcept
{
    if (connections.size() >= maxConnections)
        return false;

    // reconnect delay after connect to server failed
    if (reconnectDelay && uv_now(&loop) - lastConnectTimestamp < reconnectDelay)
        return false;

    if (connections.empty() || inFlights == connections.size())
        return true;

    // TODO: other conditions?

    return false;
}

void ActiveProxy::onIteration() noexcept
{
    if (first) {
        startPromise.set_value();
        first = false;
    }

    if (needsNewConnection()) {
        connect();
    }

    auto now = uv_now(&loop);
    if (now - lastIdleCheckTimestamp >= IDLE_CHECK_INTERVAL) {
        lastIdleCheckTimestamp = now;
        idleCheck();
    }

    if (now - lastHealthCheckTimestamp >= HEALTH_CHECK_INTERVAL) {
        lastHealthCheckTimestamp = now;
        healthCheck();
    }

    if (peer.has_value() && now - lastAnnouncePeerTimestamp >= RE_ANNOUNCE_INTERVAL) {
        lastAnnouncePeerTimestamp = now;
        announcePeer();
    }
}

void ActiveProxy::idleCheck() noexcept
{
    if (connections.size() == 0 && serverPk.has_value()) {
        serverPk.reset();
        return;
    }

    auto now = uv_now(&loop);

    // Dump the current status: should change the log level to debug later
    log->info("Addon ActiveProxy STATUS dump: Connections = {}, inFlights = {}, idle = {}",
            connections.size(), inFlights,
            idleTimestamp == UINT64_MAX ? 0 : (now - idleTimestamp) / 1000);

    for (const auto& c: connections)
        log->info("Addon ActiveProxy STATUS dump: {}", c->status());

    if (idleTimestamp == UINT64_MAX || (now - idleTimestamp) < MAX_IDLE_TIME)
        return;

    if (inFlights != 0 || connections.size() <= 1)
        return;

    log->info("Addon ActiveProxy is closing the redundant connections due to long time idle...");
    for (auto c = connections.end() - 1; c > connections.begin(); --c) {
        (*c)->onClosed(nullptr);
        (*c)->close();
        (*c)->unref();
    }

    connections.resize(1);
}

void ActiveProxy::healthCheck() noexcept
{
    for (const auto& c: connections)
        c->periodicCheck();
}

void ActiveProxy::start()
{
    log->info("Addon ActiveProxy is starting...");

    int rc = uv_loop_init(&loop);
    if (rc < 0) {
        log->error("Addon ActiveProxy failed to initialize the event loop({}): {}", rc, uv_strerror(rc));
        throw networking_error(uv_strerror(rc));
    }

    // init the stop handle
    rc = uv_async_init(&loop, &stopHandle, [](uv_async_t* handle) {
        ActiveProxy* ap = (ActiveProxy*)handle->data;
        ap->onStop();
    });
    if (rc < 0) {
        log->error("Addon ActiveProxy failed ot initialize the stop handle({}): {}", rc, uv_strerror(rc));
        uv_loop_close(&loop);
        throw networking_error(uv_strerror(rc));
    }
    stopHandle.data = this;

    // init the idle/iteration handle
    uv_idle_init(&loop, &idleHandle); // always success
    idleHandle.data = this;
    rc = uv_idle_start(&idleHandle, [](uv_idle_t* handle) {
        ActiveProxy* ap = (ActiveProxy*)handle->data;
        ap->onIteration();
    });
    if (rc < 0) {
        log->error("Addon ActiveProxy failed to start the iteration handle({}): {}", rc, uv_strerror(rc));
        uv_close((uv_handle_t*)&idleHandle, nullptr);
        uv_close((uv_handle_t*)&stopHandle, nullptr);
        uv_loop_close(&loop);
        throw networking_error(uv_strerror(rc));
    }

    auto now = uv_now(&loop);
    lastIdleCheckTimestamp = now;
    lastHealthCheckTimestamp = now;

    // Start the loop in thread
    runner = std::thread([&]() {
        log->info("Addon ActiveProxy is running.");
        running = true;
        first = true;
        int rc = uv_run(&loop, UV_RUN_DEFAULT);
        if (rc < 0) {
            log->error("Addon ActiveProxy failed to start the event loop({}): {}", rc, uv_strerror(rc));
            running = false;
            uv_idle_stop(&idleHandle);
            uv_close((uv_handle_t*)&idleHandle, nullptr);
            uv_close((uv_handle_t*)&stopHandle, nullptr);
            uv_loop_close(&loop);

            auto exp = std::make_exception_ptr(networking_error(uv_strerror(rc)));
            startPromise.set_exception(exp);
        }

        running = false;
        uv_loop_close(&loop);
        log->info("Addon ActiveProxy is stopped.");
    });
}

void ActiveProxy::stop() noexcept
{
    if (running) {
        log->info("Addon ActiveProxy is stopping...");
        uv_async_send(&stopHandle);
        try {
            runner.join();
        } catch(...) {
        }
    } else {
        stopPromise.set_value();
    }
}

void ActiveProxy::connect() noexcept
{
    assert(running);

    log->debug("Addon ActiveProxy tried to create a new connectoin.");

    ProxyConnection* connection = new ProxyConnection {*this};
    connections.push_back(connection);

    connection->onAuthorized([this](ProxyConnection* c, const CryptoBox::PublicKey& serverPk, uint16_t port, bool domainEnabled) {
        this->serverPk = serverPk;
        this->relayPort = port;

        this->box = CryptoBox{serverPk, this->sessionKey.privateKey() };

        if (peerKeypair.has_value()) {
            std::string domain = domainEnabled ? domainName : "";
            peer = PeerInfo::create(peerKeypair.value(), serverId, node->getId(), port, domain);
            // will announce the peer in the next libuv iteration
        }
    });

    connection->onOpened([this](ProxyConnection* c) {
        serverFails = 0;
        reconnectDelay = 0;
    });

    connection->onOpenFailed([this](ProxyConnection* c) {
        serverFails++;
        if (reconnectDelay < 64)
            reconnectDelay = (1 << serverFails) * 1000;
    });

    connection->onClosed([this](ProxyConnection* c) {
        auto it = connections.begin();

        for (; it != connections.end(); ++it) {
            if (*it == c)
                break;
        }

        if (it != connections.end()) {
            connections.erase(it);
        }

        c->unref();
    });

    connection->onBusy([this](ProxyConnection* c) {
        ++inFlights;
        idleTimestamp = UINT64_MAX;
    });

    connection->onIdle([this](ProxyConnection* c) {
        if (--inFlights == 0)
            idleTimestamp = uv_now(&loop);
    });

    lastConnectTimestamp = uv_now(&loop);
    connection->connectServer();
}

void ActiveProxy::announcePeer() noexcept
{
    if (!peer.has_value())
        return;

    log->info("Announce peer {} : {}", peer.value().getId().toBase58String(),
        peer.value().toString());

    node->announcePeer(peer.value());
}

} // namespace activeproxy
} // namespace carrier
} // namespace elastos
