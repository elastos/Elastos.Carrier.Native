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

#include "utils/hex.h"
#include <stdexcept>

namespace elastos {
namespace carrier {

const Hex::HexMap Hex::hex_map = {};

ssize_t Hex::encode(const uint8_t* data, size_t data_size, char* out, size_t out_size)
{
    if (out_size < data_size * 2 + 1)
        return -1;

    for (size_t i = 0; i < data_size; i++) {
        auto b = out + i * 2;
        const auto& m = hex_map[data[i]];
        *((uint16_t*)b) = *((uint16_t*)&m);
    }

    out[data_size * 2] = 0;
    return data_size * 2;
}

ssize_t Hex::decode(const char* data, size_t data_size, uint8_t* out, size_t out_size)
{
    if (data_size % 2 != 0 || out_size < data_size / 2)
        return -1;

    auto hex2bin = [](char c) -> uint8_t {
        if (c >= 'a' and c <= 'f')
            return 10 + c - 'a';
        else if (c >= 'A' and c <= 'F')
            return 10 + c - 'A';
        else if (c >= '0' and c <= '9')
            return c - '0';
        else
            throw std::domain_error("not an hex character");
    };

    for (size_t i = 0; i < (data_size / 2); i++)
        out[i] = (hex2bin(data[2*i]) << 4) | hex2bin(data[2*i+1]);

    return data_size / 2;
}

} // namespace carrier
} // namespace elastos
