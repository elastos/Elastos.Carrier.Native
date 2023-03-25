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

#include <array>
#include <vector>

namespace elastos {
namespace carrier {

class Blob {
public:
    Blob() {};
    Blob(const Blob& blob) noexcept: _ptr(blob._ptr), _size(blob._size) {}
    Blob(const Blob&& blob) noexcept: _ptr(blob._ptr), _size(blob._size) {}

    template <size_t N>
    Blob(const std::array<uint8_t, N>& arry) noexcept
        : _ptr(arry.data()), _size(arry.size()) {}

    Blob(const std::vector<uint8_t>& vector) noexcept
        : _ptr(vector.data()), _size(vector.size()) {}

    Blob(const uint8_t* ptr, size_t size) noexcept
        : _ptr(ptr), _size(size) {}

    Blob(const void* ptr, size_t size) noexcept
        : _ptr(reinterpret_cast<const uint8_t *>(ptr)), _size(size) {}

    Blob& operator=(const Blob& target) noexcept {
        this->_ptr = target._ptr;
        this->_size = target._size;
        return *this;
    }

    Blob& operator=(const Blob&& target) noexcept {
        this->_ptr = target._ptr;
        this->_size = target._size;
        return *this;
    }

    operator bool() const noexcept {
        return _ptr != nullptr && _size != 0;
    }

    uint8_t* ptr() noexcept {
        return const_cast<uint8_t*>(this->_ptr);
    }

    const uint8_t* ptr() const noexcept {
        return this->_ptr;
    }

    size_t size() const noexcept {
        return this->_size;
    }

    uint8_t* begin() noexcept {
        return const_cast<uint8_t*>(this->_ptr);
    }

    uint8_t* end() noexcept {
        return const_cast<uint8_t*>(this->_ptr + this->_size);
    }

    const uint8_t* cbegin() const noexcept {
        return this->_ptr;
    }

    const uint8_t* cend() const noexcept {
        return this->_ptr + this->_size;
    }

private:
    const uint8_t* _ptr {};
    size_t _size {0};
};

} // namespace carrier
} // namespace elastos
