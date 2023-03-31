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
#include <array>

namespace elastos {
namespace carrier {

class Hex {
public:
    static ssize_t encode(const uint8_t* data, size_t data_size, char* out, size_t out_size);

    static std::string encode(const uint8_t* data, size_t size) {
        std::string out(size * 2, '\0');
        encode(data, size, (char *)out.data(), size * 2 + 1);
        return out;
    }

    static std::string encode(const std::vector<uint8_t>& data) {
        return encode(data.data(), data.size());
    }

    template <size_t S>
    static std::string encode(const std::array<uint8_t, S>& data) {
        return encode(data.data(), data.size());
    }

    static ssize_t decode(const char* data, size_t data_size, uint8_t* out, size_t out_size);

    static std::vector<uint8_t> decode(const char* data, size_t size) {
        std::vector<uint8_t> out(size/2);
        decode(data, size, out.data(), out.size());
        return out;
    }

    template <size_t S>
    static std::array<uint8_t, S> decode(const char* data, size_t size) {
        std::array<uint8_t, S> out;
        decode(data, size, out.data(), out.size());
        return out;
    }

private:
    struct HexMap : public std::array<std::array<char, 2>, 256> {
        HexMap() {
            for (size_t i = 0; i < size(); i++) {
                auto& e = (*this)[i];
                e[0] = hex_digits[(i >> 4) & 0x0F];
                e[1] = hex_digits[i & 0x0F];
            }
        }
        private:
            static constexpr const char* hex_digits = "0123456789abcdef";
    };

    static const HexMap hex_map;
};

} // namespace carrier
} // namespace elastos
