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

#pragma once

#include <uv.h>

#include <functional>

#include "carrier.h"

namespace elastos {
namespace carrier {

namespace activeproxy {

class ActiveProxy;

enum class ConnectionState {
    Connecting = 0,
    Initializing,
    Authenticating,
    Attaching,
    Idling,
    Relaying,
    Closed
};

class ProxyConnection {
public:
    using AuthorizedHandle = std::function<void(ProxyConnection*,
            const CryptoBox::PublicKey&, uint16_t)>;
    using StatusHandler = std::function<void(ProxyConnection*)>;

    ProxyConnection(ActiveProxy& server) noexcept;
    ~ProxyConnection() noexcept;

    int connectServer() noexcept;
    void close() noexcept;

    std::string status() const noexcept;

    ProxyConnection* ref() noexcept {
        ++refCount;
        return this;
    }

    void unref() noexcept {
    if (--refCount == 0)
        delete this;
    }

    ProxyConnection& onAuthorized(AuthorizedHandle handle) noexcept {
        authorizedHandle = handle;
        return *this;
    }

    ProxyConnection& onOpened(StatusHandler handler) noexcept {
        openHandler = handler;
        return *this;
    }

    ProxyConnection& onClosed(StatusHandler handler) noexcept {
        closeHandler = handler;
        return *this;
    }

    ProxyConnection& onOpenFailed(StatusHandler handler) noexcept {
        openFailedHandle = handler;
        return *this;
    }

    ProxyConnection& onBusy(StatusHandler handler) noexcept {
        busyHandler = handler;
        return *this;
    }

    ProxyConnection& onIdle(StatusHandler handler) noexcept {
        idleHandler = handler;
        return *this;
    }

protected:
    void establish() noexcept;
    void keepAlive() noexcept;

    void sendAuthenticateRequest(const uint8_t* challenge, size_t size) noexcept;
    void sendAttachRequest(const uint8_t* data, size_t size) noexcept;
    void sendPingRequest() noexcept;
    void sendConnectResponse(bool success) noexcept;
    void sendDisconnectRequest() noexcept;
    void sendDataRequest(const uint8_t* data, size_t size) noexcept;

    void onChallenge(const uint8_t* data, size_t size) noexcept;
    void onRelayRead(const uint8_t* data, size_t size) noexcept;
    void processRelayPacket(const uint8_t* packet, size_t size) noexcept;

    void onAuthenticateResponse(const uint8_t* packet, size_t size) noexcept;
    void onAttachResponse(const uint8_t* packet, size_t size) noexcept;
    void onPingResponse(const uint8_t* packet, size_t size) const noexcept;
    void onConnectRequest(const uint8_t* packet, size_t size) noexcept;
    void onDisconnectRequest(const uint8_t* packet, size_t size) noexcept;
    void onDataRequest(const uint8_t* packet, size_t size) noexcept;

    void openUpstream() noexcept;
    void closeUpstream(bool force = false) noexcept;
    void startReadUpstream() noexcept;

    void onAuthorized(const CryptoBox::PublicKey& serverPk, uint16_t port) noexcept {
        if (authorizedHandle)
            authorizedHandle(this, serverPk, port);
    }

    void onOpened() noexcept {
        if (openHandler)
            openHandler(this);
    }

    void onClosed() noexcept {
        if (closeHandler)
            closeHandler(this);
    }

    void onOpenFailed() noexcept {
        if (openFailedHandle)
            openFailedHandle(this);
    }

    void onBusy() noexcept {
        if (busyHandler)
            busyHandler(this);
    }

    void onIdle() noexcept {
        if (idleHandler)
            idleHandler(this);
    }

private:
    uint32_t id;
    uint32_t refCount { 1 };

    ActiveProxy& proxy;
    ConnectionState state { ConnectionState::Connecting };

    uv_tcp_t relay { 0 };
    uv_tcp_t upstream { 0 };
    bool upstreamPaused { false };

    std::vector<uint8_t> stickyBuffer {};

    uv_timer_t keepAliveTimer { 0 };
    uint64_t keepAliveTimestamp { 0 };

    AuthorizedHandle authorizedHandle;
    StatusHandler openHandler;
    StatusHandler closeHandler;
    StatusHandler openFailedHandle;
    StatusHandler busyHandler;
    StatusHandler idleHandler;

    CryptoBox::Nonce nonce;
};

} // namespace activeproxy
} // namespace carrier
} // namespace elastos
