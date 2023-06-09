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

void Signature::PrivateKey::sign(Blob& sig, const Blob& data) const
{
    assert(sig.size() == Signature::BYTES);
    if (sig.size() != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    crypto_sign_detached(sig.ptr(), nullptr, data.ptr(), data.size(), bytes()); // Always success
}

////////////////////////////////////////////////////////////////////////////////
// Signature::PublicKey
////////////////////////////////////////////////////////////////////////////////
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

bool Signature::PublicKey::verify(const Blob& sig, const Blob& data) const
{
    assert(sig.size() == Signature::BYTES);
    if (sig.size() != Signature::BYTES)
        throw std::invalid_argument("Invalid signature length.");

    return crypto_sign_verify_detached(sig.ptr(), data.ptr(), data.size(), bytes()) == 0;
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

////////////////////////////////////////////////////////////////////////////////
// CryptoBox::PrivateKey
////////////////////////////////////////////////////////////////////////////////

CryptoBox::PrivateKey::PrivateKey(const Blob& sk)
{
    assert(sk.size() == BYTES);
    if (sk.size() != BYTES)
        throw std::invalid_argument("Invaild raw private key size.");

    std::memcpy(key.data(), sk.ptr(), sk.size());
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

CryptoBox::PublicKey::PublicKey(const Blob& pk)
{
    assert(pk.size() == BYTES);
    if (pk.size() != BYTES)
        throw std::invalid_argument("Invaild raw public key size.");

    std::memcpy(key.data(), pk.ptr(), pk.size());
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

CryptoBox::Nonce::Nonce(const Blob& pk)
{
    assert(pk.size() == BYTES);
    if (pk.size() != BYTES)
        throw std::invalid_argument("Invaild raw nonce size.");

    std::memcpy(nonce.data(), pk.ptr(), pk.size());
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

CryptoBox::KeyPair::KeyPair(const Blob& sk)
{
    assert(sk.size() == CryptoBox::PrivateKey::BYTES);
    if (sk.size() != CryptoBox::PrivateKey::BYTES)
        throw std::invalid_argument("Invaild raw private key size.");

    std::memcpy(this->sk.key.data(), sk.ptr(), sk.size());
    crypto_scalarmult_base(pk.key.data(), sk.ptr()); // Always success
}

// Return KeyPair object maybe will have some copy overhead,
// but easier to use also this is not a  frequently used method
CryptoBox::KeyPair CryptoBox::KeyPair::fromSeed(const Blob& seed)
{
    assert(seed.size() == CryptoBox::KeyPair::SEED_BYTES);
    if (seed.size() != CryptoBox::KeyPair::SEED_BYTES)
        throw std::invalid_argument("Invaild seed size.");

    CryptoBox::KeyPair keypair;
    crypto_box_seed_keypair(keypair.pk.key.data(), keypair.sk.key.data(), seed.ptr()); // Always success
    return keypair;
}

CryptoBox::KeyPair CryptoBox::KeyPair::fromSignatureKeyPair(const Signature::KeyPair& signKeyPair)
{
    uint8_t x25519[crypto_box_SECRETKEYBYTES];
    crypto_sign_ed25519_sk_to_curve25519(x25519, signKeyPair.privateKey().bytes()); // Always success
    return {Blob{x25519, sizeof(x25519)}};
}

////////////////////////////////////////////////////////////////////////////////
// CryptoBox
////////////////////////////////////////////////////////////////////////////////

CryptoBox::CryptoBox(const PublicKey &pk, const PrivateKey &sk)
{
    if (crypto_box_beforenm(key.data(), pk.bytes(), sk.bytes()) != 0)
        throw CryptoError("Compute symmetric key failed.");
}

void CryptoBox::encrypt(Blob& cipher, const Blob& plain, const Nonce& nonce) const
{
    assert(cipher.size() >= plain.size() + crypto_box_MACBYTES);
    if (cipher.size() < plain.size() + crypto_box_MACBYTES)
        throw std::invalid_argument("The cipher buffer is too small.");

    if (crypto_box_easy_afternm(cipher.ptr(), plain.ptr(), plain.size(), nonce.bytes(), key.data()) != 0)
        throw CryptoError("Encrypt data failed.");
}

void CryptoBox::encrypt(Blob& cipher, const Blob& plain, const Nonce& nonce,
        const PublicKey& pk, const PrivateKey& sk)
{
    assert(cipher.size() >= plain.size() + crypto_box_MACBYTES);
    if (cipher.size() < plain.size() + crypto_box_MACBYTES)
        throw std::invalid_argument("The cipher buffer is too small.");

    if (crypto_box_easy(cipher.ptr(), plain.ptr(), plain.size(), nonce.bytes(), pk.bytes(), sk.bytes()) != 0)
        throw CryptoError(std::string("Encrypt data failed."));
}

void CryptoBox::decrypt(Blob& plain, const Blob& cipher, const Nonce &nonce) const
{
    assert(plain.size() >= cipher.size() - crypto_box_MACBYTES);
    if (plain.size() < cipher.size() - crypto_box_MACBYTES)
        throw std::invalid_argument("The plain buffer is too small.");

    if (crypto_box_open_easy_afternm(plain.ptr(), cipher.ptr(), cipher.size(), nonce.bytes(), key.data()) != 0)
        throw CryptoError(std::string("Decrypt data failed."));
}

void CryptoBox::decrypt(Blob& plain, const Blob& cipher, const Nonce& nonce, 
        const PublicKey& pk, const PrivateKey& sk)
{
    assert(plain.size() >= cipher.size() - crypto_box_MACBYTES);
    if (plain.size() < cipher.size() - crypto_box_MACBYTES)
        throw std::invalid_argument("The plain buffer is too small.");

    if (crypto_box_open_easy(plain.ptr(), cipher.ptr(), cipher.size(), nonce.bytes(), pk.bytes(), sk.bytes()) != 0)
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
