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

#include <sodium.h>
#include "crypto/hex.h"

namespace elastos {
namespace carrier {

ssize_t Hex::encode(const uint8_t* data, size_t length, char* buf, size_t bufLen)
{
    if (bufLen < length *2 + 1)
        return -1;
    sodium_bin2hex(buf,  bufLen, data, length);
    return length * 2;
}

ssize_t Hex::decode(const char* data, size_t length, uint8_t* buf, size_t bufLen)
{
    if (bufLen % 2 != 0 || bufLen < length /2 )
        return -1;

    auto rc =  sodium_hex2bin(buf, bufLen, data, length, nullptr, nullptr, nullptr);
    if (rc == -1)
        throw std::domain_error("not an hex character");

    return length / 2;
}

} // namespace carrier
} // namespace elastos
