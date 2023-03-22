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
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#include "utils/random_generator.h"
#include "carrier/node_info.h"

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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

static int getRandomValue() {
    return getRandom(1, 32768);
}

//getRandomData is getting data in std::vector<uint8>
static std::vector<uint8_t> getRandomData(int length) {
    static constexpr const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!#$%&()*+,./:;<=>?@[]^_`{|}~";
    std::vector<uint8_t> b;
    b.resize(length);
    RandomGenerator<uint8_t> generator(0, (sizeof(chars)/sizeof(char)) - 2);
    std::generate_n(b.begin(), length, generator );
    return b;
}

static void setRandomBytes(unsigned char* buf, size_t buf_len) {
    std::random_device rd;
    std::uniform_int_distribution<uint32_t> rand_int;
    auto first = reinterpret_cast<uint32_t*>(buf);
    auto last = reinterpret_cast<uint32_t*>(buf + buf_len);
    std::generate(first, last, std::bind(rand_int, std::ref(rd)));
}

template<typename T>
static bool arrayEquals(std::list<std::shared_ptr<T>> &t1, std::list<std::shared_ptr<T>> &t2) {
    if (t1.size() != t2.size())
        return false;

    for (auto t : t1) {
        auto result = std::find_if(t2.begin(), t2.end(), [&](std::shared_ptr<T> item) {
            return (*item == *t);
        });
        if (result == t1.end())
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
    std::string ipAddress;
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
            if (strcmp(addressBuffer, "127.0.0.1") != 0) {
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

static void removeStorage(const std::string path) {
    fs::path tmp{path};
    fs::remove_all(tmp);
}

};

}
