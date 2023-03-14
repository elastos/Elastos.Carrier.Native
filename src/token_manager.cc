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

#include <functional>

#include "carrier/socket_address.h"
#include "utils/random_generator.h"
#include "utils/time.h"
#include "token_manager.h"

#ifdef __linux__
#include <endian.h>
#define htonll(n) htobe64(n)
#endif

static const long TOKEN_TIMEOUT = 5 * 60 * 1000;   //5 minutes
using namespace std::chrono;

namespace elastos {
namespace carrier {

TokenManager::TokenManager() {
    RandomGenerator<uint32_t> generator;
    auto a = reinterpret_cast<uint32_t*>(sessionSecret.data());
    auto b = reinterpret_cast<uint32_t*>(sessionSecret.data()+ sessionSecret.size());
    std::generate(a, b, generator);
}

void TokenManager::updateTokenTimestamps() {
    uint64_t current = timestamp.load();
    uint64_t now = currentTimeMillis();
    while (now - current > TOKEN_TIMEOUT) {
        if (timestamp.compare_exchange_weak(current, now)) {
            previousTimestamp = current;
            break;
        }
        current = timestamp.load();
    }
}

int TokenManager::generateToken(const Id& nodeId, const SocketAddress& addr, const Id& targetId, long timestamp, std::array<uint8_t, 32>& sessionSecret) {
    const uint16_t _port = htons(addr.port());
    const uint64_t _stamp = htonll(timestamp);
    const auto port = reinterpret_cast<const uint8_t*>(&_port);
    const auto stamp = reinterpret_cast<const uint8_t*>(&_stamp);

    // nodeId + ip + port + targetId + timestamp + sessionSecret
    auto sha256 = SHA256();
    sha256.update(nodeId.data(), nodeId.size());
    sha256.update(addr.inaddr(), addr.inaddrLength());
    sha256.update(port, sizeof(uint16_t));
    sha256.update(targetId.data(), targetId.size());
    sha256.update(stamp, sizeof(uint64_t));
    sha256.update(sessionSecret.data(), sessionSecret.size());

    auto digest = sha256.digest();
    int pos = (digest[0] & 0xff) & 0x1f; // mod 32
    int token = ((digest[pos] & 0xff) << 24) |
            ((digest[(pos + 1) & 0x1f] & 0xff) << 16) |
            ((digest[(pos + 2) & 0x1f] & 0xff) << 8) |
            (digest[(pos + 3) & 0x1f] & 0xff);

    return token;
}

int TokenManager::generateToken(const Id& nodeId, const SocketAddress& addr, const Id& targetId) {
    updateTokenTimestamps();
    return generateToken(nodeId, addr, targetId, timestamp.load(), sessionSecret);
}

bool TokenManager::verifyToken(int token, const Id& nodeId, const SocketAddress& addr, const Id& targetId) {
    updateTokenTimestamps();

    int currentToken = generateToken(nodeId, addr, targetId, timestamp.load(), sessionSecret);
    if (token == currentToken)
        return true;
    int previousToken = generateToken(nodeId, addr, targetId, previousTimestamp, sessionSecret);
    if (token == previousToken)
        return true;

    return false;
}

}
}
