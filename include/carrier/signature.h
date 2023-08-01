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

        void sign(const Blob& data, Blob& sig) const;

        std::vector<uint8_t> sign(const Blob& data) const {
            std::vector<uint8_t> sig(Signature::BYTES);
            Blob _sig{ sig };
            sign(data, _sig);
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

        bool verify(const Blob& data, const Blob& signature) const;

        operator std::string() const noexcept;

        friend class KeyPair;

    private:
        std::array<uint8_t, BYTES> key { 0 };
    };

    class KeyPair {
    public:
        static const uint32_t SEED_BYTES { 32 };

        KeyPair() noexcept;
        KeyPair(const PrivateKey& sk) noexcept;
        KeyPair(PrivateKey&& sk) noexcept;
        KeyPair(const Blob& sk);

        KeyPair(KeyPair& o) noexcept :
                sk(o.sk), pk(o.pk) {}
        KeyPair(KeyPair&& o) noexcept :
                sk(std::move(o.sk)), pk(std::move(o.pk)) {}

        static KeyPair fromPrivateKey(const PrivateKey& key) {
            return KeyPair(key);
        }

        static KeyPair fromPrivateKey(const Blob& key) {
            return KeyPair(key);
        }

        static KeyPair fromSeed(const Blob& seed);
        static KeyPair random();

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

    static std::vector<uint8_t> sign(const Blob& data, const PrivateKey& sk) {
         return sk.sign(data);
     }

     static bool verify(const Blob& data, const Blob& signature, const PublicKey& pk) {
         return pk.verify(data, signature);
     }

private:
    struct SignState { uint8_t __opaque__[256]; };
    SignState state;
};
} // namespace carrier
} // namespace elastos
