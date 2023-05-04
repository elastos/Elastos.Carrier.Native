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

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __ANDROID__
typedef uint16_t in_port_t;
#endif
#else
#include <iso646.h>
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef uint8_t sa_family_t;
typedef uint16_t in_port_t;
#endif

#include <string>
#include <vector>
#include <memory>
#include <cstring>

#include "def.h"
#include "blob.h"

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC SocketAddress {
public:
    SocketAddress() {}

    SocketAddress(const SocketAddress& o) noexcept
        : SocketAddress(o.ss) {}

    SocketAddress(SocketAddress&& o) noexcept
        : SocketAddress(o.ss) {}

    SocketAddress(const sockaddr_storage& ss) noexcept {
        std::memcpy((uint8_t*)&this->ss, (const uint8_t*)&ss, sizeof(sockaddr_storage));
    }

    SocketAddress(const sockaddr& sa) noexcept
        : SocketAddress((const sockaddr*)&sa) {}

    SocketAddress(const sockaddr* sa) noexcept {
        std::memcpy((uint8_t*)&ss, (const uint8_t*)sa, sslen(sa->sa_family));
    }

    SocketAddress(const std::string& ip, const std::string& service = {});
    SocketAddress(const std::string& ip, in_port_t port)
        :  SocketAddress(ip, std::to_string(port)) {}

    SocketAddress(const char* ip, in_port_t port = 0)
        :  SocketAddress(std::string(ip), std::to_string(port)) {}

    SocketAddress(const Blob& ip, in_port_t port);

    static std::vector<SocketAddress> resolve(const std::string& host, const std::string& service = {});
    static std::vector<SocketAddress> resolve(const std::string& host, in_port_t port) {
        return resolve(host, std::to_string(port));
    }

    SocketAddress& operator=(const SocketAddress& o) noexcept {
        std::memcpy((uint8_t*)&ss, (const uint8_t*)&o.ss, sizeof(sockaddr_storage));
        return *this;
    }

    SocketAddress& operator=(SocketAddress&& o) noexcept {
        std::memcpy((uint8_t*)&ss, (const uint8_t*)&o.ss, sizeof(sockaddr_storage));
        return *this;
    }

    bool operator<(const SocketAddress& o) const noexcept {
        if (ss.ss_family != o.ss.ss_family)
            return ss.ss_family < o.ss.ss_family;
        return std::memcmp((const uint8_t*)addr(), (const uint8_t*)o.addr(), sslen(ss.ss_family)) < 0;
    }

    bool operator==(const SocketAddress& o) const noexcept {
        return ss.ss_family == o.ss.ss_family
            && std::memcmp((const uint8_t*)&ss, (const uint8_t*)&o.ss, sslen(ss.ss_family)) == 0;
    }

    bool operator!=(const SocketAddress& o) const noexcept {
        return ss.ss_family != o.ss.ss_family
            || std::memcmp((const uint8_t*)&ss, (const uint8_t*)&o.ss, sslen(ss.ss_family)) != 0;
    }

    explicit operator bool() const noexcept {
        return !empty();
    }

    socklen_t length() const noexcept {
        return sslen(ss.ss_family);
    }

    sa_family_t family() const noexcept {
        return ss.ss_family;
    }

    std::string host() const noexcept;
    in_port_t port() const noexcept;

    const sockaddr* addr() const noexcept {
        return empty() ? nullptr : (const sockaddr*)&ss;
    }

    const sockaddr_in* addr4() const noexcept {
        return reinterpret_cast<const sockaddr_in*>(addr());
    }

    const sockaddr_in6* addr6() const noexcept {
        return reinterpret_cast<const sockaddr_in6*>(addr());
    }

    const in_addr* inaddr4() const noexcept {
        return empty() ? nullptr : &((reinterpret_cast<const sockaddr_in*>(&ss))->sin_addr);
    }

    const in6_addr* inaddr6() const noexcept {
        return empty() ? nullptr : &((reinterpret_cast<const sockaddr_in6*>(&ss))->sin6_addr);
    }

    const uint8_t* inaddr() const;
    size_t inaddrLength() const;

    bool isAnyLocal() const;
    bool isLoopback() const;
    bool isSiteLocal() const;
    bool isBroadcast() const;
    bool isMulticast() const;
    bool isLinkLocal() const;

    bool isBogon() const;
    bool isGlobalUnicast() const;
    bool isAnyUnicast() const;

    bool isMappedIPv4() const;

    SocketAddress getMappedIPv4() const noexcept;
    SocketAddress getMappedIPv6() const noexcept;

    std::string toString() const noexcept {
        return toString(addr());
    }

    static std::string toString(const sockaddr* sa) noexcept;
    static std::string toString(const sockaddr& sa) noexcept {
        return toString(&sa);
    }
    static std::string toString(const sockaddr_storage* ss) noexcept {
        return toString((const sockaddr*)ss);
    }
    static std::string toString(const sockaddr_storage& ss) noexcept {
        return toString((const sockaddr*)&ss);
    }

    class IpCompare {
    public:
        bool operator()(const SocketAddress& a, const SocketAddress& b) const noexcept;
    };

private:
    static socklen_t sslen(sa_family_t family);

    bool empty() const noexcept {
        return ss.ss_family != AF_INET && ss.ss_family != AF_INET6;
    }

    sockaddr_storage ss { 0, AF_UNSPEC };
};

} // namespace carrier
} // namespace elastos
