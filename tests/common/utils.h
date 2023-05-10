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

#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <functional>
#include <random>
#include <list>

#ifndef _WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#else
#include <random>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <WinNT.h>
#include <iostream>
#include <iomanip>
#endif

#include <cstring>
#include <vector>
#include <sys/types.h>
#include <filesystem>

#include "carrier/node_info.h"
#include "utils/random_generator.h"

using namespace elastos::carrier;
namespace fs = std::filesystem;

namespace test {

class Utils {
public:
//getRandomInteger/getRandom/getRandomValue are all getting integer.
static int getRandomInteger(int base) {
    std::srand(std::time(nullptr));
    int v = std::rand() % base;
    return (v == 0) ? v + 1 : v;
}

static int getRandom(int min, int max) {
    return RandomGenerator<int>(min,max)();
}

static int getRandomValue() {
    return getRandom(1, 32768);
}

//getRandomData is getting data in std::vector<uint8>
static std::vector<uint8_t> getRandomData(int length) {
    static constexpr const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!#$%&()*+,./:;<=>?@[]^_`{|}~";
    std::vector<uint8_t> b;
    b.resize(length);
    setRandomBytes(b.data(), b.size());
    return b;
}

static void setRandomBytes(unsigned char* buf, size_t buf_len) {
    std::random_device rd;
    RandomGenerator<uint32_t> generator;
    auto first = reinterpret_cast<uint32_t*>(buf);
    auto last = reinterpret_cast<uint32_t*>(buf + buf_len);
    std::generate(first, last, generator);
}

template<typename T>
static bool arrayEquals(std::list<std::shared_ptr<T>> &t1, std::list<std::shared_ptr<T>> &t2) {
    if (t1.size() != t2.size())
        return false;

    for (auto tt1 : t1) {
        bool equals = false;
        for (auto tt2 : t2) {
            if (*tt1 == *tt2) {
                equals = true;
                break;
            }
        }

        if (!equals)
            return false;
    }

    return true;
}

static bool addressEquals(std::string address1, std::string address2) {
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

static std::string getLocalIpAddresses() {
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

    family = AF_INET;

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

static void removeStorage(const std::string path) {
    fs::path tmp{path};
    fs::remove_all(tmp);
}

static bool isFileExists(const std::string path) {
    fs::path tmp{path};
    return fs::exists(fs::status(tmp));
}

static std::string getPwdStorage(const std::string dir) {
    return getenv("PWD") + dir;
}

};

}
