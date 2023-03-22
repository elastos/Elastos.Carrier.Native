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

#include <array>
#include <vector>
#include <memory>
#include <cstring>
#include <stdexcept>
#include "def.h"

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC Signature {
public:
    class KeyPair;

    class PrivateKey {
    public:
        static const uint32_t BYTES { 64 };

        PrivateKey() noexcept {}

        PrivateKey(const uint8_t* sk, size_t len);

        PrivateKey(const PrivateKey& o) noexcept :
                PrivateKey(o.key.data(), o.key.size()) {}
        PrivateKey(PrivateKey&& o) noexcept :
                PrivateKey(o.key.data(), o.key.size()) {
            o.clear();
        }

        ~PrivateKey() noexcept {
            clear();
        }

        explicit operator bool() const noexcept {
            return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
        }

        bool operator==(const PrivateKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) == 0;
        }

        bool operator!=(const PrivateKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) != 0;
        }

        PrivateKey& operator=(const PrivateKey& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            return *this;
        }

        PrivateKey& operator=(PrivateKey&& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            o.clear();
            return *this;
        }

        const uint8_t* cbegin() const noexcept {
            return key.cbegin();
        }

        const uint8_t* cend() const noexcept {
            return key.cend();
        }

        const uint8_t* bytes() const noexcept {
            return key.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        void clear() noexcept {
            key.fill(0);
        }

        void sign(uint8_t* sig, size_t sigLen, const uint8_t* data, size_t dataLen) const;

        template<typename ST, typename DT>
        void sign(ST& sig, const DT& data) const {
            return sign(sig.data(), sig.size(), data.data(), data.size());
        }

        template<typename DT>
        std::vector<uint8_t> sign(const DT& data) const {
            std::vector<uint8_t> sig(Signature::BYTES);
            sign(sig.data(), sig.size(), data.data(), data.size());
            return sig;
        }

        operator std::string() const noexcept;

        friend class KeyPair;

    private:
        std::array<uint8_t, BYTES> key { 0 };
    };

    class PublicKey {
    public:
        static const uint32_t BYTES { 32 };

        PublicKey() noexcept {}

        PublicKey(const uint8_t* pk, size_t len);

        PublicKey(const PublicKey& o) noexcept :
                PublicKey(o.key.data(), o.key.size()) {}
        PublicKey(PublicKey&& o) noexcept :
                PublicKey(o.key.data(), o.key.size()) {
            o.clear();
        }

        ~PublicKey() noexcept {
            clear();
        }

        explicit operator bool() const noexcept {
            return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
        }

        bool operator==(const PublicKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) == 0;
        }

        bool operator!=(const PublicKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) != 0;
        }

        PublicKey& operator=(const PublicKey& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            return *this;
        }

        PublicKey& operator=(PublicKey&& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            o.clear();
            return *this;
        }

        const uint8_t* cbegin() const noexcept {
            return key.cbegin();
        }

        const uint8_t* cend() const noexcept {
            return key.cend();
        }

        const uint8_t* bytes() const noexcept {
            return key.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        void clear() noexcept {
            key.fill(0);
        }

        bool verify(const uint8_t* sig, size_t sigLen, const uint8_t* data, size_t dataLen) const;

        template<typename ST, typename DT>
        bool verify(const ST& sig, const DT& data) const {
            return verify(sig.data(), sig.size(), data.data(), data.size());
        }

        operator std::string() const noexcept;

        friend class KeyPair;

    private:
        std::array<uint8_t, BYTES> key { 0 };
    };

    class KeyPair {
    public:
        static const uint32_t SEED_BYTES { 32 };

        KeyPair() noexcept;
        KeyPair(PrivateKey& sk) noexcept;
        KeyPair(PrivateKey&& sk) noexcept;
        KeyPair(const uint8_t* sk, size_t len);

        KeyPair(KeyPair& o) noexcept :
                sk(o.sk), pk(o.pk) {}
        KeyPair(KeyPair&& o) noexcept :
                sk(std::move(o.sk)), pk(std::move(o.pk)) {}

        static KeyPair fromSeed(const uint8_t* seed, size_t len);

        template<typename T>
        static KeyPair fromSeed(const T& seed) {
            return fromSeed(seed.data(), seed.size());
        }

        explicit operator bool() const noexcept {
            return static_cast<bool>(sk);
        }

        bool operator==(const KeyPair& o) const noexcept {
            return sk == o.sk;
        }

        bool operator!=(const KeyPair& o) const noexcept {
            return sk != o.sk;
        }

        KeyPair& operator=(const KeyPair& o) noexcept {
            sk = o.sk;
            pk = o.pk;
            return *this;
        }

        KeyPair& operator=(KeyPair&& o) noexcept {
            sk = o.sk;
            pk = o.pk;
            o.clear();
            return *this;
        }

        const PrivateKey& privateKey() const noexcept {
            return sk;
        }

        const PublicKey& publicKey() const noexcept {
            return pk;
        }

        void clear() noexcept {
            sk.clear();
            pk.clear();
        }

    private:
        PrivateKey sk;
        PublicKey pk;
    };

    static const uint32_t BYTES { 64 };

    Signature() noexcept {
        reset();
    }

    Signature(const Signature& o) noexcept {
        std::memcpy(state.__opaque__, o.state.__opaque__, sizeof(state.__opaque__));
    }

    Signature(Signature&& o) noexcept {
        std::memcpy(state.__opaque__, o.state.__opaque__, sizeof(state.__opaque__));
        std::memset(o.state.__opaque__, 0, sizeof(o.state.__opaque__));
    }

    ~Signature() noexcept {}

    bool operator==(const Signature& o) const noexcept {
        return std::memcmp(o.state.__opaque__, state.__opaque__, sizeof(state.__opaque__)) == 0;
    }

    bool operator!=(const Signature& o) const noexcept {
        return std::memcmp(o.state.__opaque__, state.__opaque__, sizeof(state.__opaque__)) != 0;
    }

    Signature& operator=(const Signature& o) noexcept {
        std::memcpy(state.__opaque__, o.state.__opaque__, sizeof(state.__opaque__));
        return *this;
    }

    Signature& operator=(Signature&& o) noexcept {
        std::memcpy(state.__opaque__, o.state.__opaque__, sizeof(state.__opaque__));
        std::memset(o.state.__opaque__, 0, sizeof(o.state.__opaque__));
        return *this;
    }

    void reset();

    void update(const uint8_t* part, size_t len);

    template <typename T>
    void update(const T& part) {
        return update(part.data(), part.size());
    }

    void sign(uint8_t* sig, size_t len, const PrivateKey& sk) const;

    template <typename T>
    bool sign(T& sig, const PrivateKey& sk) const {
        return sign(sig.data(), sig.size(), sk);
    }

    std::vector<uint8_t> sign(const PrivateKey& sk) const {
        std::vector<uint8_t> sig(BYTES);
        sign(sig.data(), sig.size(), sk);
        return sig;
    }

    bool verify(const uint8_t* sig, size_t len, const PublicKey& pk) const;

    template <typename T>
    bool verify(const T& sig, const PublicKey& pk) const {
        return verify(sig.data(), sig.size(), pk);
    }

private:
    struct SignState { uint8_t __opaque__[256]; };
    SignState state;
};

class CryptoBox {
public:
    class KeyPair;

    class PrivateKey {
    public:
        static const uint32_t BYTES { 32 };

        PrivateKey() noexcept {}

        PrivateKey(const uint8_t* sk, size_t len);

        PrivateKey(PrivateKey& o) noexcept :
                PrivateKey(o.key.data(), o.key.size()) {}
        PrivateKey(PrivateKey&& o) noexcept :
                PrivateKey(o.key.data(), o.key.size()) {
            o.clear();
        }

        static PrivateKey fromSignatureKey(const Signature::PrivateKey& signSk);

        ~PrivateKey() noexcept {
            clear();
        }

        explicit operator bool() const noexcept {
            return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
        }

        bool operator==(const PrivateKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) == 0;
        }

        bool operator!=(const PrivateKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) != 0;
        }

        PrivateKey& operator=(const PrivateKey& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            return *this;
        }

        PrivateKey& operator=(PrivateKey&& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            o.clear();
            return *this;
        }

        const uint8_t* cbegin() const noexcept {
            return key.cbegin();
        }

        const uint8_t* cend() const noexcept {
            return key.cend();
        }

        const uint8_t* bytes() const noexcept {
            return key.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        void clear() noexcept {
            key.fill(0);
        }

        operator std::string() const noexcept;

        friend class KeyPair;

    private:
        std::array<uint8_t, BYTES> key { 0 };
    };

    class PublicKey {
    public:
        static const uint32_t BYTES { 32 };

        PublicKey() noexcept {}

        PublicKey(const uint8_t* pk, size_t len);

        PublicKey(PublicKey& o) noexcept :
                PublicKey(o.key.data(), o.key.size()) {}
        PublicKey(PublicKey&& o) noexcept :
                PublicKey(o.key.data(), o.key.size()) {
            o.clear();
        }

        static PublicKey fromSignatureKey(const Signature::PublicKey& signPk);

        ~PublicKey() noexcept {
            clear();
        }

        explicit operator bool() const noexcept {
            return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
        }

        bool operator==(const PublicKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) == 0;
        }

        bool operator!=(const PublicKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) != 0;
        }

        PublicKey& operator=(const PublicKey& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            return *this;
        }

        PublicKey& operator=(PublicKey&& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            o.clear();
            return *this;
        }

        const uint8_t* cbegin() const noexcept {
            return key.cbegin();
        }

        const uint8_t* cend() const noexcept {
            return key.cend();
        }

        const uint8_t* bytes() const noexcept {
            return key.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        void clear() noexcept {
            key.fill(0);
        }

        operator std::string() const noexcept;

        friend class KeyPair;

    private:
        std::array<uint8_t, BYTES> key { 0 };
    };

    class Nonce {
    public:
        static const uint32_t BYTES { 24 };

        Nonce() noexcept {};

        Nonce(const uint8_t* pk, size_t len);

        Nonce(const Nonce& o) noexcept :
                Nonce(o.nonce.data(), o.nonce.size()) {}
        Nonce(Nonce&& o) noexcept :
                Nonce(o.nonce.data(), o.nonce.size()) {
            o.clear();
        }

        ~Nonce() noexcept {
            clear();
        }

        explicit operator bool() const noexcept {
            return !std::all_of(nonce.cbegin(), nonce.cend(), [](uint8_t i){ return !i; });
        }

        bool operator==(const Nonce& o) const noexcept {
            return std::memcmp(nonce.data(), o.nonce.data(), BYTES) == 0;
        }

        bool operator!=(const Nonce& o) const noexcept {
            return std::memcmp(nonce.data(), o.nonce.data(), BYTES) != 0;
        }

        Nonce& operator=(const Nonce& o) noexcept {
            std::memcpy(nonce.data(), o.nonce.data(), BYTES);
            return *this;
        }

        Nonce& operator=(Nonce&& o) noexcept {
            std::memcpy(nonce.data(), o.nonce.data(), BYTES);
            o.clear();
            return *this;
        }

        const uint8_t* cbegin() const noexcept {
            return nonce.cbegin();
        }

        const uint8_t* cend() const noexcept {
            return nonce.cend();
        }

        const uint8_t* bytes() const noexcept {
            return nonce.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        void clear() noexcept {
            nonce.fill(0);
        }

        Nonce& increment() noexcept;
        Nonce& random() noexcept;

        operator std::string() const noexcept;

    private:
        std::array<uint8_t, BYTES> nonce {};
    };

    class KeyPair {
    public:
        static const uint32_t SEED_BYTES { 32 };

        KeyPair() noexcept;
        KeyPair(PrivateKey& sk) noexcept;
        KeyPair(PrivateKey&& sk) noexcept;
        KeyPair(const uint8_t* sk, size_t len);

        KeyPair(KeyPair& o) noexcept :
                sk(o.sk), pk(o.pk) {}
        KeyPair(KeyPair&& o) noexcept :
                sk(std::move(o.sk)), pk(std::move(o.pk)) {}

        static KeyPair fromSeed(const uint8_t* seed, size_t len);

        template<typename T>
        static KeyPair fromSeed(const T& seed) {
            return fromSeed(seed.data(), seed.size());
        }

        static KeyPair fromSignatureKeyPair(const Signature::KeyPair& signKeyPair);

        explicit operator bool() const noexcept {
            return static_cast<bool>(sk);
        }

        bool operator==(const KeyPair& o) const noexcept {
            return sk == o.sk;
        }

        bool operator!=(const KeyPair& o) const noexcept {
            return sk != o.sk;
        }

        KeyPair& operator=(const KeyPair& o) noexcept {
            sk = o.sk;
            pk = o.pk;
            return *this;
        }

        KeyPair& operator=(KeyPair&& o) noexcept {
            sk = o.sk;
            pk = o.pk;
            o.clear();
            return *this;
        }

        const PrivateKey& privateKey() const noexcept {
            return sk;
        }

        const PublicKey& publicKey() const noexcept {
            return pk;
        }

        void clear() noexcept {
            sk.clear();
            pk.clear();
        }

    private:
        PrivateKey sk;
        PublicKey pk;
    };

    static const uint32_t SYMMETRIC_KEY_BYTES { 32 };
    static const uint32_t MAC_BYTES { 16 };

    CryptoBox() noexcept {};

    CryptoBox(const PublicKey& pk, const PrivateKey& sk);

    CryptoBox(const CryptoBox& o) noexcept :
            key(o.key) {}
    CryptoBox(CryptoBox&& o) noexcept :
            key(o.key) {
        o.clear();
    }

    ~CryptoBox() noexcept {
        clear();
    }

    explicit operator bool() const noexcept {
        return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
    }

    bool operator==(const CryptoBox& o) const noexcept {
        return std::memcmp(key.data(), o.key.data(), SYMMETRIC_KEY_BYTES) == 0;
    }

    bool operator!=(const CryptoBox& o) const noexcept {
        return std::memcmp(key.data(), o.key.data(), SYMMETRIC_KEY_BYTES) != 0;
    }

    CryptoBox& operator=(const CryptoBox& o) noexcept {
        std::memcpy(key.data(), o.key.data(), SYMMETRIC_KEY_BYTES);
        return *this;
    }

    CryptoBox& operator=(CryptoBox&& o) noexcept {
        std::memcpy( key.data(), o.key.data(), SYMMETRIC_KEY_BYTES);
        o.clear();
        return *this;
    }

    const uint8_t* cbegin() const noexcept {
        return key.cbegin();
    }

    const uint8_t* cend() const noexcept {
        return key.cend();
    }

    const uint8_t* bytes() const noexcept {
        return key.data();
    }

    size_t size() const noexcept {
        return SYMMETRIC_KEY_BYTES;
    }

    void clear() noexcept {
        key.fill(0);
    }

    void encrypt(uint8_t* cipher, size_t cipherLen,
        const uint8_t* plain, size_t plainLen, const Nonce& nonce) const;

    template<typename CT, typename PT>
    void encrypt(CT& cipher, const PT& plain, const Nonce& nonce) const {
        encrypt(cipher.data(), cipher.size(), plain.data(), plain.size(), nonce);
    }

    std::vector<uint8_t> encrypt(const uint8_t* plain, size_t length, const Nonce& nonce) const {
        std::vector<uint8_t> cipher(length + MAC_BYTES);
        encrypt(cipher.data(), cipher.size(), plain, length, nonce);
        return cipher;
    }

    template<typename T>
    std::vector<uint8_t> encrypt(const T& plain, const Nonce& nonce) const {
        return encrypt(plain.data(), plain.size(), nonce);
    }

    static void encrypt(uint8_t* cipher, size_t cipherLen, const uint8_t* plain, size_t plainLen,
            const Nonce& nonce, const PublicKey& pk, const PrivateKey& sk);

    template<typename CT, typename PT>
    static void encrypt(CT& cipher, const PT& plain,
            const Nonce& nonce, const PublicKey& pk, const PrivateKey& sk) {
        encrypt(cipher.data(), cipher.size(), plain.data(), plain.size(), nonce, pk, sk);
    }

    static std::vector<uint8_t> encrypt(const uint8_t* plain, size_t length,
            const Nonce& nonce, const PublicKey& pk, const PrivateKey& sk) {
        std::vector<uint8_t> cipher(length + MAC_BYTES);
        encrypt(cipher.data(), cipher.size(), plain, length, nonce, pk, sk);
        return cipher;
    }

    template<typename T>
    static std::vector<uint8_t> encrypt(const T& plain, const Nonce &nonce,
            const PublicKey& pk, const PrivateKey& sk) {
        return encrypt(plain.data(), plain.size(), nonce, pk, sk);
    }

    void decrypt(uint8_t* plain, size_t plainLen,
            const uint8_t *cipher, size_t cipherLen, const Nonce &nonce) const;

    template<typename PT, typename CT>
    void decrypt(PT& plain, const CT& cipher, const Nonce& nonce) const {
        decrypt(plain.data(), plain.size(), cipher.data(), cipher.size(), nonce);
    }

    std::vector<uint8_t> decrypt(const uint8_t *cipher, size_t length, const Nonce &nonce) const {
        std::vector<uint8_t> plain(length - MAC_BYTES);
        decrypt(plain.data(), plain.size(), cipher, length, nonce);
        return plain;
    }

    template<typename T>
    std::vector<uint8_t> decrypt(const T& cipher, const Nonce &nonce) const {
        return decrypt(cipher.data(), cipher.size(), nonce);
    }

    static void decrypt(uint8_t* plain, size_t plainLen, const uint8_t* cipher, size_t cipherLen,
            const Nonce &nonce, const PublicKey& pk, const PrivateKey& sk);

    template<typename PT, typename CT>
    static void decrypt(PT& plain, const CT& cipher,
            const Nonce& nonce, const PublicKey& pk, const PrivateKey& sk) {
        decrypt(plain.data(), plain.size(), cipher.data(), cipher.size(), nonce, pk, sk);
    }

    static std::vector<uint8_t> decrypt(const uint8_t* cipher, size_t length,
            const Nonce &nonce, const PublicKey& pk, const PrivateKey& sk) {
        std::vector<uint8_t> plain(length - MAC_BYTES);
        decrypt(plain.data(), plain.size(), cipher, length, nonce, pk, sk);
        return plain;
    }

    template<typename T>
    static std::vector<uint8_t> decrypt(const T& cipher, const Nonce &nonce,
            const PublicKey& pk, const PrivateKey& sk) {
        return decrypt(cipher.data(), cipher.size(), nonce, pk, sk);
    }

protected:

private:
    std::array<uint8_t, SYMMETRIC_KEY_BYTES> key {};
};

class SHA256 {
public:
    static const uint32_t BYTES { 32 };

    SHA256() noexcept {
        reset();
    }

    void reset();

    void update(const uint8_t* part, size_t len);

    template<typename T>
    void update(const T& part) {
        update(part.data(), part.size());
    }

    void digest(uint8_t *hash, size_t len);

    template<typename T>
    void digest(T& hash) {
        digest(hash.data(), hash.size());
    }

    std::vector<uint8_t> digest() {
        std::vector<uint8_t> hash(BYTES);
        digest(hash.data(), hash.size());
        return hash;
    }

    static void digest(uint8_t *hash, size_t hashLen, const uint8_t* data, size_t dataLen);

    template<typename HT, typename DT>
    static void _digest(HT& hash, const DT& data) {
        digest(hash.data(), hash.size(), data.data(), data.size());
    }

    template<typename T>
    static std::vector<uint8_t> _digest(const T& data) {
        std::vector<uint8_t> hash(BYTES);
        digest(hash.data(), hash.size(), data.data(), data.size());
        return hash;
    }

private:
    struct DigestState { uint8_t __opaque__[128]; };
    DigestState state;
};

class Random {
public:
    // [0, upbound)
    static uint8_t uint8();
    static uint8_t uint8(uint8_t upbound);

    static uint16_t uint16();
    static uint16_t uint16(uint16_t upbound);

    static uint32_t uint32();
    static uint32_t uint32(uint32_t upbound);

    static uint64_t uint64();
    static uint64_t uint64(uint64_t upbound);

    static void buffer(void* buf, size_t length);
};

class CryptoError : public std::runtime_error
{
public:
    explicit CryptoError(const std::string& what) : runtime_error(what) {}
    explicit CryptoError(const char* what) : runtime_error(what) {}

    CryptoError(const CryptoError&) noexcept = default;
    virtual ~CryptoError() noexcept = default;
};

}
}
