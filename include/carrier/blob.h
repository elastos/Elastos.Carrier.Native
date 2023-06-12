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

#include "def.h"

namespace elastos {
namespace carrier {

/**
 * @brief 封装二进制数据包
 *
 */
class CARRIER_PUBLIC Blob {
public:
    /**
     * @brief 创建空Blob
     *
     */
    Blob() {};
    /**
     * @brief 创建新Blob
     *
     * @param blob Blob二进制内容
     */
    Blob(const Blob& blob) noexcept: _ptr(blob._ptr), _size(blob._size) {}
    /**
     * @brief 创建新Blob
     *
     * @param blob Blob二进制内容
     */
    Blob(Blob&& blob) noexcept: _ptr(blob._ptr), _size(blob._size) {}

    /**
     * @brief 创建新Blob
     *
     * @tparam N 返回Blob二进制包长度
     * @param arry 返回Blob二进制包内容
     */
    template <size_t N>
    Blob(const std::array<uint8_t, N>& arry) noexcept
        : _ptr(arry.data()), _size(arry.size()) {}

    /**
     * @brief 创建新Blob
     *
     * @param vector 返回Blob二进制内容
     */
    Blob(const std::vector<uint8_t>& vector) noexcept
        : _ptr(vector.data()), _size(vector.size()) {}

    /**
     * @brief 创建新Blob
     *
     * @param ptr 返回二进制数据包指针
     * @param size 返回二进制数据包大小
     */
    Blob(const uint8_t* ptr, size_t size) noexcept
        : _ptr(ptr), _size(size) {}

    /**
     * @brief 创建新Blob
     *
     * @param ptr 返回二进制数据包指针
     * @param size 返回二进制数据包大小
     */
    Blob(const void* ptr, size_t size) noexcept
        : _ptr(reinterpret_cast<const uint8_t *>(ptr)), _size(size) {}

    /**
     * @brief 复制target Blob内容
     *
     * @param target 被复制的Blob
     * @return Blob& 返回已复制内容的Blob对象
     */
    Blob& operator=(const Blob& target) noexcept {
        this->_ptr = target._ptr;
        this->_size = target._size;
        return *this;
    }

    /**
     * @brief 复制target Blob内容
     *
     * @param target 被复制的Blob
     * @return Blob& 返回已复制内容的Blob对象
     */
    Blob& operator=(Blob&& target) noexcept {
        this->_ptr = target._ptr;
        this->_size = target._size;
        return *this;
    }

    /**
     * @brief 判断Blob是否为有效Blob
     *
     * @return true 含二进制内容的有效Blob
     * @return false 无二进制内容的无效Blob
     */
    operator bool() const noexcept {
        return _ptr != nullptr && _size != 0;
    }

    /**
     * @brief Blob内容指针
     *
     * @return uint8_t* 返回Blob内容指针
     */
    uint8_t* ptr() noexcept {
        return const_cast<uint8_t*>(this->_ptr);
    }

    /**
     * @brief Blob内容指针
     *
     * @return const uint8_t* 返回Blob内容指针
     */
    const uint8_t* ptr() const noexcept {
        return this->_ptr;
    }

    /**
     * @brief Blob内容长度
     *
     * @return size_t 返回Blob内容长度
     */
    size_t size() const noexcept {
        return this->_size;
    }

    /**
     * @brief Blob头指针，指向Blob第一个二进制
     *
     * @return uint8_t* 返回Blob头指针
     */
    uint8_t* begin() noexcept {
        return const_cast<uint8_t*>(this->_ptr);
    }

    /**
     * @brief Blob尾指针，指向Blob最后二进制
     *
     * @return uint8_t* 返回Blob尾指针
     */
    uint8_t* end() noexcept {
        return const_cast<uint8_t*>(this->_ptr + this->_size);
    }

    /**
     * @brief Blob头指针，指向Blob第一个二进制
     *
     * @return const uint8_t* 返回Blob头指针
     */
    const uint8_t* cbegin() const noexcept {
        return this->_ptr;
    }

    /**
     * @brief Blob尾指针，指向Blob最后二进制
     *
     * @return const uint8_t* 返回Blob尾指针
     */
    const uint8_t* cend() const noexcept {
        return this->_ptr + this->_size;
    }

private:
    /**
     * @brief 二进制包指针
     *
     */
    const uint8_t* _ptr {};

    /**
     * @brief 二进制包大小
     *
     */
    size_t _size {0};
};

} // namespace carrier
} // namespace elastos
