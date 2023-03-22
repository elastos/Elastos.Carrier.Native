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
#include <list>
#include <thread>
#include <cassert>

#include "activeproxy.h"
#include "connection.h"
#include "exceptions.h"

namespace elastos {
namespace carrier {
namespace activeproxy {

using Logger = elastos::carrier::Logger;

static std::shared_ptr<Logger> log = Logger::get("AcriveProxy");
static const uint32_t IDLE_CHECK_INTERVAL = 60000;  // 1 minute
static const uint32_t MAX_IDLE_TIME = 300000;       // 3 minutes

ActiveProxy::ActiveProxy(const Node& node, const Id& serverId,
        const std::string& serverHost, uint16_t serverPort,
        const std::string& upstreamHost, uint16_t upstreamPort)
        : node(node), serverId(serverId),
          serverHost(serverHost), serverPort(serverPort),
          upstreamHost(upstreamHost), upstreamPort(upstreamPort)
{
    assert(!serverHost.empty() && serverPort != 0);
    assert(!upstreamHost.empty() && upstreamPort != 0);

    log->setLevel(Level::Info);

    auto addrs = SocketAddress::resolve(serverHost, serverPort);
    serverAddr = addrs[0];

    addrs = SocketAddress::resolve(upstreamHost, upstreamPort);
    upstreamAddr = addrs[0];

    serverName.reserve(serverHost.length() + 8);
    upstreamName.reserve(upstreamHost.length() + 8);

    serverName.append(serverHost).append(":").append(std::to_string(serverPort));
    upstreamName.append(upstreamHost).append(":").append(std::to_string(upstreamPort));
}

void ActiveProxy::onStop() noexcept
{
    log->info("ActiveProxy stopping...");
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
    for (auto c : connections) {
        c->onClosed(nullptr);
        c->close();
        c->unref();
    }

    connections.clear();
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
    if (needsNewConnection()) {
        connect();
    }
}

void ActiveProxy::idleCheck() noexcept
{
    // Dump the current status: should change the log level to debug later
    log->info("STATUS: Connections = {}, inFlights = {}, idle = {}",
            connections.size(), inFlights,
            idleTimestamp == MAX_IDLE_TIME ? 0 : (uv_now(&loop) - idleTimestamp) / 1000);
    for (auto c : connections)
        log->info("STATUS: {}", c->status());

    if (idleTimestamp == UINT64_MAX)
        return;

    if ((uv_now(&loop) - idleTimestamp) < MAX_IDLE_TIME)
        return;

    if (inFlights != 0 || connections.size() <= 1)
        return;

    log->info("ActiveProxy closing the redundant connections due to long time idle...");
    for (auto c = connections.end() - 1; c > connections.begin(); --c) {
        (*c)->onClosed(nullptr);
        (*c)->close();
        (*c)->unref();
    }

    connections.resize(1);
}

void ActiveProxy::start()
{
    log->info("ActiveProxy starting...");

    int rc = uv_loop_init(&loop);
    if (rc < 0) {
        log->error("ActiveProxy failed to initialize the event loop({}): {}", rc, uv_strerror(rc));
        throw networking_error(uv_strerror(rc));
    }

    // init the stop handle
    rc = uv_async_init(&loop, &stopHandle, [](uv_async_t* handle) {
        ActiveProxy* ap = (ActiveProxy*)handle->data;
        ap->onStop();
    });
    stopHandle.data = this;
    if (rc < 0) {
        log->error("ActiveProxy failed to initialize the stop handle({}): {}", rc, uv_strerror(rc));
        uv_loop_close(&loop);
        throw networking_error(uv_strerror(rc));
    }

    // init the idle/iteration handle
    uv_idle_init(&loop, &idleHandle); // always success
    idleHandle.data = this;
    uv_idle_start(&idleHandle, [](uv_idle_t* handle) {
        ActiveProxy* ap = (ActiveProxy*)handle->data;
        ap->onIteration();
    });
    if (rc < 0) {
        log->error("ActiveProxy failed to start the iteration handle({}): {}", rc, uv_strerror(rc));
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
        log->error("ActiveProxy failed to start the idle check timer({}): {}", rc, uv_strerror(rc));
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
        log->info("ActiveProxy started.");
        running = true;
        rc = uv_run(&loop, UV_RUN_DEFAULT);
        if (rc < 0) {
            log->error("ActiveProxy failed to start the event loop({}): {}", rc, uv_strerror(rc));
            running = false;
            uv_idle_stop(&idleHandle);
            uv_close((uv_handle_t*)&idleHandle, nullptr);
            uv_close((uv_handle_t*)&stopHandle, nullptr);
            uv_timer_stop(&idleCheckTimer);
            uv_close((uv_handle_t*)&idleCheckTimer, nullptr);
            uv_close((uv_handle_t*)&reconnectTimer, nullptr);
            uv_loop_close(&loop);
            throw networking_error(uv_strerror(rc));
        }

        running = false;
        uv_loop_close(&loop);
        log->info("ActiveProxy stopped.");
    });
}

void ActiveProxy::stop() noexcept
{
    if (running) {
        log->info("ActiveProxy stopping...");
        uv_async_send(&stopHandle);

        runner.join();
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
    log->info("ActiveProxy try to reconnect after {} seconeds.", reconnectInterval / 1000);
    reconnectTimer.data = this;
    auto rc = uv_timer_start(&reconnectTimer, [](uv_timer_t* handle) {
        ActiveProxy* self = (ActiveProxy*)handle->data;
        handle->data = nullptr; // For timer status checking
        uv_timer_stop(handle);
        self->_connect();
    }, reconnectInterval, 0);
    if (rc < 0) {
        log->error("ActiveProxy failed to start the reconnect timer({}): {}", rc, uv_strerror(rc));
        reconnectInterval = 0;
    }
}

void ActiveProxy::_connect() noexcept
{
    log->debug("ActiveProxy try to create a new connectoin.");

    ProxyConnection* connection = new ProxyConnection {*this};
    connections.push_back(connection);

    connection->onAuthorized([this](ProxyConnection* c, const CryptoBox::PublicKey& serverPk,
            const CryptoBox::Nonce& nonce, uint16_t port) {
        this->serverPk = serverPk;
        this->nonce = nonce;
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
