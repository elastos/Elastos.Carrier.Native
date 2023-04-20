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
#include <algorithm>

#include "def.h"
#include "blob.h"

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC Signature {
public:
    class KeyPair;

    class PrivateKey {
    public:
        static const uint32_t BYTES { 64 };

        PrivateKey() noexcept {}

        PrivateKey(const Blob& sk);

        PrivateKey(const PrivateKey& o) noexcept :
                PrivateKey(o.blob()) {}

        PrivateKey(PrivateKey&& o) noexcept :
                PrivateKey(o.blob()) {
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
            return key.data();
        }

        const uint8_t* cend() const noexcept {
            return key.data() + key.size();
        }

        const uint8_t* bytes() const noexcept {
            return key.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        const Blob blob() const noexcept {
            return key;
        }

        void clear() noexcept {
            key.fill(0);
        }

        void sign(Blob& sig, const Blob& data) const;

        std::vector<uint8_t> sign(const Blob& data) const {
            std::vector<uint8_t> sig(Signature::BYTES);
            Blob _sig{ sig };
            sign(_sig, data);
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

        PublicKey(const Blob& pk);

        PublicKey(const PublicKey& o) noexcept :
                PublicKey(o.blob()) {}

        PublicKey(PublicKey&& o) noexcept :
                PublicKey(o.blob()) {
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
            return key.data();
        }

        const uint8_t* cend() const noexcept {
            return key.data() + key.size();
        }

        const uint8_t* bytes() const noexcept {
            return key.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        const Blob blob() const noexcept {
            return key;
        }

        void clear() noexcept {
            key.fill(0);
        }

        bool verify(const Blob& sig, const Blob& data) const;

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
        KeyPair(const Blob& sk);

        KeyPair(KeyPair& o) noexcept :
                sk(o.sk), pk(o.pk) {}
        KeyPair(KeyPair&& o) noexcept :
                sk(std::move(o.sk)), pk(std::move(o.pk)) {}

        static KeyPair fromSeed(const Blob& seed);

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

    void update(const Blob& part);

    void sign(Blob& sig, const PrivateKey& sk) const;

    std::vector<uint8_t> sign(const PrivateKey& sk) const {
        std::vector<uint8_t> sig(BYTES);
        Blob _sig{sig};
        sign(_sig, sk);
        return sig;
    }

    bool verify(const Blob& sig, const PublicKey& pk) const;

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

        PrivateKey(const Blob& sk);

        PrivateKey(PrivateKey& o) noexcept :
                PrivateKey(o.blob()) {}
        PrivateKey(PrivateKey&& o) noexcept :
                PrivateKey(o.blob()) {
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
            return key.data();
        }

        const uint8_t* cend() const noexcept {
            return key.data() + key.size();
        }

        const uint8_t* bytes() const noexcept {
            return key.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        const Blob blob() const noexcept {
            return key;
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

        PublicKey(const Blob& pk);

        PublicKey(PublicKey& o) noexcept :
                PublicKey(o.blob()) {}
        PublicKey(PublicKey&& o) noexcept :
                PublicKey(o.blob()) {
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
            return key.data();
        }

        const uint8_t* cend() const noexcept {
            return key.data() + key.size();
        }

        const uint8_t* bytes() const noexcept {
            return key.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        const Blob blob() const noexcept {
            return key;
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

        Nonce(const Blob& pk);

        Nonce(const Nonce& o) noexcept :
                Nonce(o.blob()) {}
        Nonce(Nonce&& o) noexcept :
                Nonce(o.blob()) {
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
            return nonce.data();
        }

        const uint8_t* cend() const noexcept {
            return nonce.data() + nonce.size();
        }

        const uint8_t* bytes() const noexcept {
            return nonce.data();
        }

        size_t size() const noexcept {
            return BYTES;
        }

        const Blob blob() const noexcept {
            return nonce;
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
        KeyPair(const Blob& sk);

        KeyPair(KeyPair& o) noexcept :
                sk(o.sk), pk(o.pk) {}
        KeyPair(KeyPair&& o) noexcept :
                sk(std::move(o.sk)), pk(std::move(o.pk)) {}

        static KeyPair fromSeed(const Blob& seed);

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
        return key.data();
    }

    const uint8_t* cend() const noexcept {
        return key.data() + key.size();
    }

    const uint8_t* bytes() const noexcept {
        return key.data();
    }

    size_t size() const noexcept {
        return SYMMETRIC_KEY_BYTES;
    }

    const Blob blob() const noexcept {
        return key;
    }

    void clear() noexcept {
        key.fill(0);
    }

    void encrypt(Blob& cipher, const Blob& plain, const Nonce& nonce) const;

    std::vector<uint8_t> encrypt(const Blob& plain, const Nonce& nonce) const {
        std::vector<uint8_t> cipher(plain.size() + MAC_BYTES);
        Blob _cipher{cipher};
        encrypt(_cipher, plain, nonce);
        return cipher;
    }

    static void encrypt(Blob& cipher, const Blob& plain, const Nonce& nonce,
            const PublicKey& pk, const PrivateKey& sk);

    static std::vector<uint8_t> encrypt(const Blob& plain, const Nonce& nonce,
            const PublicKey& pk, const PrivateKey& sk) {
        std::vector<uint8_t> cipher(plain.size() + MAC_BYTES);
        Blob _cipher{cipher};
        encrypt(_cipher, plain, nonce, pk, sk);
        return cipher;
    }

    void decrypt(Blob& plain, const Blob& cipher, const Nonce& nonce) const;

    std::vector<uint8_t> decrypt(const Blob& cipher, const Nonce &nonce) const {
        std::vector<uint8_t> plain(cipher.size() - MAC_BYTES);
        Blob _plain{plain};
        decrypt(_plain, cipher, nonce);
        return plain;
    }

    static void decrypt(Blob& plain, const Blob& cipher, const Nonce& nonce,
            const PublicKey& pk, const PrivateKey& sk);

    static std::vector<uint8_t> decrypt(const Blob& cipher, const Nonce &nonce,
            const PublicKey& pk, const PrivateKey& sk) {
        std::vector<uint8_t> plain(cipher.size() - MAC_BYTES);
        Blob _plain{plain};
        decrypt(_plain, cipher, nonce, pk, sk);
        return plain;
    }

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

    void update(const Blob& part);

    void digest(Blob& hash);

    std::vector<uint8_t> digest() {
        std::vector<uint8_t> hash(BYTES);
        Blob _hash{hash};
        digest(_hash);
        return hash;
    }

    static void digest(Blob& hash, const Blob& data);

    static std::vector<uint8_t> digest(const Blob& data) {
        std::vector<uint8_t> hash(BYTES);
        Blob _hash{hash};
        digest(_hash, data);
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
