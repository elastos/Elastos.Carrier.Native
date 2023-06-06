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

#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <functional>
#include <random>

#ifndef _WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <limits.h>
#else
#include <random>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <WinNT.h>
#include <iostream>
#include <iomanip>
#include <direct.h>
#endif

#include <cstring>
#include <vector>
#include <sys/types.h>
#include <filesystem>

#include "carrier/node_info.h"
#include "utils.h"

namespace fs = std::filesystem;

namespace test {

#if defined(_WIN32) || defined(_WIN64)
    std::string Utils::PATH_SEP = "\\";
#else
    std::string Utils::PATH_SEP = "/";
#endif

int Utils::getRandomInteger(int base) {
    std::srand(std::time(nullptr));
    int v = std::rand() % base;
    return (v == 0) ? v + 1 : v;
}

void Utils::setRandomBytes(unsigned char* buf, size_t buf_len) {
    std::random_device rd;
    RandomGenerator<uint32_t> generator;
    auto first = reinterpret_cast<uint32_t*>(buf);
    auto last = reinterpret_cast<uint32_t*>(buf + buf_len);
    std::generate(first, last, generator);
}

std::vector<uint8_t> Utils::getRandomData(int length) {
    static constexpr const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!#$%&()*+,./:;<=>?@[]^_`{|}~";
    std::vector<uint8_t> b;
    b.resize(length);
    setRandomBytes(b.data(), b.size());
    return b;
}

bool Utils::addressEquals(std::string address1, std::string address2) {
    char buf1[sizeof(struct in6_addr)] = {0};
    char buf2[sizeof(struct in6_addr)] = {0};
    sa_family_t family1;
    sa_family_t family2;
    size_t len;

    if (address1.find(".") != std::string::npos) {
        family1 = AF_INET;
        len = sizeof(struct in_addr);
    } else {
        family1 = AF_INET6;
        len = sizeof(struct in6_addr);
    }

    if (address2.find(".") != std::string::npos) {
        family2 = AF_INET;
    } else {
        family2 = AF_INET6;
    }

    if (family1 != family2)
        return false;

    inet_pton(family1, address1.c_str(), buf1);
    inet_pton(family2, address2.c_str(), buf2);

    return std::strncmp(buf1, buf2, len) == 0;
}

std::string Utils::getPwdStorage(const std::string dir) {
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    return dir.empty() ? std::string(cwd) : std::string(cwd) + PATH_SEP + dir;
}

void Utils::removeStorage(const std::string path) {
    fs::path tmp{path};
    fs::remove_all(tmp);
}

bool Utils::isFileExists(const std::string path) {
    fs::path tmp{path};
    return fs::exists(fs::status(tmp));
}

uint64_t Utils::currentTimeMillis() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = ms.time_since_epoch();
    return  value.count();
}

std::string Utils::getLocalIpAddresses() {
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
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
    PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;

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

    family = AF_INET;

    dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);
    if (dwRetVal == NO_ERROR) {
        pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            pUnicast = pCurrAddresses->FirstUnicastAddress;
            for (i = 0; pUnicast != NULL; i++) {
                SOCKET_ADDRESS address = pUnicast->Address;

                char ip[INET6_ADDRSTRLEN];
                int len = INET6_ADDRSTRLEN;
                int ret = WSAAddressToString(address.lpSockaddr, address.iSockaddrLength, NULL, ip, (LPDWORD)&len);
                if (ret != 0) {
                    pUnicast = pUnicast->Next;
                } else {
                    ipAddress = std::string(ip);
                    break;
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
    void* tmpAddrPtr = nullptr;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) {
            tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
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

}