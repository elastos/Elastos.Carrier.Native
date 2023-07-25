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

#include <string>
#include <algorithm>
#include <stdexcept>
#include <cassert>

#include <sodium.h>
#include "carrier/signature.h"

namespace elastos {
namespace carrier {

static_assert(Signature::PrivateKey::BYTES == crypto_sign_SECRETKEYBYTES,
    "Inappropriate Signature::PrivateKey size definition.");

static_assert(Signature::PublicKey::BYTES == crypto_sign_PUBLICKEYBYTES,
    "Inappropriate Signature::PublicKey size definition.");

static_assert(Signature::KeyPair::SEED_BYTES == crypto_sign_SEEDBYTES,
    "Inappropriate Signature::KeyPair seed size definition.");

static_assert(Signature::BYTES == crypto_sign_BYTES,
    "Inappropriate Signature size definition.");

static std::string to_hex_string(const uint8_t* bin, size_t len)
{
    std::string hex;

    hex.reserve(len * 2 + 4);
    hex.append("0x");
    sodium_bin2hex(hex.data() + 2, len * 2, bin, len);
    return hex;
}

Signature::PrivateKey::PrivateKey(const Blob& sk)
{
    assert(sk.size() == BYTES);
    if (sk.size() != BYTES)
        throw std::invalid_argument("Invaild raw private key size.");

    std::memcpy(key.data(), sk.ptr(), sk.size());
}

Signature::PrivateKey::operator std::string() const noexcept
{
    return to_hex_string(key.data(), key.size());
}

void Signature::PrivateKey::sign(const Blob& data, Blob& signature) const
{
    assert(signature.size() == Signature::BYTES);
    if (signature.size() != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    crypto_sign_detached(signature.ptr(), nullptr, data.ptr(), data.size(), bytes()); // Always success
}

Signature::PublicKey::PublicKey(const Blob& pk)
{
    assert(pk.size() == BYTES);
    if (pk.size() != BYTES)
        throw std::invalid_argument("Invaild raw public key size.");

    std::memcpy(key.data(), pk.ptr(), pk.size());
}

Signature::PublicKey::operator std::string() const noexcept
{
    return to_hex_string(key.data(), key.size());
}

bool Signature::PublicKey::verify(const Blob& data, const Blob& signature) const
{
    assert(signature.size() == Signature::BYTES);
    if (signature.size() != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    return crypto_sign_verify_detached(signature.ptr(), data.ptr(), data.size(), bytes()) == 0;
}

Signature::KeyPair::KeyPair() noexcept
{
    crypto_sign_keypair(pk.key.data(), sk.key.data()); // Always success
}

Signature::KeyPair::KeyPair(const PrivateKey& sk) noexcept : sk(sk)
{
    crypto_sign_ed25519_sk_to_pk(pk.key.data(), sk.bytes()); // Always success
}

Signature::KeyPair::KeyPair(PrivateKey&& sk) noexcept : sk(std::move(sk))
{
    crypto_sign_ed25519_sk_to_pk(pk.key.data(), sk.bytes()); // Always success
}

Signature::KeyPair::KeyPair(const Blob& sk)
{
    assert(sk.size() == Signature::PrivateKey::BYTES);
    if (sk.size() != Signature::PrivateKey::BYTES)
        throw std::invalid_argument("Invaild raw private key size.");

    std::memcpy(this->sk.key.data(), sk.ptr(), sk.size());
    crypto_sign_ed25519_sk_to_pk(pk.key.data(), sk.ptr()); // Always success
}

// Return KeyPair object maybe will have some copy overhead,
// but easier to use also this is not a  frequently used method
Signature::KeyPair Signature::KeyPair::fromSeed(const Blob& seed)
{
    assert(seed.size() == Signature::KeyPair::SEED_BYTES);
    if (seed.size() != Signature::KeyPair::SEED_BYTES)
        throw std::invalid_argument("Invaild seed size.");

    Signature::KeyPair keypair;
    crypto_sign_seed_keypair(keypair.pk.key.data(), keypair.sk.key.data(), seed.ptr()); // Always success
    return keypair;
}

Signature::KeyPair Signature::KeyPair::random()
{
    //Maybe later will improve the random
    Signature::KeyPair keypair {};
    return keypair;
}

void Signature::reset()
{
    static_assert(sizeof(Signature::SignState) >= sizeof(crypto_sign_state),
        "Inappropriate signature state size.");

    crypto_sign_state *s = (crypto_sign_state*)&state;
    crypto_sign_init(s); // Always success
}

void Signature::update(const Blob& part)
{
    crypto_sign_state *s = (crypto_sign_state*)&state;
    crypto_sign_update(s, part.ptr(), part.size()); // Always success
}

void Signature::sign(Blob& sig, const Signature::PrivateKey& sk) const
{
    assert(sig.size() == Signature::BYTES);
    if (sig.size() != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    crypto_sign_state *s = (crypto_sign_state*)&state;
    crypto_sign_final_create(s, sig.ptr(), nullptr, sk.bytes()); // Always success
}

bool Signature::verify(const Blob& sig, const Signature::PublicKey& pk) const
{
    assert(sig.size() == Signature::BYTES);
    if (sig.size() != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    crypto_sign_state *s = (crypto_sign_state*)&state;
    return crypto_sign_final_verify(s, sig.ptr(), pk.bytes()) == 0;
}

} // namespace carrier
} // namespace elastos
