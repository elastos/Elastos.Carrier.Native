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

#include <string>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <cstring>

#include "carrier/blob.h"

#if defined(_WIN32) || defined(_WIN64)
typedef ptrdiff_t       ssize_t;
#endif

namespace elastos {
namespace carrier {

class Hex {
public:
    // convert hex string to binary data
    static char* encode(const uint8_t* data, size_t length, char* buf, size_t bufLen);

    static std::string encode(const uint8_t* data, size_t length, bool needPrefix = false) {
        assert(length > 0);

        char buf[length * 2 + 4];
        int pos {0};

        if (needPrefix) {
            memcpy(buf, "0x", 2);
            pos += 2;
        }

        encode(data, length, buf + pos, sizeof(buf) - pos);
        return std::string(buf);
    }

    static std::string encode(const Blob& data, bool needPrefix = false) {
        return encode(data.ptr(), data.size(), needPrefix);
    }

    static std::string encode(const std::vector<uint8_t>& data) {
        return encode(data.data(), data.size());
    }

    // convert binary data to hex string
    static int decode(const char* data, size_t length, uint8_t* buf, size_t bufLen);

    static std::vector<uint8_t> decode(const std::string& str) {
        assert(str.length() > 0);

        std::vector<uint8_t> out(str.length()/2);
        decode(str.data(), str.length(), out.data(), out.size());
        return out;
    }
};

} // namespace carrier
} // namespace elastos
