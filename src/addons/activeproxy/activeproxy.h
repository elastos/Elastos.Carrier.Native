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

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <optional>
#include <thread>

#include "carrier.h"
#include "carrier/blob.h"

namespace elastos {
namespace carrier {
namespace activeproxy {

using SocketAddress = elastos::carrier::SocketAddress;
using CryptoBox = elastos::carrier::CryptoBox;

class ProxyConnection;

class ActiveProxy : public Addon {
public:
    std::future<void> initialize(Sp<Node> node, const std::map<std::string, std::any>& config) override;

    std::future<void> deinitialize() override;

    bool isInitialized() override {
        return isRunning();
    }

    const std::string& serverHostName() const noexcept {
        return serverHost;
    }

    const std::string& serverEndpoint() const noexcept {
        return serverName;
    }

    const SocketAddress& serverAddress() const noexcept {
        return serverAddr;
    }

    const Id& serverNodeId() const noexcept {
        return serverId;
    }

    const std::string& upstreamEndpoint() const noexcept {
        return upstreamName;
    }

    const SocketAddress& upstreamAddress() const noexcept {
        return upstreamAddr;
    }

    bool isRunning() const noexcept {
        return running;
    }

    void start();
    void stop() noexcept;

    uv_loop_t* getLoop() noexcept {
        return &loop;
    }

    const Id& getNodeId() const noexcept {
        return node->getId();
    }

    const CryptoBox::PublicKey& getSessionKey() const noexcept {
        return sessionKey.publicKey();
    }

    bool isAuthenticated() const noexcept {
        return serverPk.has_value();
    }

    bool allow(SocketAddress& client) const noexcept {
        return true;
    }

    uint16_t getRelayPort() const noexcept {
        return relayPort;
    }

    const std::string& getDomainName() const noexcept {
        return domainName;
    }

    // encrypt/decrypt with the session context
    void encrypt(Blob& cipher, const Blob& plain, const CryptoBox::Nonce& nonce) const {
        box.encrypt(cipher, plain, nonce);
    }

    std::vector<uint8_t> encrypt(const Blob& plain, const CryptoBox::Nonce& nonce) const {
        return box.encrypt(plain, nonce);
    }

    void decrypt(Blob& plain, const Blob& cipher, const CryptoBox::Nonce& nonce) const {
        box.decrypt(plain, cipher, nonce);
    }

    std::vector<uint8_t> decrypt(const Blob& cipher, const CryptoBox::Nonce& nonce) const {
        return box.decrypt(cipher, nonce);
    }

    // encrypt/decrypt with the node context
    void encryptWithNode(Blob& cipher, const Blob& plain) const {
        node->encrypt(serverId, cipher, plain);
    }

    std::vector<uint8_t> encryptWithNode(const Blob& plain) const {
        return node->encrypt(serverId, plain);
    }

    void decryptWithNode(Blob& plain, const Blob& cipher) const {
        node->decrypt(serverId, plain, cipher);
    }

    std::vector<uint8_t> decryptWithNode(const Blob& cipher) const {
        return node->decrypt(serverId, cipher);
    }

    std::vector<uint8_t> signWithNode(const Blob& data) const {
        return node->sign(data);
    }

    const std::string& getLogLevel() const {
        return logLevel;
    }

protected:
    void onStop() noexcept;
    void onIteration() noexcept;
    void connect() noexcept;
    void idleCheck() noexcept;
    void healthCheck() noexcept;

    bool needsNewConnection() noexcept;

    void announcePeer() noexcept;

    void reset() noexcept {
        sessionKey = CryptoBox::KeyPair();
        serverPk.reset();

        lastConnectTimestamp = 0;
        serverFails = 0;
        reconnectDelay = 0;

        idleTimestamp = UINT64_MAX;

        lastIdleCheckTimestamp = 0;
        lastHealthCheckTimestamp = 0;
        lastAnnouncePeerTimestamp = 0;

        inFlights = 0;
    }

private:
    Sp<Node> node;

    CryptoBox::KeyPair sessionKey;
    std::optional<CryptoBox::PublicKey> serverPk {};
    CryptoBox box;

    Id serverId {};
    std::string serverHost {};
    int serverPort {0};
    std::string serverName {};
    SocketAddress serverAddr {};
    std::string domainName {};
    uint16_t relayPort;

    std::string upstreamHost;
    int upstreamPort;
    std::string upstreamName;
    SocketAddress upstreamAddr;
    uint32_t upstreamFails { 0 };

    uv_loop_t loop { 0 };
    uv_async_t stopHandle { 0 };
    uv_idle_t idleHandle { 0 };

    uint64_t lastConnectTimestamp { 0 };
    uint32_t serverFails  {0};
    uint32_t reconnectDelay { 0 };

    uint64_t idleTimestamp { UINT64_MAX };

    uint64_t lastIdleCheckTimestamp { 0 };
    uint64_t lastHealthCheckTimestamp { 0 };
    uint64_t lastAnnouncePeerTimestamp { 0 };

    std::optional<Signature::KeyPair> peerKeypair {};
    std::optional<PeerInfo> peer {};

    uint32_t maxConnections { 16 };
    uint32_t inFlights { 0 };
    std::vector<ProxyConnection*> connections;

    bool running { false };
    bool first { false };

    std::thread runner;

    std::promise<void> startPromise {};
    std::promise<void> stopPromise {};

    std::string logLevel {};
};

} // namespace activeproxy
} // namespace carrier
} // namespace elastos

