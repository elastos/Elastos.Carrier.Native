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
#include <list>

#include <carrier.h>
#include "utils/random_generator.h"

using namespace elastos::carrier;

#if defined(_WIN32) || defined(_WIN64)
    #define PATH_MAX                MAX_PATH
    #define getcwd(buf, size)       _getcwd(buf, size)
#endif

namespace test {
class Utils {
public:
static std::string PATH_SEP;

//getRandomInteger/getRandom/getRandomValue are all getting integer.
static int getRandomInteger(int base);

static int getRandom(int min, int max) {
    return RandomGenerator<int>(min,max)();
}

static int getRandomValue() {
    return getRandom(1, 32768);
}

//getRandomData is getting data in std::vector<uint8>
static std::vector<uint8_t> getRandomData(int length);
static void setRandomBytes(unsigned char* buf, size_t buf_len);

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

static bool addressEquals(std::string address1, std::string address2);

static std::string getLocalIpAddresses();

static std::string getPwdStorage(const std::string dir);
static void removeStorage(const std::string path);
static bool isFileExists(const std::string path);

static uint64_t currentTimeMillis();

static std::vector<uint8_t> getSignData(const Id& nodeId, const Id& proxyId, uint16_t port, const std::string& alt);
};

}
