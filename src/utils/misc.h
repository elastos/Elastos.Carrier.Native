/*
 * Copyright (c) 2022 - 2023 Elastos Foundation
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
#include <functional>
#include <stdexcept>
#include <cstring>

namespace elastos {
namespace carrier {

#if defined(__linux__)
#define ntohll(x) htobe64(x)
#endif

class DhtException : public std::runtime_error {
public:
    DhtException(const std::string &str = "") :
        std::runtime_error("DhtException occurred: " + str) {}
};

class JsonException : public std::runtime_error {
public:
    JsonException(const std::string &str = "") :
        std::runtime_error("JsonException occurred: " + str) {}
};

class SocketException : public DhtException {
public:
    SocketException(int err) :
        DhtException(strerror(err)) {}
};

} // namespace carrier
} // namespace elastos
