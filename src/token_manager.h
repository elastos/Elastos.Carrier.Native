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

#include <atomic>
#include <array>

#include "carrier/id.h"
#include "carrier/socket_address.h"

namespace elastos {
namespace carrier {

class SocketAddress;

class TokenManager {
public:
    TokenManager();

    int generateToken(const Id& nodeId, const SocketAddress& addr, const Id& targetId);
    bool verifyToken(int token, const Id& nodeId, const SocketAddress& addr, const Id& targetId);

private:
    void updateTokenTimestamps();
    static int generateToken(const Id&, const SocketAddress&, const Id&, long, std::array<uint8_t, 32>&);

    std::atomic_uint64_t timestamp {0};
    uint64_t previousTimestamp;
    std::array<uint8_t, 32> sessionSecret {};
};

} /* namespace carrier */
} /* namespace elastos */
