/*
 * Copyright (c) 2022 - 2023 trinity-tech.io
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

#include "def.h"
#include "id.h"
#include "socket_address.h"

namespace elastos {
namespace carrier {

struct CARRIER_PUBLIC PeerInfo {
    PeerInfo() = default;

    static PeerInfo of(Blob peerId, Blob privateKey, Blob nodeId, Blob origin, uint16_t port,
        const std::string& alternativeURL, Blob signature) {
        return PeerInfo(peerId, privateKey, nodeId, origin, port, alternativeURL, signature);
    }

    static PeerInfo create(const Id& nodeId, int port) {
        Signature::KeyPair keypair = Signature::KeyPair::random();
        return create(keypair, nodeId, {}, port, {});
    }

    static PeerInfo create(const Signature::KeyPair& keypair, const Id& nodeId, int port) {
        return create(keypair, nodeId, {}, port, {});
    }

    static PeerInfo create(const Id& nodeId, Id origin, int port) {
        Signature::KeyPair keypair = Signature::KeyPair::random();
        return create(keypair, nodeId, origin, port, {});
    }

    static PeerInfo create(const Signature::KeyPair& keypair, const Id& nodeId, Id origin, int port) {
        return create(keypair, nodeId, origin, port, {});
    }

    static PeerInfo create(const Id& nodeId, int port, const std::string& alternativeURL) {
        Signature::KeyPair keypair = Signature::KeyPair::random();
        return create(keypair, nodeId, {}, port, alternativeURL);
    }

    static PeerInfo create(const Signature::KeyPair& keypair, const Id& nodeId, int port, const std::string& alternativeURL) {
        return create(keypair, nodeId, {}, port, alternativeURL);
    }

    static PeerInfo create(const Id& nodeId, Id origin, int port, const std::string& alternativeURL) {
        Signature::KeyPair keypair = Signature::KeyPair::random();
        return create(keypair, nodeId, origin, port, alternativeURL);
    }

    static PeerInfo create(const Signature::KeyPair& keypair, const Id& nodeId, Id origin,
            int port, const std::string& alternativeURL) {
        return PeerInfo(keypair, nodeId, origin, port, alternativeURL);
    }

    const Id& getId() const noexcept {
        return publicKey;
    }

    bool hasPrivateKey() const noexcept {
        return privateKey.size() != 0;
    }

    const Signature::PrivateKey& getPrivateKey() const noexcept {
        return privateKey;
    }

    const Id& getNodeId() const noexcept {
        return nodeId;
    }

    const Id& getOrigin() const noexcept {
        return origin;
    }

    bool isDelegated() const noexcept {
        return nodeId != origin;
    }

    uint16_t getPort() const noexcept {
        return port;
    };

    const std::string& getAlternativeURL() const noexcept {
        return alternativeURL;
    }

    bool hasAlternativeURL() const noexcept {
        return !alternativeURL.empty();
    }

    const std::vector<uint8_t>& getSignature() const noexcept {
        return signature;
    }

    bool isValid() const;

    bool operator==(const PeerInfo& other) const;

    bool operator<(const PeerInfo& other) const {
        int rc = publicKey.compareTo(other.publicKey);
        if (rc != 0)
            return rc < 0;

        rc = nodeId.compareTo(other.nodeId);
        if (rc != 0)
            return rc < 0;

        rc = origin.compareTo(other.origin);
        return rc < 0;
    }

    operator std::string() const;
    friend std::ostream& operator<< (std::ostream& s, const PeerInfo& pi);

private:
    PeerInfo(const Blob& peerId, const Blob& privateKey, const Blob& nodeId, const Blob& origin, uint16_t port,
            const std::string& alternativeURL, const Blob& signature);

    PeerInfo(const Signature::KeyPair& keypair, const Id& nodeId, const Id& origin, uint16_t port,
            const std::string& alternativeURL);

    std::vector<uint8_t> getSignData() const;

private:
    Id publicKey {};
    Signature::PrivateKey privateKey {};
    Id nodeId {};
    Id origin {};
    uint16_t port {0};
    std::string alternativeURL {};
    std::vector<uint8_t> signature {};
};

}
}
