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

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <iso646.h>
#include <stdint.h>
#include <winsock2.h>
#include <ws2config.h>
#include <ws2tcpip.h>
#endif

#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "carrier/socket_address.h"

namespace elastos {
namespace carrier {

// Invalid address
static std::string NA = "N/A";
static constexpr std::array<uint8_t, 12> MAPPED_IPV4_PREFIX {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff}};

/// An IPv4 equivalent to IN6_IS_ADDR_UNSPECIFIED
#ifndef IN_IS_ADDR_UNSPECIFIED
#define IN_IS_ADDR_UNSPECIFIED(a) (((long int) (a)->s_addr) == 0x00000000)
#endif /* IN_IS_ADDR_UNSPECIFIED */

SocketAddress::SocketAddress(const uint8_t* ip, size_t len, in_port_t port)
{
    port = htons(port);

    switch (len) {
    case sizeof(in_addr): {
        sockaddr_in* addr4 = (sockaddr_in*)&ss;
#ifdef __APPLE__
        addr4->sin_len = sizeof(sockaddr_in);
#endif
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        std::memcpy(&(addr4->sin_addr.s_addr), ip, len);
        break;
    }
    case sizeof(in6_addr): {
        sockaddr_in6* addr6 = (sockaddr_in6*)&ss;
#ifdef __APPLE__
        addr6->sin6_len = sizeof(sockaddr_in6);
#endif
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        std::memcpy(&(addr6->sin6_addr.s6_addr), ip, len);
        break;
    }
    default:
        throw std::invalid_argument("invalid ip lenght");
    }
}

std::vector<SocketAddress> SocketAddress::resolve(const std::string& host, const std::string& service)
{
    std::vector<SocketAddress> addrs {};
    if (host.empty() || service.empty())
        throw std::invalid_argument("Invalid host or service");

    addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    addrinfo* ais = nullptr;
    auto rc = getaddrinfo(host.c_str(), service.empty() ? nullptr : service.c_str(), &hints, &ais);
    if (rc < 0)
        throw std::runtime_error(gai_strerror(rc));

    for (addrinfo* ai = ais; ai; ai = ai->ai_next)
        addrs.emplace_back(ai->ai_addr);

    freeaddrinfo(ais);
    return addrs;
}

SocketAddress::SocketAddress(const std::string& ip, const std::string& service)
{
    if (ip.empty() || service.empty())
        throw std::invalid_argument("Invalid host or service");

    addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;

    addrinfo* ais = nullptr;
    auto rc = getaddrinfo(ip.c_str(), service.empty() ? nullptr : service.c_str(), &hints, &ais);
    if (rc < 0)
        throw std::runtime_error(gai_strerror(rc));

    std::memcpy(&ss, ais->ai_addr, sslen(ais->ai_addr->sa_family)); // use the first one
    freeaddrinfo(ais);
}

socklen_t SocketAddress::sslen(sa_family_t family) {
    switch (family) {
    case AF_INET:
        return (socklen_t)sizeof(sockaddr_in);
    case AF_INET6:
        return (socklen_t)sizeof(sockaddr_in6);
    default:
        return 0;
    }
}

std::string SocketAddress::host() const noexcept
{
    char host[NI_MAXHOST];
    const void *inaddr;

    switch (ss.ss_family) {
    case AF_INET:
        inaddr = &(((const sockaddr_in*)&ss)->sin_addr);
        break;
    case AF_INET6:
        inaddr = &(((const sockaddr_in6*)&ss)->sin6_addr);
        break;
    default:
        return NA;
    }

    return inet_ntop(ss.ss_family, inaddr, host, sizeof(host)) ? host : NA;
}

in_port_t SocketAddress::port() const noexcept
{
    switch (ss.ss_family) {
    case AF_INET:
        return addr4() ? ntohs(addr4()->sin_port) : 0;
    case AF_INET6:
        return addr6() ? ntohs(addr6()->sin6_port) : 0;
    default:
        return 0;
    }
}

const uint8_t* SocketAddress::inaddr() const
{
    switch (ss.ss_family) {
        case AF_INET:
            return (uint8_t*)(inaddr4());
        case AF_INET6:
            return (uint8_t*)(inaddr6());
        default:
            return nullptr;
    }
}

size_t SocketAddress::inaddrLength() const
{
    switch (ss.ss_family) {
        case AF_INET:
            return sizeof(in_addr);
        case AF_INET6:
            return sizeof(in6_addr);
        default:
            return 0;
    }
}

bool SocketAddress::isAnyLocal() const
{
    switch (ss.ss_family) {
    case AF_INET:
        return IN_IS_ADDR_UNSPECIFIED(&(((const sockaddr_in*)&ss)->sin_addr));
    case AF_INET6:
        return IN6_IS_ADDR_UNSPECIFIED(&(((const sockaddr_in6*)&ss)->sin6_addr));
    default:
        throw std::domain_error("Invalid address");
    }
}

bool SocketAddress::isLoopback() const
{
    switch (ss.ss_family) {
    case AF_INET: {
        auto addr = ntohl(inaddr4()->s_addr);
        uint8_t b1 = (uint8_t)(addr >> 24);
        return b1 == 127;
    }
    case AF_INET6:
        return IN6_IS_ADDR_LOOPBACK(inaddr6());
    default:
        throw std::domain_error("Invalid address");
    }
}

bool SocketAddress::isSiteLocal() const
{
    switch (ss.ss_family) {
    case AF_INET: {
        auto addr = ntohl(inaddr4()->s_addr);
        uint8_t b1, b2;
        b1 = (uint8_t)(addr >> 24);
        b2 = (uint8_t)((addr >> 16) & 0x0ff);

        // 10.0.0.0/8
        if (b1 == 10)
            return true;

        // 172.16.0.0/12
        if ((b1 == 172) && (b2 >= 16) && (b2 <= 31))
            return true;

        // 192.168.0.0/16
        if ((b1 == 192) && (b2 == 168))
            return true;

        return false;
    }
    case AF_INET6: {
        const uint8_t* addr6 = reinterpret_cast<const uint8_t*>(inaddr6());
        if (addr6[0] == 0xfc || addr6[0] == 0xfd)
            return true;

        return false;
    }
    default:
        throw std::domain_error("Invalid address");
    }
}

bool SocketAddress::isBroadcast() const
{
    switch (ss.ss_family) {
    case AF_INET:
        return inaddr4()->s_addr == 0xffffffff;
    default:
        return false;
    }
}

bool SocketAddress::isMulticast() const
{
    switch (ss.ss_family) {
    case AF_INET:
        return (inaddr4()->s_addr & 0xf0000000) == 0xe0000000;
    case AF_INET6:
        return IN6_IS_ADDR_MULTICAST(inaddr6());
    default:
        throw std::domain_error("Invalid address");
    }
}

bool SocketAddress::isLinkLocal() const
{
    switch (ss.ss_family) {
    case AF_INET: {
        auto addr_host = ntohl(inaddr4()->s_addr);
        uint8_t b1, b2;
        b1 = (uint8_t)(addr_host >> 24);
        b2 = (uint8_t)((addr_host >> 16) & 0x0ff);
        // 169.254.y.z
        if (b1 == 169 && b2 == 254)
            return true;

        return false;
    }

    case AF_INET6: {
        const uint8_t* addr6 = reinterpret_cast<const uint8_t*>(inaddr6());
        if (addr6[0] == 0xfe && addr6[1] == 0x80)
            return true;

        return false;
    }

    default:
        throw std::domain_error("Invalid address");
    }
}

bool SocketAddress::isGlobalUnicast() const
{
    return !(isAnyLocal() || isLinkLocal() || isLoopback()
            || isMulticast() || isSiteLocal() || isMappedIPv4() || isBroadcast());
}

bool SocketAddress::isAnyUnicast() const
{
    return isGlobalUnicast() || isSiteLocal();
}

bool SocketAddress::isBogon() const
{
    return !(port() > 0 && port() <= 0xffff && isGlobalUnicast());
}

bool SocketAddress::isMappedIPv4() const
{
    if (ss.ss_family != AF_INET6)
        return false;
    const uint8_t* adress6 = reinterpret_cast<const uint8_t*>(addr6());
    return std::equal(MAPPED_IPV4_PREFIX.begin(), MAPPED_IPV4_PREFIX.end(), adress6);
}

SocketAddress SocketAddress::getMappedIPv4() const noexcept
{
    if (!isMappedIPv4())
        return std::move(*this);

    SocketAddress addr;
    sockaddr_in* inaddr4 = (sockaddr_in *)&addr.ss;
    sockaddr_in6* inaddr6 = (sockaddr_in6 *)&ss;

#ifdef __APPLE__
    inaddr4->sin_len = sizeof(sockaddr_in);
#endif
    inaddr4->sin_family = AF_INET;
    inaddr4->sin_port = inaddr6->sin6_port;

    std::memcpy((uint8_t *)&inaddr4->sin_addr,
            ((const uint8_t *)&inaddr6->sin6_addr) + MAPPED_IPV4_PREFIX.size(),
            sizeof(in_addr));

    return addr;
}

SocketAddress SocketAddress::getMappedIPv6() const noexcept
{
    if (ss.ss_family != AF_INET)
        return std::move(*this);

    SocketAddress addr;
    sockaddr_in* inaddr4 = (sockaddr_in *)&ss;
    sockaddr_in6* inaddr6 = (sockaddr_in6 *)&addr.ss;

#ifdef __APPLE__
    inaddr6->sin6_len = sizeof(sockaddr_in6);
#endif
    inaddr6->sin6_family = AF_INET6;
    inaddr6->sin6_port = inaddr4->sin_port;

    std::memcpy((uint8_t *)&inaddr6->sin6_addr, MAPPED_IPV4_PREFIX.data(), MAPPED_IPV4_PREFIX.size());
    std::memcpy(((uint8_t *)&inaddr6->sin6_addr) + MAPPED_IPV4_PREFIX.size(),
            (const uint8_t *)&inaddr4->sin_addr, sizeof(in_addr));

    return addr;
}

std::string SocketAddress::toString(const sockaddr* sa) noexcept
{
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    const void *inaddr;
    in_port_t port;
    switch (sa->sa_family) {
    case AF_INET:
        inaddr = &(((const sockaddr_in*)sa)->sin_addr);
        port = ((const sockaddr_in*)sa)->sin_port;
        break;
    case AF_INET6:
        inaddr = &(((const sockaddr_in6*)sa)->sin6_addr);
        port = ((const sockaddr_in6*)sa)->sin6_port;
        break;
    default:
        return NA;
    }

    if (!inet_ntop(sa->sa_family, inaddr, host, sizeof(host)))
        return NA;

    std::string address;
    address.reserve(std::strlen(host) + NI_MAXSERV + 1);

    address.append(host);
    if (port) {
        port = ntohs(port);
        address.append(":").append(std::to_string(port));
    }

    return address;
}

bool SocketAddress::IpCompare::operator()(const SocketAddress& a, const SocketAddress& b) const noexcept {
    if (a.empty() && b.empty())
        return true; // both are uninitialized address

    if (a.ss.ss_family != b.ss.ss_family)
        return a.ss.ss_family < b.ss.ss_family;

    socklen_t offset, len;
    switch (a.family()) {
        case AF_INET:
            offset = offsetof(sockaddr_in, sin_addr);
            len = sizeof(in_addr);
            break;
        case AF_INET6:
            offset = offsetof(sockaddr_in6, sin6_addr);
            len = sizeof(in6_addr);
            break;
        default:
            offset = 0;
            len = sizeof(sockaddr_storage);
            break;
    }
    return std::memcmp((const uint8_t*)&a.ss + offset,
                        (const uint8_t*)&b.ss + offset, len) < 0;
}

} // namespace carrier
} // namespace elastos
