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

#include <vector>
#include <memory>

#include "def.h"
#include "types.h"
#include "id.h"
#include "socket_address.h"
#include "crypto_box.h"
#include "signature.h"

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC Value {
public:
    Value() = default;

    static Value of(Blob publicKey, Blob privateKey, Blob recipient, Blob nonce,
            int sequenceNumber, Blob signature, Blob value) {
        return Value(publicKey, privateKey, recipient, nonce, sequenceNumber, signature, value);
    }

    static Value createValue(const std::vector<uint8_t>& data) {
        auto empty = Blob();
        auto _data = Blob(data);
        return Value(empty, empty, empty, empty, -1, empty, _data);
    }

    static Value createSignedValue(const std::vector<uint8_t>& data) {
        auto keyPair = Signature::KeyPair::random();
        auto nonce = CryptoBox::Nonce::random();
        return  createSignedValue(keyPair, nonce, data);
    }

    static Value createSignedValue(const Signature::KeyPair& keyPair, const CryptoBox::Nonce& nonce,
        const std::vector<uint8_t>& data) {
        return Value(keyPair, {}, nonce, 0, data);
    }

    static Value createSignedValue(const Signature::KeyPair& keyPair, const CryptoBox::Nonce& nonce,
        int sequenceNumber, const std::vector<uint8_t>& data) {
        return Value(keyPair, {}, nonce, sequenceNumber, data);
    }

    static Value createEncryptedValue(const Id& recipient, const std::vector<uint8_t>& data) {
        auto keyPair = Signature::KeyPair::random();
        auto nonce = CryptoBox::Nonce::random();
        return createEncryptedValue(keyPair, recipient, nonce, data);
    }

    static Value createEncryptedValue(const Signature::KeyPair& keyPair, const Id& recipient,
        const CryptoBox::Nonce& nonce, const std::vector<uint8_t>& data) {
        return Value(keyPair, recipient, nonce, 0, data);
    }

    static Value createEncryptedValue(const Signature::KeyPair& keyPair, const Id& recipient,
        const CryptoBox::Nonce& nonce, int sequenceNumber, const std::vector<uint8_t>& data) {
        if (recipient == Id::MIN_ID)
            throw std::invalid_argument("Invalid recipient");
        return Value(keyPair, recipient, nonce, sequenceNumber, data);
    }

    static Id calculateId(const Id& publicKey, const CryptoBox::Nonce& nonce, const std::vector<uint8_t>& data);

    Id getId() const {
        return Value::calculateId(publicKey, nonce, data);
    }

    const Id& getPublicKey() const noexcept {
        return publicKey;
    }

    const Id& getRecipient() const noexcept {
        return recipient;
    }

    bool hasPrivateKey() const noexcept {
        return static_cast<bool>(privateKey);
    }

    const Signature::PrivateKey& getPrivateKey() const noexcept {
        return privateKey;
    }

    int getSequenceNumber() const noexcept {
        return sequenceNumber;
    }

    const CryptoBox::Nonce& getNonce() const noexcept {
        return nonce;
    }

    const std::vector<uint8_t>& getSignature() const noexcept {
        return signature;
    }

    const std::vector<uint8_t>& getData() const noexcept {
        return data;
    }

    Value update(const std::vector<uint8_t>& data);

    size_t size() const noexcept {
        return data.size() + signature.size();
    }

    bool isEncrypted() const noexcept {
        return static_cast<bool>(recipient);
    }

    bool isSigned() const noexcept {
        return !signature.empty();
    }

    bool isMutable() const noexcept {
        return static_cast<bool>(publicKey);
    }

    bool isValid() const;

    bool operator== (const Value& other) const;
    operator std::string() const;

private: // internal methods used in friend class.
    Value(const Blob& publicKey, const Blob& privateKey, const Blob& recipient,
        const Blob& nonce, int sequenceNumber, const Blob& signature, const Blob& data);

    Value(const Signature::KeyPair& keyPair, const Id& recipient, const CryptoBox::Nonce& nonce,
        int sequenceNumber, const std::vector<uint8_t>& data);

    std::vector<uint8_t> getSignData() const;

    Id publicKey {};
    Id recipient {};

    Signature::PrivateKey privateKey {};
    CryptoBox::Nonce nonce {};
    std::vector<uint8_t> signature {};
    std::vector<uint8_t> data {};
    int32_t sequenceNumber {0};
};
}
}
