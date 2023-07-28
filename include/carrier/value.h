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
#include <optional>

#include "def.h"
#include "id.h"
#include "blob.h"
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
        return Value({}, {}, {}, {}, -1, {}, Blob(data));
    }

    static Value createSignedValue(const std::vector<uint8_t>& data) {
        auto keyPair = Signature::KeyPair::random();
        auto nonce = CryptoBox::Nonce::random();
        return Value(keyPair, std::nullopt, nonce, 0, data);
    }

    static Value createSignedValue(const Signature::KeyPair& keyPair, const CryptoBox::Nonce& nonce,
        const std::vector<uint8_t>& data) {
        return Value(keyPair, std::nullopt, nonce, 0, data);
    }

    static Value createSignedValue(const Signature::KeyPair& keyPair, const CryptoBox::Nonce& nonce,
        int sequenceNumber, const std::vector<uint8_t>& data) {
        return Value(keyPair, std::nullopt, nonce, sequenceNumber, data);
    }

    static Value createEncryptedValue(const Id& recipient, const std::vector<uint8_t>& data) {
        auto keyPair = Signature::KeyPair::random();
        auto nonce = CryptoBox::Nonce::random();
        return Value(keyPair, std::optional<Id>(recipient), nonce, 0, data);
    }

    static Value createEncryptedValue(const Signature::KeyPair& keyPair, const Id& recipient,
        const CryptoBox::Nonce& nonce, const std::vector<uint8_t>& data) {
        return Value(keyPair, std::optional<Id>(recipient), nonce, 0, data);
    }

    static Value createEncryptedValue(const Signature::KeyPair& keyPair, const Id& recipient,
        const CryptoBox::Nonce& nonce, int sequenceNumber, const std::vector<uint8_t>& data) {
        return Value(keyPair, std::optional<Id>(recipient), nonce, sequenceNumber, data);
    }

    static Id calculateId(const std::optional<Id>& publicKey, const std::optional<CryptoBox::Nonce>& nonce,
        const std::vector<uint8_t>& data);

    Id getId() const {
        return Value::calculateId(publicKey, nonce, data);
    }

    const Id& getPublicKey() const {
        if (!publicKey.has_value())
            throw std::runtime_error("No public key");

        return publicKey.value();
    }

    const Id& getRecipient() const {
        if (!recipient.has_value())
            throw std::runtime_error("No recipient");

        return recipient.value();
    }

    bool hasPrivateKey() const noexcept {
        return privateKey.has_value();
    }

    const Signature::PrivateKey& getPrivateKey() const {
        if (!privateKey.has_value())
            throw std::runtime_error("No private key");

        return privateKey.value();
    }

    int getSequenceNumber() const noexcept {
        return sequenceNumber;
    }

    const CryptoBox::Nonce& getNonce() const {
        if (!nonce.has_value())
            throw std::runtime_error("No nonce");

        return nonce.value();
    }

    const std::vector<uint8_t>& getSignature() const {
        if (!signature.has_value())
            throw std::runtime_error("No signature");
        return signature.value();
    }

    const std::vector<uint8_t>& getData() const noexcept {
        return data;
    }

    Value update(const std::vector<uint8_t>& data);

    size_t size() const noexcept {
        auto size = data.size();
        if (signature.has_value())
            size += signature.value().size();
        return size;
    }

    bool isEncrypted() const noexcept {
        return recipient.has_value();
    }

    bool isSigned() const noexcept {
        return signature.has_value();
    }

    bool isMutable() const noexcept {
        return publicKey.has_value();
    }

    bool isValid() const;

    bool operator== (const Value& other) const;
    operator std::string() const;

private:
    Value(const Blob& publicKey, const Blob& privateKey, const Blob& recipient,
        const Blob& nonce, int sequenceNumber, const Blob& signature, const Blob& data);

    Value(const Signature::KeyPair& keyPair, const std::optional<Id>& recipient, const CryptoBox::Nonce& nonce,
        int sequenceNumber, const std::vector<uint8_t>& data);

    std::vector<uint8_t> getSignData() const;

    std::optional<Id> publicKey {};
    std::optional<Id> recipient {};

    std::optional<Signature::PrivateKey> privateKey {};
    std::optional<CryptoBox::Nonce> nonce {};
    std::optional<std::vector<uint8_t>> signature {};
    std::vector<uint8_t> data {};
    int32_t sequenceNumber {0};
};
}
}
