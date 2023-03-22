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

#include "carrier/crypto_core.h"

namespace elastos {
namespace carrier {

class CryptoContext
{
public:
    CryptoContext() {};
    CryptoContext(const CryptoContext& cc) : box(cc.box), nonce(cc.nonce) {}
    CryptoContext(CryptoContext&& cc) : box(cc.box), nonce(cc.nonce) {
        cc.box.clear();
        cc.nonce.clear();
    }

    CryptoContext(const CryptoBox::PublicKey& pk, const CryptoBox::KeyPair& keypair);
    ~CryptoContext() {
        box.clear();
        nonce.clear();
    };

    CryptoContext& operator=(const CryptoContext& cc) noexcept {
        box = cc.box;
        nonce = cc.nonce;
        return *this;
    }

    CryptoContext& operator=(CryptoContext&& cc) noexcept {
        box = cc.box;
        nonce = cc.nonce;
        cc.box.clear();
        cc.nonce.clear();
        return *this;
    }

    std::vector<uint8_t> encrypt(const uint8_t* plain, size_t length) {
        return box.encrypt(plain, length, nonce);
    }

    std::vector<uint8_t> decrypt(const uint8_t* cipher, size_t length) {
        return box.decrypt(cipher, length, nonce);
    }

    void encrypt(uint8_t* cipher, size_t cipherLen, const uint8_t* plain, size_t plainLen) {
        box.encrypt(cipher, cipherLen, plain, plainLen, nonce);
    }

    void decrypt(uint8_t* plain, size_t plainLen, const uint8_t* cipher, size_t cipherLen) {
        box.decrypt(plain, plainLen, cipher, cipherLen, nonce);
    }

private:
    CryptoBox box;
    CryptoBox::Nonce nonce;
};

} // namespace carrier
} // namespace elastos
