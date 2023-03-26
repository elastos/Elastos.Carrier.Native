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
#include "crypto_core.h"

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC Value {
public:
    Value() = default;
    Value(const Value&) = delete;
    Value(Value&&) = delete;
    Value& operator=(const Value&) = delete;

    static Id calculateId(const Value& value);

    Id getId() const {
        return Value::calculateId(*this);
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

    int getSequenceNumber() const noexcept {
        return sequenceNumber;
    }

    const CryptoBox::Nonce& getNonce() const  noexcept{
        return nonce;
    }

    const std::vector<uint8_t>& getSignature() const noexcept {
        return signature;
    }

    const std::vector<uint8_t>& getData() const noexcept {
        return data;
    }

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
    friend class SqliteStorage;
    friend class FindValueResponse;
    friend class StoreValueRequest;
    friend class Node;

    // internal setts used in SqliteStorage type.
    void setPrivateKey(const Blob& val) noexcept {
        this->privateKey = Signature::PrivateKey(val);
    }
    void setPublicKey(const Blob& val) {
        this->publicKey = Id(val);
    }
    void setRecipient(const Blob& val) {
        this->recipient = Id(val);
    }
    void setSignature(const Blob& val) {
        signature.resize(val.size());
        std::memcpy(signature.data(), val.ptr(), val.size());
    }
    void setNonce(const Blob& val) {
        this->nonce = CryptoBox::Nonce(val);
    }
    void setData(const Blob& val) {
        data.resize(val.size());
        std::memcpy(data.data(), val.ptr(), val.size());
    }

    void setSequenceNumber(int seqNumber) {
        this->sequenceNumber = seqNumber;
    }

    const Signature::PrivateKey& getPrivateKey() const {
        return privateKey;
    }

    void purgePrivateKey() {
        privateKey.clear();
    }

    static Sp<Value> create(const std::vector<uint8_t>& data);
    static Sp<Value> createSigned(const std::vector<uint8_t>& data);
    static Sp<Value> createEncrypted(const Id& to, const std::vector<uint8_t>& data);
    static Sp<Value> updateValue(const Sp<Value> oldValue, const std::vector<uint8_t>& newData);

    void createSignature();
    bool verifySignature() const;

    Id publicKey {};
    Id recipient {};
    Signature::PrivateKey privateKey {};
    CryptoBox::Nonce nonce {};
    std::vector<std::uint8_t> signature {};
    std::vector<std::uint8_t> data {};
    int32_t sequenceNumber {0};
};
}
}
