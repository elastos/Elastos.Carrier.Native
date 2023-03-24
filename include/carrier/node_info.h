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

struct CARRIER_PUBLIC NodeInfo {
    NodeInfo() = default;

    NodeInfo(const uint8_t* id, size_t idLen, const uint8_t* addr,  size_t addrLen, int port)
        : nodeId(id, idLen), sockaddr(addr, addrLen, port) {}

    explicit NodeInfo(const std::vector<uint8_t>& id, const std::vector<uint8_t>& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}

    explicit NodeInfo(const std::string& id, const std::string& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}

    explicit NodeInfo(const Id& id, const std::string& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}

    explicit NodeInfo(const Id& id, const std::vector<uint8_t>& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}

    explicit NodeInfo(const Id& id, const sockaddr* addr) noexcept
        : nodeId(id), sockaddr(addr) {}

    explicit NodeInfo(const Id& id, const SocketAddress& addr) noexcept
        : nodeId(id), sockaddr(addr)  {}

    NodeInfo(const NodeInfo& ni)
        : nodeId(ni.nodeId), sockaddr(ni.sockaddr) {}

    const Id& getId() const noexcept {
        return nodeId;
    }
    const SocketAddress& getAddress() const noexcept {
        return sockaddr;
    }
    int getPort() const noexcept {
        return sockaddr.port();
    }
    int getVersion() const noexcept {
        return version;
    }

    bool isIPv4() const noexcept { return sockaddr.family() == AF_INET;  }
    bool isIPv6() const noexcept { return sockaddr.family() == AF_INET6; }

    virtual bool match(const NodeInfo& other) const {
        return nodeId == other.nodeId || sockaddr == other.sockaddr;
    }

    virtual bool equals(const NodeInfo& other) const {
        return nodeId == other.nodeId && sockaddr == other.sockaddr;;
    }

    bool operator==(const NodeInfo& other) const {
        return equals(other);
    }

    operator std::string() const;
    friend std::ostream& operator<< (std::ostream& s, const NodeInfo& ni);

protected:
    void setVersion(int version) {
        this->version = version;
    }

private:
    Id nodeId;
    SocketAddress sockaddr;
    int version {0};
};

}
}
