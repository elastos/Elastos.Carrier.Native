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
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <sodium.h>
#include "carrier/crypto_box.h"
#include "crypto/random.h"
#include "crypto/hex.h"
#include "exceptions/crypto_error.h"

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

CryptoBox::PrivateKey::PrivateKey(const Blob& sk)
{
    if (sk.size() != BYTES)
        throw std::invalid_argument("Invaild raw private key size.");

    std::memcpy(key.data(), sk.ptr(), sk.size());
}

CryptoBox::PrivateKey::operator std::string() const noexcept
{
    return Hex::encode(key.data(), key.size(), true);
}

CryptoBox::PrivateKey CryptoBox::PrivateKey::fromSignatureKey(const Signature::PrivateKey& signSk)
{
    CryptoBox::PrivateKey sk;
    crypto_sign_ed25519_sk_to_curve25519(sk.key.data(), signSk.bytes()); // Always success
    return sk;
}

CryptoBox::PublicKey::PublicKey(const Blob& pk)
{
    assert(pk.size() == BYTES);
    if (pk.size() != BYTES)
        throw std::invalid_argument("Invaild raw public key size.");

    std::memcpy(key.data(), pk.ptr(), pk.size());
}

CryptoBox::PublicKey::operator std::string() const noexcept
{
    return Hex::encode(key.data(), key.size(), true);
}

CryptoBox::PublicKey CryptoBox::PublicKey::fromSignatureKey(const Signature::PublicKey& signPk)
{
    CryptoBox::PublicKey pk;

    if (crypto_sign_ed25519_pk_to_curve25519(pk.key.data(), signPk.bytes()) != 0)
        throw CryptoError("converts Ed25519 key to x25519 key failed.");

    return pk;
}

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

CryptoBox::Nonce CryptoBox::Nonce::random()
{
    Nonce nonce {};
    Blob blob = nonce.blob();
    Random::buffer(blob);
    return nonce;
}

CryptoBox::Nonce::operator std::string() const noexcept
{
    return Hex::encode(nonce.data(), nonce.size(), true);
}

CryptoBox::KeyPair::KeyPair() noexcept
{
    crypto_box_keypair(pk.key.data(), sk.key.data()); // Always success
}

CryptoBox::KeyPair::KeyPair(const PrivateKey& sk) noexcept : sk(sk)
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

} // namespace carrier
} // namespace elastos
