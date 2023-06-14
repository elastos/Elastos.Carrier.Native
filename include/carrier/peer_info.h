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

    PeerInfo(const Blob& id, const Blob& pid, uint16_t port, const std::string alt, const Blob& sig, int family = AF_INET);
    PeerInfo(const Blob& id, const uint16_t port, const std::string alt, const Blob& sig, int family = AF_INET);

    PeerInfo(const Id& id, const Id& pid, uint16_t port, const std::string alt, const std::vector<std::uint8_t>& sig, int family = AF_INET);

    PeerInfo(const PeerInfo& pi);

    const Id& getNodeId() const noexcept {
        return nodeId;
    }

    const Id& getProxyId() const noexcept {
        return proxyId;
    }

    uint16_t getPort() const noexcept {
        return port;
    };

    const std::string& getAlt() const noexcept {
        return alt;
    }

    const std::vector<uint8_t>& getSignature() const noexcept {
        return signature;
    }

    bool isUsedProxy() const noexcept { return usedProxy;  }
    bool isIPv4() const noexcept { return family == AF_INET;  }
    bool isIPv6() const noexcept { return family == AF_INET6; }

    bool isValid() const;

    int getFamily() const noexcept { return family; }

    bool operator==(const PeerInfo& other) const;

    bool operator<(const PeerInfo& other) const {
        return nodeId.compareTo(other.nodeId) < 0;
    }

    operator std::string() const;
    friend std::ostream& operator<< (std::ostream& s, const PeerInfo& pi);

    int size() const {
        auto size = nodeId.size() + 16 + alt.size() + signature.size();
        if (usedProxy) {
            size += proxyId.size();
        }
        return size;
    }

    int estimateSize() const {
        return nodeId.size() + 16 + proxyId.size() + alt.size() + signature.size() + sizeof(int) + sizeof(bool);
    }

private:
    friend class FindPeerResponse;
    friend class SqliteStorage;
    friend class Node;

    void setNodeId(const Blob& id) {
        this->nodeId = Id(id);
    }

    void setProxyId(const Blob& id) {
        this->proxyId = Id(id);
    }

    void setPort(const int port) {
        this->port = port;
    }

    void setAlt(const std::string alt) {
        this->alt = alt;
    }

    void setSignature(const Blob& val) {
        signature.resize(val.size());
        std::memcpy(signature.data(), val.ptr(), val.size());
    }

    void setIpV4() {
        this->family = AF_INET;
    }

    void setIpV6() {
        this->family = AF_INET6;
    }

    void setFamily(int family) {
        this->family = family;
    }

    static std::vector<uint8_t> createSignature(const Signature::PrivateKey& privateKey, const Id& nodeId,
        uint16_t port, const std::string& alt);
    bool verifySignature() const;

private:
    Id nodeId {};

    Id proxyId {};
    uint16_t port {0};
    std::string alt {};
    std::vector<std::uint8_t> signature {};

    bool usedProxy {false};
    int family { AF_INET };
};

}
}
