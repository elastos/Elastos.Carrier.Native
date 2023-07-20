
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

namespace elastos {
namespace carrier {

class _LIBCPP_EXCEPTION_ABI illegal_state
    : public std::runtime_error
{
public:
    _LIBCPP_INLINE_VISIBILITY explicit illegal_state(const std::string& __s) : runtime_error(__s) {}
    _LIBCPP_INLINE_VISIBILITY explicit illegal_state(const char* __s)   : runtime_error(__s) {}

#ifndef _LIBCPP_ABI_VCRUNTIME
    illegal_state(const illegal_state&) _NOEXCEPT = default;
    ~illegal_state() _NOEXCEPT = default;
#endif
};

inline void checkArgument(bool expression, const std::string& errorMessage) {
    if (!expression) {
        throw std::invalid_argument(errorMessage);
    }
}

inline void checkState(bool expression, const std::string& errorMessage) {
    if (!expression) {
        throw illegal_state(errorMessage);
    }
}

} // namespace carrier
} // namespace elastos
