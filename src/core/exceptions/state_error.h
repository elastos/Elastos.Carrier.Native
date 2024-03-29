
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
#include <stdexcept>

namespace elastos {
namespace carrier {

class StateError: public std::runtime_error
{
public:
    explicit StateError(const std::string& __s) : runtime_error(__s) {}
    explicit StateError(const char* __s)  : runtime_error(__s) {}

    StateError(const StateError&) noexcept = default;
    ~StateError() noexcept = default;
};

inline void checkArgument(bool expression, const std::string& errorMessage) {
    if (!expression) {
        throw std::invalid_argument(errorMessage);
    }
}

inline void checkState(bool expression, const std::string& errorMessage) {
    if (!expression) {
        throw StateError(errorMessage);
    }
}

} // namespace carrier
} // namespace elastos
