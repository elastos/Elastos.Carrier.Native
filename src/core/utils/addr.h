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

#include <chrono>
#include <random>
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#include "carrier/socket_address.h"

namespace elastos {
namespace carrier {

#if defined(__linux__)
#define ntohll(x) htobe64(x)
#endif

std::string getLocalIpAddresses(bool ipv4 = true) {
    std::string ipAddress {};
    struct ifaddrs* ifAddrStruct = nullptr;
    struct ifaddrs* ifa = nullptr;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        void* tmpAddrPtr = nullptr;
        char addressBuffer[INET_ADDRSTRLEN];
        auto family = ifa->ifa_addr->sa_family;

        if (ipv4 && family == AF_INET)
            tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
        else if (!ipv4 && family == AF_INET6)
            tmpAddrPtr = &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;

        if (tmpAddrPtr) {
            inet_ntop(family, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            auto socketaddress = SocketAddress(addressBuffer);
            if (socketaddress.isAnyUnicast()) {
                ipAddress = std::string(addressBuffer);
                break;
            }
        }
    }
    if (ifAddrStruct != nullptr) {
        freeifaddrs(ifAddrStruct);
    }

    return ipAddress;
}

} // namespace carrier
} // namespace elastos
