/*
 * Copyright (c) 2023 trinity-tech.io
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

#include <chrono>
#include <random>
#include <cstring>
#include <sys/types.h>

#ifndef _WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <WinNT.h>
#endif

#include "carrier/socket_address.h"
#include "addr.h"

namespace elastos {
namespace carrier {

#if defined(__linux__)
#define ntohll(x) htobe64(x)
#endif

CARRIER_PUBLIC std::string getLocalIpAddresses(bool ipv4) {
    std::string ipAddress {};

#if defined(_WIN32) || defined(_WIN64)
    DWORD dwRetVal = 0;
    unsigned int i = 0;

    ULONG family = AF_UNSPEC;

    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    LPVOID lpMsgBuf = NULL;
    ULONG outBufLen = 0;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;

    outBufLen = sizeof(IP_ADAPTER_ADDRESSES);
    pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);

    // Make an initial call to GetAdaptersAddresses to get the
    // size needed into the outBufLen variable
    if (GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen)
            == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
    }

    if (pAddresses == NULL)
        return ipAddress;

    family = ipv4 ? AF_INET : AF_INET6;

    dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);
    if (dwRetVal == NO_ERROR) {
        pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            for (i = 0; pAnycast != NULL; i++) {
                pAnycast = pCurrAddresses->FirstAnycastAddress;
                while(pAnycast) {
                    SOCKET_ADDRESS address = pAnycast->Address;

                    char ip[INET6_ADDRSTRLEN];
                    int len = INET6_ADDRSTRLEN;
                    int ret = WSAAddressToString(address.lpSockaddr, address.iSockaddrLength, NULL, ip, (LPDWORD)&len);
                    if (ret != 0) {
                        pAnycast = pAnycast->Next;
                    } else {
                        ipAddress = std::string(ip);
                        break;
                    }
                }
            }

            if (!ipAddress.empty())
                break;

            pCurrAddresses = pCurrAddresses->Next;
        }
    }

    free(pAddresses);

#else
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
#endif
    return ipAddress;
}

} // namespace carrier
} // namespace elastos
