/*
 * Copyright (c) 2022 trinity-tech.io
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
namespace activeproxy {

class networking_error : public std::runtime_error
{
public:
    explicit networking_error(const std::string& what) : runtime_error(what) {}
    explicit networking_error(const char* what) : runtime_error(what) {}

    networking_error(const networking_error&) noexcept = default;
    virtual ~networking_error() noexcept = default;
};

class state_error : public std::runtime_error
{
public:
    explicit state_error(const std::string& what) : runtime_error(what) {}
    explicit state_error(const char* what) : runtime_error(what) {}

    state_error(const state_error&) noexcept = default;
    virtual ~state_error() noexcept = default;
};

} // namespace activeproxy
} // namespace carrier
} // namespace elastos
