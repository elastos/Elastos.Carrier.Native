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
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <stdexcept>

#include <sodium.h>
#include "shasum.h"

namespace elastos {
namespace carrier {

void SHA256::reset()
{
    static_assert(sizeof(SHA256::DigestState) >= sizeof(crypto_hash_sha256_state),
        "Inappropriate SHA256 digest state size.");

    crypto_hash_sha256_state* s = (crypto_hash_sha256_state*)&state;
    crypto_hash_sha256_init(s); // Always success
}

void SHA256::update(const Blob& part)
{
    crypto_hash_sha256_state* s = (crypto_hash_sha256_state*)&state;
    crypto_hash_sha256_update(s, part.ptr(), part.size()); // Always success
}

void SHA256::digest(Blob& hash)
{
    assert(hash.size() == SHA256::BYTES);
    if (hash.size() != SHA256::BYTES)
        throw std::invalid_argument("Invaild hash size.");

    crypto_hash_sha256_state* s = (crypto_hash_sha256_state*)&state;
    crypto_hash_sha256_final(s, hash.ptr()); // Always success
}

void SHA256::digest(Blob& hash, const Blob& data)
{
    assert(hash.size() == SHA256::BYTES);
    if (hash.size() != SHA256::BYTES)
        throw std::invalid_argument("Invaild hash size.");

    crypto_hash_sha256(hash.ptr(), data.ptr(), data.size()); // Always success
}

} // namespace carrier
} // namespace elastos
