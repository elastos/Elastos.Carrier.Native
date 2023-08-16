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
static const uint32_t IDLE_CHECK_INTERVAL = 60000;  // 1 minute
static const uint32_t MAX_IDLE_TIME = 300000;       // 3 minutes

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
    if (upstreamHost == "LOCAL-IP4-ADDRESS")
        upstreamHost = getLocalIPv4();
    else if (upstreamHost == "LOCAL-IP6-ADDRESS")
        upstreamHost = getLocalIPv6();
    upstreamPort = (uint16_t)std::any_cast<int64_t>(configure.at("upstreamPort"));
    if (upstreamHost.empty() || upstreamPort == 0)
        throw std::invalid_argument("Addon ActiveProxy's configure item has error: empty upstreamHost or upstreamPort is not allowed");

    if (configure.count("serverPeerId")) {
        std::string strId = std::any_cast<std::string>(configure.at("serverPeerId"));

        log->info("Addon ActiveProxy finding peer...");
        auto future = node->findPeer(Id(strId), 1);
        auto peers = future.get();
        if (peers.empty())
            throw std::invalid_argument("Addon ActiveProxy can't find peer: " + strId + "!");

        auto peer = peers.front();
        serverPort = peer.getPort();
        serverId = peer.getNodeId();

        log->info("Addon ActiveProxy finding node...");
        auto future2 = node->findNode(serverId);
        auto nodes = future2.get();
        if (nodes.empty())
            throw std::invalid_argument("Addon ActiveProxy can't find node: " + serverId.toBase58String() + "!");
        auto ni = nodes.front();
        serverHost = ni->getAddress().host();
    }
    else if (configure.count("serverId") && configure.count("serverHost") && configure.count("serverPort")) {
        std::string strId = std::any_cast<std::string>(configure.at("serverId"));
        serverId = Id(strId);
        serverHost = std::any_cast<std::string>(configure.at("serverHost"));
        serverPort = (uint16_t)std::any_cast<int64_t>(configure.at("serverPort"));
    }
    else {
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

    if (reconnectTimer.data)
        uv_timer_stop(&reconnectTimer);
    uv_close((uv_handle_t*)&reconnectTimer, nullptr);

    uv_timer_stop(&idleCheckTimer);
    uv_close((uv_handle_t*)&idleCheckTimer, nullptr);

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

    // connecting or reconnecting
    if (reconnectTimer.data) // TODO: checkme!!! ugly timer status checking
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
}

void ActiveProxy::idleCheck() noexcept
{
    // Dump the current status: should change the log level to debug later
    log->info("Addon ActiveProxy STATUS dump: Connections = {}, inFlights = {}, idle = {}",
            connections.size(), inFlights,
            idleTimestamp == MAX_IDLE_TIME ? 0 : (uv_now(&loop) - idleTimestamp) / 1000);
    for (const auto& c: connections)
        log->info("Addon ActiveProxy STATUS dump: {}", c->status());

    if (idleTimestamp == UINT64_MAX)
        return;

    if ((uv_now(&loop) - idleTimestamp) < MAX_IDLE_TIME)
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

    uv_timer_init(&loop, &idleCheckTimer); // always success
    idleCheckTimer.data = this;
    rc = uv_timer_start(&idleCheckTimer, [](uv_timer_t* handle) {
        ActiveProxy* self = (ActiveProxy*)handle->data;
        self->idleCheck();
    }, IDLE_CHECK_INTERVAL, IDLE_CHECK_INTERVAL);
    if (rc < 0) {
        log->error("Addon ActiveProxy failed to start the idle check timer({}): {}", rc, uv_strerror(rc));
        uv_idle_stop(&idleHandle);
        uv_close((uv_handle_t*)&idleHandle, nullptr);
        uv_close((uv_handle_t*)&stopHandle, nullptr);
        uv_close((uv_handle_t*)&idleCheckTimer, nullptr);
        uv_loop_close(&loop);
        throw networking_error(uv_strerror(rc));
    }

    uv_timer_init(&loop, &reconnectTimer); // always success

    idleTimestamp = UINT64_MAX;

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
            uv_timer_stop(&idleCheckTimer);
            uv_close((uv_handle_t*)&idleCheckTimer, nullptr);
            uv_close((uv_handle_t*)&reconnectTimer, nullptr);
            uv_loop_close(&loop);
            try {
                throw networking_error(uv_strerror(rc));
            } catch(...) {
                startPromise.set_exception(std::current_exception());
            }
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

    if (serverFails == 0) {
        _connect();
        return;
    }

    reconnectInterval = (1 << (serverFails <= 6 ? serverFails : 6)) * 1000;
    log->info("Addon ActiveProxy tried to reconnect after {} seconeds.", reconnectInterval / 1000);
    reconnectTimer.data = this;
    auto rc = uv_timer_start(&reconnectTimer, [](uv_timer_t* handle) {
        ActiveProxy* self = (ActiveProxy*)handle->data;
        handle->data = nullptr; // For timer status checking
        uv_timer_stop(handle);
        self->_connect();
    }, reconnectInterval, 0);
    if (rc < 0) {
        log->error("Addon ActiveProxy failed to start the reconnect timer({}): {}", rc, uv_strerror(rc));
        reconnectInterval = 0;
    }
}

void ActiveProxy::_connect() noexcept
{
    log->debug("Addon ActiveProxy tried to create a new connectoin.");

    ProxyConnection* connection = new ProxyConnection {*this};
    connections.push_back(connection);

    connection->onAuthorized([this](ProxyConnection* c, const CryptoBox::PublicKey& serverPk, uint16_t port) {
        this->serverPk = serverPk;
        this->relayPort = port;

        this->box = CryptoBox{serverPk, this->sessionKey.privateKey() };
    });

    connection->onOpened([this](ProxyConnection* c) {
        serverFails = 0;
    });

    connection->onOpenFailed([this](ProxyConnection* c) {
        serverFails++;
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

    if (connection->connectServer() < 0)  {
        serverFails++;
        connections.pop_back();
        connection->unref();
    }
}

} // namespace activeproxy
} // namespace carrier
} // namespace elastos
