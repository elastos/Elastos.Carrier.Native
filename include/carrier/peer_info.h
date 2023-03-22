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

    PeerInfo(const uint8_t* id, size_t idLen, const uint8_t* addr,  size_t addrLen, int port)
        : nodeId(id, idLen), sockaddr(addr, addrLen, port) {}

    explicit PeerInfo(const std::vector<uint8_t>& id, const std::vector<uint8_t>& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}

    explicit PeerInfo(const Id& id, const std::string& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}

    explicit PeerInfo(const Id& id, const std::vector<uint8_t>& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}

    PeerInfo(const Id& id, const SocketAddress& _addr) noexcept
        : nodeId(id), sockaddr(_addr)  {}

    PeerInfo(const PeerInfo& ni)
        : nodeId(ni.nodeId), sockaddr(ni.sockaddr) {}

    const Id& getNodeId() const noexcept {
        return nodeId;
    }
    const SocketAddress& getAddress() const noexcept {
        return sockaddr;
    }
    int getPort() const noexcept {
        return sockaddr.port();
    };

    bool isIPv4() const noexcept { return sockaddr.family() == AF_INET;  }
    bool isIPv6() const noexcept { return sockaddr.family() == AF_INET6; }

    bool operator==(const PeerInfo& other) const {
        return nodeId == other.nodeId && sockaddr == other.sockaddr;
    }
    bool operator<(const PeerInfo& other) const {
        return nodeId.compareTo(other.nodeId) < 0;
    }

    operator std::string() const;
    friend std::ostream& operator<< (std::ostream& s, const PeerInfo& pi);

    friend void to_json(nlohmann::json& json, const PeerInfo& pi);
    friend void from_json(const nlohmann::json& json, PeerInfo& pi);

private:
    friend class SqliteStorage;
    void setNodeId(const Blob& blob) {
        this->nodeId = Id(blob.ptr(), blob.size());
    }

    void setSocketAddress(const uint8_t* ip, size_t len, in_port_t port) {
        this->sockaddr = SocketAddress(ip, len, port);
    }

private:
    Id nodeId {};
    SocketAddress sockaddr {};
};

}
}
