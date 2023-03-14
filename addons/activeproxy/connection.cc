/*
 * Copyright (c) 2022 Elastos Foundation
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

#include <array>
#include <vector>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <alloca.h>

#include "connection.h"
#include "activeproxy.h"
#include "packetflag.h"
#include "exceptions.h"

namespace elastos {
namespace carrier {
namespace activeproxy {

using Logger = elastos::carrier::Logger;

static std::shared_ptr<Logger> log = Logger::get("ProxyConnection");

const static size_t PACKET_HEADER_BYTES = sizeof(uint16_t) + sizeof(uint8_t);
const static uint32_t KEEP_ALIVE_INTERVAL = 60000; // 60 seconds
static const uint32_t KEEP_ALIVE_CHECK_INTERVAL = 5000; // 5 seconds

static uint32_t lastConnectionId = 0;

ProxyConnection::ProxyConnection(ActiveProxy& proxy) noexcept :
        id(lastConnectionId++), proxy(proxy)
{
    log->setLevel(Level::Trace);

    log->trace("Connection {} created.", id);
}

ProxyConnection::~ProxyConnection() noexcept
{
    log->trace("Connection {} destoried.", id);
}

void ProxyConnection::unref() noexcept {
    if (--refCount == 0) {
        log->debug("Connection {} closed.", id);
        delete this;
    }
}

void ProxyConnection::close() noexcept
{
    if (state == ConnectionState::Closed)
        return;

    log->debug("Connection {} is closing...", id);

    if (state == ConnectionState::Authenticating)
        onOpenFailed();

    if (state == ConnectionState::Relaying)
        onIdle();

    state = ConnectionState::Closed;

    if (keepAliveTimer.data) {
        uv_timer_stop(&keepAliveTimer);
        uv_close((uv_handle_t*)&keepAliveTimer, [](uv_handle_t *handle) {
            ProxyConnection* pc = (ProxyConnection*)handle->data;
            handle->data = nullptr;
            pc->unref(); // keepAliveTimer.data
        });
    }

    if (upstream.data) {
        uv_read_stop((uv_stream_t*)&upstream);
        uv_close((uv_handle_t *)&upstream, [](uv_handle_t *handle) {
            ProxyConnection* pc = (ProxyConnection*)handle->data;
            handle->data = nullptr;
            pc->unref(); // upstream.data
        });

    }

    if (relay.data) {
        uv_read_stop((uv_stream_t*)&relay);
        uv_close((uv_handle_t *)&relay, [](uv_handle_t *handle) {
            ProxyConnection* pc = (ProxyConnection*)handle->data;
            handle->data = nullptr;
            pc->unref(); // relay.data
        });
    }

    onClosed();
}

int ProxyConnection::connectServer() noexcept
{
    log->info("Connection {} connecting to the server {}...", id, proxy.serverEndpoint());

    auto rc = uv_tcp_init(proxy.getLoop(), &relay);
    if (rc < 0) {
        log->error("Connection {} failed to initialize the tcp conection({}): {}",
                id, rc, uv_strerror(rc));
        return rc;
    }

    // Mandatory
    relay.data = (void*)ref(); // release in close()

    uv_connect_t* request = new uv_connect_t;
    request->data = (void*)ref();

    rc = uv_tcp_connect(request, &relay, proxy.serverAddress().addr(), [](uv_connect_t *req, int status) {
        ProxyConnection *pc = (ProxyConnection *)req->data;
        delete req;

        if (status < 0) {
            log->error("Connection {} connect to server {} failed({}): {}",
                    pc->id, pc->proxy.serverEndpoint(), status, uv_strerror(status));
            pc->close();
        } else {
            log->info("Connection {} connected to server {}", pc->id, pc->proxy.serverEndpoint());
            pc->establish();
        }

        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} connect to server {} failed({}): {}", id, rc, uv_strerror(rc));
        delete request;
        unref(); // request->data
        return rc;
    }

    return 0;
}

void ProxyConnection::establish() noexcept {
    // The server/Java side not support the socket keep-alive idle time,
    // so we don't use the built-in socket keep-alive mechanism.
    /*
    auto rc = uv_tcp_keepalive(&relay, true, KEEP_ALIVE_INTERVAL);
    if (rc < 0) {
        log->error("Set socket keep-alive failed({}): {}", rc, uv_strerror(rc));
        close()
        return;
    }
    */

    // Setup the keep-alive timer
    log->trace("Connection {} start the keep-alive timer.", id);
    uv_timer_init(proxy.getLoop(), &keepAliveTimer); // always success
    keepAliveTimer.data = (void*)ref(); // release in close()

    // we make sure timer should start success
    uv_timer_start(&keepAliveTimer, [](uv_timer_t *handle) {
        ProxyConnection* pc = (ProxyConnection*)handle->data;
        pc->keepAlive();
    }, KEEP_ALIVE_CHECK_INTERVAL, KEEP_ALIVE_CHECK_INTERVAL);

    state = ConnectionState::Authenticating;

    log->trace("Connection {} start reading from the server.", id);
    auto rc = uv_read_start((uv_stream_t*)&relay, [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
        ProxyConnection *pc = (ProxyConnection*)handle->data;
        pc->relayReadBuffer.resize(pc->relayReadStickyBytes + suggested_size);

        buf->base = (char *)pc->relayReadBuffer.data() + pc->relayReadStickyBytes;
        buf->len = suggested_size;
    }, [](uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {
        ProxyConnection *pc = (ProxyConnection *)handle->data;
        if (nread > 0) {
            pc->relayReadBuffer.resize(pc->relayReadStickyBytes + nread);
            pc->onRelayRead();
        } else if (nread < 0) {
            if (nread == UV_EOF) {
                log->info("Connection {} closed by the server.", pc->id);
            } else {
                log->error("Connection {} read server error({}): {}.", pc->id, nread, uv_strerror(nread));
            }
            pc->close();
        }
        // Regarding to the libuv document: nread might be 0,
        // which does not indicate an error or EOF.
        // This is equivalent to EAGAIN or EWOULDBLOCK under read(2).
    });
    if (rc < 0) {
        log->error("Connection {} start read from server {} failed({}): {}",
                id, proxy.serverEndpoint(), rc, uv_strerror(rc));
        close();
        return;
    }

    if (proxy.isAuthenticated()) {
        sendAttachRequest();
    } else {
        sendAuthenticateRequest();
    }
}

void ProxyConnection::keepAlive() noexcept
{
    // Dead connection check
    if (uv_now(proxy.getLoop()) - keepAliveTimestamp >= 3 * KEEP_ALIVE_INTERVAL) {
        log->warn("Connection {} is dead.", id);
        close();
        return;
    }

    // Keep-alive check
    uint32_t randomShift = Random::uint32(KEEP_ALIVE_CHECK_INTERVAL * 2);
    if (state == ConnectionState::Idling && (uv_now(proxy.getLoop()) - keepAliveTimestamp) >=
            (KEEP_ALIVE_INTERVAL - randomShift))
        sendPingRequest();
}

static inline size_t randomPadding(void)
{
    return Random::uint32(256);
}

void ProxyConnection::sendAuthenticateRequest() noexcept
{
    const Id& nodeId = proxy.getNodeId();
    const CryptoBox::PublicKey& pk = proxy.getSessionKey();
    const CryptoBox::Nonce& nonce = proxy.getSessionNonce();

    std::array<uint8_t, CryptoBox::PublicKey::BYTES + CryptoBox::Nonce::BYTES> plain;
    std::memcpy(plain.data(), pk.bytes(), pk.size());
    std::memcpy(plain.data() + pk.size(), nonce.bytes(), nonce.size());

    size_t padding = randomPadding();
    size_t size = PACKET_HEADER_BYTES + nodeId.size() + CryptoBox::MAC_BYTES + plain.size() + padding;
    relayWriteBuffer.resize(size);

    uint8_t* ptr = relayWriteBuffer.data();
    // size
    *(uint16_t*)ptr = htons(size);
    ptr += sizeof(uint16_t);
    // flag
    *ptr++ = PacketFlag::auth();
    // node id
    std::memcpy(ptr, nodeId.data(), nodeId.size());
    ptr += nodeId.size();
    // encrypted: session pk, nonce
    proxy.encryptWithNode(ptr, plain.size() + CryptoBox::MAC_BYTES, plain.data(), plain.size());
    ptr += CryptoBox::MAC_BYTES;
    ptr += plain.size();
    // random padding
    Random::buffer(ptr, padding);

    uv_buf_t buf = { (char*)relayWriteBuffer.data(), relayWriteBuffer.size() };

    log->debug("Connection {} send AUTH to server {}.", id, proxy.serverEndpoint());
    uv_write_t* request = new uv_write_t;
    request->data = (void*)ref();
    auto rc = uv_write(request, (uv_stream_t*)(&relay), &buf, 1, [](uv_write_t *req, int status) {
        ProxyConnection* pc = (ProxyConnection*)req->data;
        delete req;

        if (status < 0) {
            log->error("Connection {} send AUTH to server {} failed({}): {}",
                    pc->id, pc->proxy.serverEndpoint(), status, uv_strerror(status));
            pc->close();
        }

        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} send AUTH to server {} failed({}): {}",
                id, proxy.serverEndpoint(), rc, uv_strerror(rc));
        delete request;
        unref(); // request->data
        close();
    }
}

void ProxyConnection::sendAttachRequest() noexcept
{
    const CryptoBox::PublicKey& pk = proxy.getSessionKey();
    uint16_t port = htons(proxy.getRelayPort());

    size_t padding = randomPadding();
    size_t size = PACKET_HEADER_BYTES + pk.size() + CryptoBox::MAC_BYTES + sizeof(uint16_t) + padding;
    relayWriteBuffer.resize(size);

    uint8_t* ptr = relayWriteBuffer.data();
    // size
    *(uint16_t*)ptr = htons(size);
    ptr += sizeof(uint16_t);
    // flag
    *ptr++ = PacketFlag::attach();
    // session key
    std::memcpy(ptr, pk.bytes(), pk.size());
    ptr += pk.size();
    // encrypted: port
    proxy.encrypt(ptr, sizeof(uint16_t) + CryptoBox::MAC_BYTES, (const uint8_t*)&port, sizeof(uint16_t));
    ptr += CryptoBox::MAC_BYTES;
    ptr += sizeof(uint16_t);
    // random padding
    Random::buffer(ptr, padding);

    uv_buf_t buf = { (char*)relayWriteBuffer.data(), relayWriteBuffer.size() };

    log->debug("Connection {} send ATTACH to server {}.", id, proxy.serverEndpoint());
    uv_write_t* request = new uv_write_t;
    request->data = (void*)ref();
    auto rc = uv_write(request, (uv_stream_t*)(&relay), &buf, 1, [](uv_write_t *req, int status) {
        ProxyConnection* pc = (ProxyConnection*)req->data;
        delete req;

        if (status < 0) {
            log->error("Connection {} send ATTACH to server {} failed({}): {}",
                pc->id, pc->proxy.serverEndpoint(), status, uv_strerror(status));
            pc->close();
        }

        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} send ATTACH to server {} failed({}): {}",
                id, proxy.serverEndpoint(), rc, uv_strerror(rc));
        delete request;
        unref(); // request->data
        close();
    }
}

void ProxyConnection::sendPingRequest() noexcept
{
    size_t padding = randomPadding();
    size_t size = PACKET_HEADER_BYTES + padding;
    relayWriteBuffer.resize(size);

    uint8_t* ptr = relayWriteBuffer.data();
    // size
    *(uint16_t*)ptr = htons(size);
    ptr += sizeof(uint16_t);
    // flag
    *ptr++ = PacketFlag::ping();
    // random padding
    Random::buffer(ptr, padding);

    uv_buf_t buf = { (char*)relayWriteBuffer.data(), relayWriteBuffer.size() };

    log->debug("Connection {} send PING to server {}.", id, proxy.serverEndpoint());
    uv_write_t* request = new uv_write_t;
    request->data = (void*)ref();
    auto rc = uv_write(request, (uv_stream_t*)(&relay), &buf, 1, [](uv_write_t *req, int status) {
        ProxyConnection* pc = (ProxyConnection*)req->data;
        delete req;

        if (status < 0) {
            log->error("Connection {} send PING to server {} failed({}): {}",
                    pc->id, pc->proxy.serverEndpoint(), status, uv_strerror(status));
            pc->close();
        }

        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} send PING to server {} failed({}): {}",
                id, proxy.serverEndpoint(), rc, uv_strerror(rc));
        delete request;
        unref(); // request->data
        close();
    }
}

static uint8_t randomBoolean(bool v)
{
    uint8_t i = Random::uint8();
    return v ? i | 0x01 : i & 0xFE;
}

void ProxyConnection::sendConnectResponse(bool success) noexcept
{
    size_t padding = randomPadding();
    size_t size = PACKET_HEADER_BYTES + sizeof(uint8_t) + padding;
    relayWriteBuffer.resize(size);

    uint8_t* ptr = relayWriteBuffer.data();
    // size
    *(uint16_t*)ptr = htons(size);
    ptr += sizeof(uint16_t);
    // flag
    *ptr++ = PacketFlag::pingAck();
    // success?
    *ptr++ = randomBoolean(success);
    // random padding
    Random::buffer(ptr, padding);

    uv_buf_t buf = { (char*)relayWriteBuffer.data(), relayWriteBuffer.size() };

    log->debug("Connection {} send CONNECT ACK{} to server {}.", id, success, proxy.serverEndpoint());
    uv_write_t* request = new uv_write_t;
    request->data = (void*)ref();
    auto rc = uv_write(request, (uv_stream_t*)(&relay), &buf, 1, [](uv_write_t *req, int status) {
        ProxyConnection* pc = (ProxyConnection*)req->data;
        delete req;

        if (status < 0) {
            log->error("Connection {} send CONNECT ACK to server {} failed({}): {}",
                    pc->id, pc->proxy.serverEndpoint(), status, uv_strerror(status));
            pc->close();
        } else {
            if (pc->upstream.data)
                pc->startReadUpstream();
        }

        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} send CONNECT ACK to server {} failed({}): {}",
                id, proxy.serverEndpoint(), rc, uv_strerror(rc));
        delete request;
        unref(); // request->data
        close();
    }
}

void ProxyConnection::sendDisconnectRequest() noexcept
{
    size_t padding = randomPadding();
    size_t size = PACKET_HEADER_BYTES + padding;
    relayWriteBuffer.resize(size);

    uint8_t* ptr = relayWriteBuffer.data();
    // size
    *(uint16_t*)ptr = htons(size);
    ptr += sizeof(uint16_t);
    // flag
    *ptr++ = PacketFlag::disconnect();
    // random padding
    Random::buffer(ptr, padding);

    uv_buf_t buf = { (char*)relayWriteBuffer.data(), relayWriteBuffer.size() };

    log->debug("Connection {} send DISCONNECT to server {}.", id, proxy.serverEndpoint());
    uv_write_t* request = new uv_write_t;
    request->data = (void*)ref();
    auto rc = uv_write(request, (uv_stream_t*)(&relay), &buf, 1, [](uv_write_t *req, int status) {
        ProxyConnection* pc = (ProxyConnection*)req->data;
        delete req;

        if (status < 0) {
            log->error("Connection {} send DISCONNECT to server {} failed({}): {}",
                    pc->id, pc->proxy.serverEndpoint(), status, uv_strerror(status));
            pc->close();
        }

        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} send DISCONNECT to server {} failed({}): {}",
                id, proxy.serverEndpoint(), rc, uv_strerror(rc));
        delete request;
        unref(); // request->data
        close();
    }
}

void ProxyConnection::sendDataRequest(const uint8_t* data, size_t _size) noexcept
{
    size_t size = PACKET_HEADER_BYTES + CryptoBox::MAC_BYTES + _size;
    relayWriteBuffer.resize(size);

    uint8_t* ptr = relayWriteBuffer.data();
    // size
    *(uint16_t*)ptr = htons(size);
    ptr += sizeof(uint16_t);
    // flag
    *ptr++ = PacketFlag::data();
    // encrypted: data
    proxy.encrypt(ptr, _size + CryptoBox::MAC_BYTES, data, _size);

    uv_buf_t buf = { (char*)relayWriteBuffer.data(), relayWriteBuffer.size() };

    log->trace("Connection {} send DATA{} to server {}.", id, buf.len, proxy.serverEndpoint());
    uv_write_t* request = new uv_write_t;
    request->data = (void*)ref();
    auto rc = uv_write(request, (uv_stream_t*)(&relay), &buf, 1, [](uv_write_t *req, int status) {
        ProxyConnection* pc = (ProxyConnection*)req->data;
        delete req;

        if (status < 0) {
            log->error("Connection {} send DATA to server {} failed({}): {}",
                    pc->id, pc->proxy.serverEndpoint(), status, uv_strerror(status));
            pc->close();
        }

        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} send DATA to server {} failed({}): {}",
                id, proxy.serverEndpoint(), rc, uv_strerror(rc));
        delete request;
        unref(); // request->data
        close();
    }
}

void ProxyConnection::onRelayRead() noexcept
{
    // update the keep-alive timestamp first when we get data from the server
    keepAliveTimestamp = uv_now(proxy.getLoop());

    const uint8_t* ptr = relayReadBuffer.data();
    size_t read = 0;

    while (true) {
        if ((relayReadBuffer.size() - read) < PACKET_HEADER_BYTES)
            break;

        uint16_t size = ntohs(*(uint16_t *)ptr);
        if (size < PACKET_HEADER_BYTES) {
            log->error("Connection {} got invalid packet from the server {}, packet size({})",
                    id, proxy.serverEndpoint(), size);
            close();
            return;
        }

        if ((relayReadBuffer.size() - read) < size)
            break;

        processRelayPacket(ptr, size);

        read += size;
        ptr += size;
    }

    size_t remaining = relayReadBuffer.size() - read;
    if (remaining > 0 && relayReadBuffer.data() != ptr)
        std::memmove(relayReadBuffer.data(), ptr, remaining);

    relayReadStickyBytes = remaining;
}

void ProxyConnection::processRelayPacket(const uint8_t* packet, size_t size) noexcept
{
    uint8_t flag = *(packet + sizeof(uint16_t));
    bool ack = PacketFlag::isAck(flag);
    uint8_t type = PacketFlag::getType(flag);

    log->trace("Connection {} got packet from server {}: type={}, ack={}, size={}",
            proxy.serverEndpoint(), id, type, ack, size);

    if (type == PacketFlag::ERROR) {
        size_t len = size - PACKET_HEADER_BYTES - CryptoBox::MAC_BYTES;
        uint8_t* plain = (uint8_t*)alloca(len);
        proxy.decrypt(plain, len, packet + PACKET_HEADER_BYTES, size - PACKET_HEADER_BYTES);
        int code = ntohs(*(uint16_t*)plain);
        char *msg = (char *)plain + sizeof(uint16_t);
        log->error("Connection {} got ERROR response from the server {}, error({}): {}",
                id, proxy.serverEndpoint(), code, msg);
        close();
        return;
    }

    switch (state) {
    case ConnectionState::Connecting:
    case ConnectionState::Closed:
        assert(!"should not receive any data when connecting or closed.");
        log->error("Connection {} got a packet when connecting or closed.", id);
        close();
        return;

    case ConnectionState::Authenticating:
        if (ack && type == PacketFlag::AUTH) {
            onAuthenticateResponse(packet, size);
            return;
        } else if (ack && type == PacketFlag::ATTACH) {
            onAttachResponse(packet, size);
            return;
        } else {
            log->error("Connection {} got a wrong packet, AUTH or ATTACH acknowledge expected.", id);
            close();
            return;
        }
        break;

    case ConnectionState::Idling:
        if (ack && type == PacketFlag::PING) {
            onPingResponse(packet, size);
            return;
        } else if (!ack && type == PacketFlag::CONNECT) {
            onConnectRequest(packet, size);
            return;
        } else {
            log->error("Connection {} got a wrong packet, PING acknowledge or CONNECT expected.", id);
            close();
            return;
        }
        break;

    case ConnectionState::Relaying:
        if (type == PacketFlag::DATA) {
            onDataRequest(packet, size);
            return;
        } else if (!ack && type == PacketFlag::DISCONNECT) {
            onDisconnectRequest(packet, size);
            return;
        } else {
            log->error("Connection {} got a wrong packet, DATA or DISCONNECT expected.", id);
            close();
            return;
        }
        break;
    }
}

const static size_t AUTH_ACK_SIZE = PACKET_HEADER_BYTES + CryptoBox::MAC_BYTES +
    CryptoBox::PublicKey::BYTES + CryptoBox::Nonce::BYTES + sizeof(uint16_t);

void ProxyConnection::onAuthenticateResponse(const uint8_t* packet, size_t size) noexcept
{
    if (size < AUTH_ACK_SIZE) {
        log->error("Connection {} got an invalid AUTH ACK from server {}", id, proxy.serverEndpoint());
        close();
        return;
    }

    log->debug("Connection {} got AUTH ACK from server {}", id, proxy.serverEndpoint());

    std::array<uint8_t, AUTH_ACK_SIZE - PACKET_HEADER_BYTES - CryptoBox::MAC_BYTES> plain;
    proxy.decryptWithNode(plain.data(), plain.size(), packet + PACKET_HEADER_BYTES, AUTH_ACK_SIZE - PACKET_HEADER_BYTES);

    const uint8_t* ptr = plain.data();
    CryptoBox::PublicKey serverPk{ptr, CryptoBox::PublicKey::BYTES};
    ptr += CryptoBox::PublicKey::BYTES;
    CryptoBox::Nonce nonce{ptr, CryptoBox::Nonce::BYTES};
    ptr += CryptoBox::Nonce::BYTES;
    uint16_t port = ntohs(*(uint16_t*)ptr);

    onAuthorized(serverPk, nonce, port);
    state = ConnectionState::Idling;
    onOpened();

    log->info("Connection {} opened.", id);
}

const static size_t ATTACH_ACK_SIZE = PACKET_HEADER_BYTES;

void ProxyConnection::onAttachResponse(const uint8_t* packet, size_t size) noexcept
{
    log->debug("Connection {} got ATTACH ACK from server {}", id, proxy.serverEndpoint());
    state = ConnectionState::Idling;
    onOpened();
}

void ProxyConnection::onPingResponse(const uint8_t* packet, size_t size) const noexcept
{
    log->debug("Connection {} got PING ACK from server {}", id, proxy.serverEndpoint());
    // ignore the random padding payload.
    // keep-alive time stamp already update when we got the server data.
    // so nothing to do here.
}

const static size_t CONNECT_REQ_SIZE = PACKET_HEADER_BYTES + CryptoBox::MAC_BYTES + 1 + 16 + 2;

void ProxyConnection::onConnectRequest(const uint8_t* packet, size_t size) noexcept
{
    if (size < CONNECT_REQ_SIZE) {
        log->error("Connection {} got an invalid CONNECT from server {}.", id, proxy.serverEndpoint());
        close();
        return;
    }

    log->debug("Connection {} got CONNECT from server {}", id, proxy.serverEndpoint());

    state = ConnectionState::Relaying;
    onBusy();

    std::array<uint8_t, CONNECT_REQ_SIZE - PACKET_HEADER_BYTES - CryptoBox::MAC_BYTES> plain;
    proxy.decrypt(plain.data(), plain.size(), packet + PACKET_HEADER_BYTES,
                        CONNECT_REQ_SIZE - PACKET_HEADER_BYTES);

    const uint8_t* ptr = plain.data();
    uint8_t addrLen = *ptr++;
    const uint8_t* addr = ptr;
    ptr += 16;
    uint16_t port = ntohs(*(uint16_t*)ptr);
    SocketAddress client{addr, addrLen, port};

    if (proxy.allow(client)) {
        state = ConnectionState::Relaying;
        connectUpstream();
    } else {
        sendConnectResponse(false);
        state = ConnectionState::Idling;
        onIdle();
    }
}

void ProxyConnection::onDisconnectRequest(const uint8_t* packet, size_t size) noexcept
{
    log->debug("Connection {} got DISCONNECT from server {}", id, proxy.serverEndpoint());
    disconnectUpstream();
}

void ProxyConnection::onDataRequest(const uint8_t* packet, size_t size) noexcept
{
    log->trace("Connection {} got DATA({}) from server {}", id, size, proxy.serverEndpoint());

    upstreamWriteBuffer.resize(size - PACKET_HEADER_BYTES - CryptoBox::MAC_BYTES);
    proxy.decrypt(upstreamWriteBuffer.data(), upstreamWriteBuffer.size(),
                packet + PACKET_HEADER_BYTES, size - PACKET_HEADER_BYTES);

    uv_buf_t buf { (char *)upstreamWriteBuffer.data(), upstreamWriteBuffer.size() };

    log->trace("Connection {} sending {} bytes data to upstream {}", id, buf.len, proxy.upstreamEndpoint());
    uv_write_t* request = new uv_write_t;
    request->data = (void*)ref();
    auto rc = uv_write(request, (uv_stream_t*)(&upstream), &buf, 1, [](uv_write_t *req, int status) {
        ProxyConnection* pc = (ProxyConnection*)req->data;
        delete req;

        if (status < 0) {
            log->error("Connection {} sent to upstream {} failed({}): {}",
                    pc->id, pc->proxy.upstreamEndpoint(), status, uv_strerror(status));
            pc->close();
        }

        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} write to upstream {} failed({}): {}",
                id, proxy.upstreamEndpoint(), rc, uv_strerror(rc));
        disconnectUpstream();
        delete request;
        unref(); // request->data
    }
}

void ProxyConnection::connectUpstream() noexcept
{
    log->debug("Connection {} connecting to the upstream server {}...", id, proxy.upstreamEndpoint());

    auto rc = uv_tcp_init(proxy.getLoop(), &upstream);
    if (rc < 0) {
        log->error("Connection {} failed to initialize the tcp conection({}): {}", id, rc, uv_strerror(rc));
        sendConnectResponse(false);
        state = ConnectionState::Idling;
        onIdle();
    }
    upstream.data = (void*)ref(); // release in disconnectUpstream() or close()

    uv_connect_t* request = new uv_connect_t;
    request->data = (void*)ref();
    rc = uv_tcp_connect(request, &upstream, proxy.upstreamAddress().addr(), [](uv_connect_t *req, int status) {
        ProxyConnection *pc = (ProxyConnection *)req->data;
        delete req;

        if (status < 0) {
            log->error("Connection {} connect to upstream {} failed({}): {}",
                    pc->id, pc->proxy.upstreamEndpoint(), status, uv_strerror(status));
            uv_close((uv_handle_t*)&pc->upstream, [](uv_handle_t *handle) {
                ProxyConnection* pc = (ProxyConnection*)handle->data;
                handle->data = nullptr;
                pc->unref(); // upstream.data
            });
        } else {
            log->info("Connection {} connected to upstream {}", pc->id, pc->proxy.upstreamEndpoint());
        }

        pc->sendConnectResponse(status == 0);
        if (status < 0) {
            pc->state = ConnectionState::Idling;
            pc->onIdle();
        }

        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} connect to upstream {} failed({}): {}",
                id, proxy.upstreamEndpoint(), rc, uv_strerror(rc));
        uv_close((uv_handle_t*)&upstream, [](uv_handle_t *handle) {
            ProxyConnection* pc = (ProxyConnection*)handle->data;
            handle->data = nullptr;
            pc->unref(); // upstream.data
        });

        state = ConnectionState::Idling;
        onIdle();

        delete request;
        unref(); // request->data
    }
}

void ProxyConnection::disconnectUpstream() noexcept
{
    log->debug("Connection {} closing upstream {}", id, proxy.upstreamEndpoint());

    // Stop reading
    uv_read_stop((uv_stream_t*)&upstream);

    if (!upstream.data)
        return;

    uv_shutdown_t *request = new uv_shutdown_t;
    request->data = (void *)ref();
    auto rc = uv_shutdown(request, (uv_stream_t*)&upstream, [](uv_shutdown_t *req, int status){
        ProxyConnection *pc = (ProxyConnection *)req->data;
        delete req;

        pc->upstream.data = nullptr;
        uv_close((uv_handle_t*)&pc->upstream, [](uv_handle_t *handle) {
            ProxyConnection* pc = (ProxyConnection*)handle->data;
            handle->data = nullptr;
            pc->unref(); // upstream.data
        });

        log->debug("Connection {} closed upstream {}", pc->id, pc->proxy.upstreamEndpoint());

        pc->state = ConnectionState::Idling;
        pc->onIdle();
        pc->unref(); // request->data
    });
    if (rc < 0) {
        log->error("Connection {} shutdown upstream failed({}): {}", id, rc, uv_strerror(rc));
        // Ingore, maybe cause resources leak
        upstream.data = nullptr;
        unref(); // upstream.data
        delete request;
        unref(); // request->data
    }
}

void ProxyConnection::startReadUpstream() noexcept
{
    log->trace("Connection {} start reading from the upstream.", id);

    auto rc = uv_read_start((uv_stream_t*)&upstream, [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
        ProxyConnection *pc = (ProxyConnection*)handle->data;
        pc->upstreamReadBuffer.resize(suggested_size);

        buf->base = (char *)pc->upstreamReadBuffer.data();
        buf->len = suggested_size;
   }, [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
        ProxyConnection *pc = (ProxyConnection *)stream->data;
        if (nread > 0) {
            log->trace("Connection {} got {} bytes data from upstream {}", pc->id, nread, pc->proxy.upstreamEndpoint());
            pc->upstreamReadBuffer.resize(nread);
            pc->sendDataRequest(pc->upstreamReadBuffer.data(), pc->upstreamReadBuffer.size());
        } else if (nread < 0) {
            if (nread == UV_EOF) {
                log->info("Connection {} upstream closed.", pc->id);
            } else {
                log->error("Connection {} read upstream error({}): {}.", pc->id, nread, uv_strerror(nread));
            }
            pc->disconnectUpstream();
        }
        // Regarding to the libuv document: nread might be 0,
        // which does not indicate an error or EOF.
        // This is equivalent to EAGAIN or EWOULDBLOCK under read(2).
   });
   if (rc < 0) {
        log->error("Connection {} start read from upstream {} failed({}): {}",
                id, proxy.upstreamEndpoint(), rc, uv_strerror(rc));
        disconnectUpstream();
   }
}

} // namespace activeproxy
} // namespace carrier
} // namespace elastos
