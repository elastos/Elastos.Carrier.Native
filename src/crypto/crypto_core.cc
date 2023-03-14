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

#include <sodium.h>

#include <string>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include "carrier/crypto_core.h"

namespace elastos {
namespace carrier {

static_assert(CryptoBox::PrivateKey::BYTES == crypto_box_SECRETKEYBYTES,
    "error Inappropriate CryptoBox::PrivateKey size definition.");

static_assert(CryptoBox::PublicKey::BYTES == crypto_box_PUBLICKEYBYTES,
    "Inappropriate CryptoBox::PublicKey size definition.");

static_assert(CryptoBox::Nonce::BYTES == crypto_box_NONCEBYTES,
    "Inappropriate CryptoBox::Nonce size definition.");

static_assert(CryptoBox::KeyPair::SEED_BYTES == crypto_box_SEEDBYTES,
    "Inappropriate CryptoBox::KeyPair seed size definition");

static_assert(CryptoBox::SYMMETRIC_KEY_BYTES == crypto_box_BEFORENMBYTES,
    "Inappropriate CryptoBox symmetric key size definition.");

static_assert(CryptoBox::MAC_BYTES == crypto_box_MACBYTES,
    "Inappropriate CryptoBox MAC size definition.");

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

////////////////////////////////////////////////////////////////////////////////
// Signature::PrivateKey
////////////////////////////////////////////////////////////////////////////////

Signature::PrivateKey::PrivateKey(const uint8_t* sk, size_t len)
{
    assert(len == BYTES);
    if (len != BYTES)
        throw std::invalid_argument("Invaild raw private key size.");

    std::memcpy(key.data(), sk, len);
}

Signature::PrivateKey::operator std::string() const noexcept
{
    return to_hex_string(key.data(), key.size());
}

void Signature::PrivateKey::sign(uint8_t* sig, size_t sigLen, const uint8_t* data, size_t dataLen) const
{
    assert(sigLen == Signature::BYTES);
    if (sigLen != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    crypto_sign_detached(sig, nullptr, data, dataLen, bytes()); // Always success
}

////////////////////////////////////////////////////////////////////////////////
// Signature::PublicKey
////////////////////////////////////////////////////////////////////////////////
Signature::PublicKey::PublicKey(const uint8_t* pk, size_t len)
{
    assert(len == BYTES);
    if (len != BYTES)
        throw std::invalid_argument("Invaild raw public key size.");

    std::memcpy(key.data(), pk, len);
}

Signature::PublicKey::operator std::string() const noexcept
{
    return to_hex_string(key.data(), key.size());
}

bool Signature::PublicKey::verify(const uint8_t* sig, size_t sigLen, const uint8_t* data, size_t dataLen) const
{
    assert(sigLen == Signature::BYTES);
    if (sigLen != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    return crypto_sign_verify_detached(sig, data, dataLen, bytes()) == 0;
}

////////////////////////////////////////////////////////////////////////////////
// Signature::KeyPair
////////////////////////////////////////////////////////////////////////////////

Signature::KeyPair::KeyPair() noexcept
{
    crypto_sign_keypair(pk.key.data(), sk.key.data()); // Always success
}

Signature::KeyPair::KeyPair(PrivateKey& sk) noexcept : sk(sk)
{
    crypto_sign_ed25519_sk_to_pk(pk.key.data(), sk.bytes()); // Always success
}

Signature::KeyPair::KeyPair(PrivateKey&& sk) noexcept : sk(std::move(sk))
{
    crypto_sign_ed25519_sk_to_pk(pk.key.data(), sk.bytes()); // Always success
}

Signature::KeyPair::KeyPair(const uint8_t* sk, size_t len)
{
    assert(len == Signature::PrivateKey::BYTES);
    if (len != Signature::PrivateKey::BYTES)
        throw std::invalid_argument("Invaild raw private key size.");

    std::memcpy(this->sk.key.data(), sk, len);
    crypto_sign_ed25519_sk_to_pk(pk.key.data(), sk); // Always success
}

// Return KeyPair object maybe will have some copy overhead,
// but easier to use also this is not a  frequently used method
Signature::KeyPair Signature::KeyPair::fromSeed(const uint8_t* seed, size_t len)
{
    assert(len == Signature::KeyPair::SEED_BYTES);
    if (len != Signature::KeyPair::SEED_BYTES)
        throw std::invalid_argument("Invaild seed size.");

    Signature::KeyPair keypair;
    crypto_sign_seed_keypair(keypair.pk.key.data(), keypair.sk.key.data(), seed); // Always success
    return keypair;
}

////////////////////////////////////////////////////////////////////////////////
// Signature
////////////////////////////////////////////////////////////////////////////////

void Signature::reset()
{
    static_assert(sizeof(Signature::SignState) >= sizeof(crypto_sign_state),
        "Inappropriate signature state size.");

    crypto_sign_state *s = (crypto_sign_state*)&state;
    crypto_sign_init(s); // Always success
}

void Signature::update(const uint8_t* part, size_t len)
{
    crypto_sign_state *s = (crypto_sign_state*)&state;
    crypto_sign_update(s, part, len); // Always success
}

void Signature::sign(uint8_t* sig, size_t len, const Signature::PrivateKey& sk) const
{
    assert(len == Signature::BYTES);
    if (len != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    crypto_sign_state *s = (crypto_sign_state*)&state;
    crypto_sign_final_create(s, sig, nullptr, sk.bytes()); // Always success
}

bool Signature::verify(const uint8_t* sig, size_t len, const Signature::PublicKey& pk) const
{
    assert(len == Signature::BYTES);
    if (len != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    crypto_sign_state *s = (crypto_sign_state*)&state;
    return crypto_sign_final_verify(s, sig, pk.bytes()) == 0;
}

////////////////////////////////////////////////////////////////////////////////
// CryptoBox::PrivateKey
////////////////////////////////////////////////////////////////////////////////

CryptoBox::PrivateKey::PrivateKey(const uint8_t* sk, size_t len)
{
    assert(len == BYTES);
    if (len != BYTES)
        throw std::invalid_argument("Invaild raw private key size.");

    std::memcpy(key.data(), sk, len);
}

CryptoBox::PrivateKey::operator std::string() const noexcept
{
    return to_hex_string(key.data(), key.size());
}

CryptoBox::PrivateKey CryptoBox::PrivateKey::fromSignatureKey(const Signature::PrivateKey& signSk)
{
    CryptoBox::PrivateKey sk;
    crypto_sign_ed25519_sk_to_curve25519(sk.key.data(), signSk.bytes()); // Always success
    return sk;
}

////////////////////////////////////////////////////////////////////////////////
// CryptoBox::PublicKey
////////////////////////////////////////////////////////////////////////////////

CryptoBox::PublicKey::PublicKey(const uint8_t* pk, size_t len)
{
    assert(len == BYTES);
    if (len != BYTES)
        throw std::invalid_argument("Invaild raw public key size.");

    std::memcpy(key.data(), pk, len);
}

CryptoBox::PublicKey::operator std::string() const noexcept
{
    return to_hex_string(key.data(), key.size());
}

CryptoBox::PublicKey CryptoBox::PublicKey::fromSignatureKey(const Signature::PublicKey& signPk)
{
    CryptoBox::PublicKey pk;

    if (crypto_sign_ed25519_pk_to_curve25519(pk.key.data(), signPk.bytes()) != 0)
        throw CryptoError("converts Ed25519 key to x25519 key failed.");

    return pk;
}

////////////////////////////////////////////////////////////////////////////////
// CryptoBox::Nonce
////////////////////////////////////////////////////////////////////////////////

CryptoBox::Nonce::Nonce(const uint8_t* pk, size_t len)
{
    assert(len == BYTES);
    if (len != BYTES)
        throw std::invalid_argument("Invaild raw nonce size.");

    std::memcpy(nonce.data(), pk, len);
}

CryptoBox::Nonce& CryptoBox::Nonce::increment() noexcept
{
    sodium_increment(nonce.data(), BYTES);
    return *this;
}

CryptoBox::Nonce& CryptoBox::Nonce::random() noexcept
{
    randombytes_buf(nonce.data(), BYTES);
    return *this;
}

CryptoBox::Nonce::operator std::string() const noexcept
{
    return to_hex_string(nonce.data(), nonce.size());
}

////////////////////////////////////////////////////////////////////////////////
// CryptoBox::KeyPair
////////////////////////////////////////////////////////////////////////////////

CryptoBox::KeyPair::KeyPair() noexcept
{
    crypto_box_keypair(pk.key.data(), sk.key.data()); // Always success
}

CryptoBox::KeyPair::KeyPair(PrivateKey& sk) noexcept : sk(sk)
{
    crypto_scalarmult_base(pk.key.data(), sk.bytes()); // Always success
}

CryptoBox::KeyPair::KeyPair(PrivateKey&& sk) noexcept : sk(sk)
{
    crypto_scalarmult_base(pk.key.data(), sk.bytes()); // Always success
}

CryptoBox::KeyPair::KeyPair(const uint8_t* sk, size_t len)
{
    assert(len == CryptoBox::PrivateKey::BYTES);
    if (len != CryptoBox::PrivateKey::BYTES)
        throw std::invalid_argument("Invaild raw private key size.");

    std::memcpy(this->sk.key.data(), sk, len);
    crypto_scalarmult_base(pk.key.data(), sk); // Always success
}

// Return KeyPair object maybe will have some copy overhead,
// but easier to use also this is not a  frequently used method
CryptoBox::KeyPair CryptoBox::KeyPair::fromSeed(const uint8_t* seed, size_t len)
{
    assert(len == CryptoBox::KeyPair::SEED_BYTES);
    if (len != CryptoBox::KeyPair::SEED_BYTES)
        throw std::invalid_argument("Invaild seed size.");

    CryptoBox::KeyPair keypair;
    crypto_box_seed_keypair(keypair.pk.key.data(), keypair.sk.key.data(), seed); // Always success
    return keypair;
}

CryptoBox::KeyPair CryptoBox::KeyPair::fromSignatureKeyPair(const Signature::KeyPair& signKeyPair)
{
    uint8_t x25519[crypto_box_SECRETKEYBYTES];
    crypto_sign_ed25519_sk_to_curve25519(x25519, signKeyPair.privateKey().bytes()); // Always success
    return {x25519, sizeof(x25519)};
}

////////////////////////////////////////////////////////////////////////////////
// CryptoBox
////////////////////////////////////////////////////////////////////////////////

CryptoBox::CryptoBox(const PublicKey &pk, const PrivateKey &sk)
{
    if (crypto_box_beforenm(key.data(), pk.bytes(), sk.bytes()) != 0)
        throw CryptoError("Compute symmetric key failed.");
}

void CryptoBox::encrypt(uint8_t* cipher, size_t cipherLen,
        const uint8_t* plain, size_t plainLen, const Nonce& nonce) const
{
    assert(cipherLen >= plainLen + crypto_box_MACBYTES);
    if (cipherLen < plainLen + crypto_box_MACBYTES)
        throw std::invalid_argument("The cipher buffer is too small.");

    if (crypto_box_easy_afternm(cipher, plain, plainLen, nonce.bytes(), key.data()) != 0)
        throw CryptoError("Encrypt data failed.");
}

void CryptoBox::encrypt(uint8_t* cipher, size_t cipherLen, const uint8_t* plain, size_t plainLen,
            const Nonce& nonce, const PublicKey& pk, const PrivateKey& sk)
{
    assert(cipherLen >= plainLen + crypto_box_MACBYTES);
    if (cipherLen < plainLen + crypto_box_MACBYTES)
        throw std::invalid_argument("The cipher buffer is too small.");

    if (crypto_box_easy(cipher, plain, plainLen, nonce.bytes(), pk.bytes(), sk.bytes()) != 0)
        throw CryptoError(std::string("Encrypt data failed."));
}

void CryptoBox::decrypt(uint8_t* plain, size_t plainLen,
    const uint8_t *cipher, size_t cipherLen, const Nonce &nonce) const
{
    assert(plainLen >= cipherLen - crypto_box_MACBYTES);
    if (plainLen < cipherLen - crypto_box_MACBYTES)
        throw std::invalid_argument("The plain buffer is too small.");

    if (crypto_box_open_easy_afternm(plain, cipher, cipherLen, nonce.bytes(), key.data()) != 0)
        throw CryptoError(std::string("Decrypt data failed."));
}

void CryptoBox::decrypt(uint8_t* plain, size_t plainLen, const uint8_t* cipher, size_t cipherLen,
        const Nonce& nonce, const PublicKey& pk, const PrivateKey& sk)
{
    assert(plainLen >= cipherLen - crypto_box_MACBYTES);
    if (plainLen < cipherLen - crypto_box_MACBYTES)
        throw std::invalid_argument("The plain buffer is too small.");

    if (crypto_box_open_easy(plain, cipher, cipherLen, nonce.bytes(), pk.bytes(), sk.bytes()) != 0)
        throw CryptoError(std::string("Decrypt data failed."));
}

////////////////////////////////////////////////////////////////////////////////
// SHA256
////////////////////////////////////////////////////////////////////////////////

void SHA256::reset()
{
    static_assert(sizeof(SHA256::DigestState) >= sizeof(crypto_hash_sha256_state),
        "Inappropriate SHA256 digest state size.");

    crypto_hash_sha256_state* s = (crypto_hash_sha256_state*)&state;
    crypto_hash_sha256_init(s); // Always success
}

void SHA256::update(const uint8_t* part, size_t len)
{
    crypto_hash_sha256_state* s = (crypto_hash_sha256_state*)&state;
    crypto_hash_sha256_update(s, part, len); // Always success
}

void SHA256::digest(uint8_t *hash, size_t len)
{
    assert(len == SHA256::BYTES);
    if (len != SHA256::BYTES)
        throw std::invalid_argument("Invaild hash size.");

    crypto_hash_sha256_state* s = (crypto_hash_sha256_state*)&state;
    crypto_hash_sha256_final(s, hash); // Always success
}

void SHA256::digest(uint8_t *hash, size_t hashLen, const uint8_t* data, size_t dataLen)
{
    assert(hashLen == SHA256::BYTES);
    if (hashLen != SHA256::BYTES)
        throw std::invalid_argument("Invaild hash size.");

    crypto_hash_sha256(hash, data, dataLen); // Always success
}

////////////////////////////////////////////////////////////////////////////////
// Random
////////////////////////////////////////////////////////////////////////////////

uint8_t Random::uint8()
{
    return (uint8_t)randombytes_uniform(UINT8_MAX + 1);
}

uint8_t Random::uint8(uint8_t upbound)
{
    return (uint8_t)randombytes_uniform(upbound);
}

uint16_t Random::uint16()
{
    return (uint16_t)randombytes_uniform(UINT16_MAX + 1);
}

uint16_t Random::uint16(uint16_t upbound)
{
    return (uint16_t)randombytes_uniform(upbound);
}

uint32_t Random::uint32()
{
    return randombytes_random();
}

uint32_t Random::uint32(uint32_t upbound)
{
    return randombytes_uniform(upbound);
}

uint64_t Random::uint64()
{
    return ((uint64_t)randombytes_random() << 32) | (uint64_t)randombytes_random();
}

uint64_t Random::uint64(uint64_t upbound)
{
    return (((uint64_t)randombytes_random() << 32) | (uint64_t)randombytes_random()) % upbound;
}

void Random::buffer(void* buf, size_t length)
{
    randombytes_buf(buf, length);
}

}
}
