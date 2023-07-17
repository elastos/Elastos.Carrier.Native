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
    // Value(const Value& val) noexcept;
    // Value(Value&& val) noexcept;
    // Value& operator=(const Value&) = delete;

    static Value of(const std::vector<uint8_t>& data) {
		return Value({}, {}, {}, {}, -1, {}, data);
	}

	static Value of(const Id& publicKey, const Blob& nonce, int sequenceNumber, const std::vector<uint8_t>& signature,
            const std::vector<uint8_t>& data) {
		return Value(publicKey, {}, {}, nonce, sequenceNumber, signature, data);
	}

	static Value of(const Id& publicKey, const Id& recipient, const Blob& nonce, int sequenceNumber,
			const std::vector<uint8_t>& signature, const std::vector<uint8_t>& data) {
		return Value(publicKey, {}, recipient, nonce, sequenceNumber, signature, data);
	}

	static Value of(const Id& publicKey, const Blob& privateKey, const Id& recipient, const Blob& nonce,
			int sequenceNumber, const std::vector<uint8_t>& signature, const std::vector<uint8_t>& data) {
		return Value(publicKey, privateKey, recipient, nonce, sequenceNumber, signature, data);
	}

	static Value createValue(const std::vector<uint8_t>& data) {
		return Value({}, {}, {}, {}, -1, {}, data);
	}

	static Value createSignedValue(const std::vector<uint8_t>& data) {
		auto keypair = Signature::KeyPair::random();
		auto nonce = CryptoBox::Nonce::random();

		return  createSignedValue(keypair, nonce.blob(), 0, data);
	}

	static Value createSignedValue(Signature::KeyPair keypair, const Blob& nonce,
			const std::vector<uint8_t>& data) {
		return createSignedValue(keypair, nonce, 0, data);
	}

	static Value createSignedValue(Signature::KeyPair keypair, const Blob& nonce,
			int sequenceNumber, const std::vector<uint8_t>& data) {
		return Value(keypair, {}, nonce, sequenceNumber, data);
	}

	static Value createEncryptedValue(const Id& recipient, const std::vector<uint8_t>& data) {
		auto keypair = Signature::KeyPair::random();
		auto nonce = CryptoBox::Nonce::random();

		return createEncryptedValue(keypair, recipient, nonce.blob(), 0, data);
	}

	static Value createEncryptedValue(Signature::KeyPair keypair, const Id& recipient, const Blob& nonce, const std::vector<uint8_t>& data) {
		return createEncryptedValue(keypair, recipient, nonce, 0, data);
	}

	static Value createEncryptedValue(Signature::KeyPair keypair, const Id& recipient, const Blob& nonce, int sequenceNumber, const std::vector<uint8_t>& data) {
		if (recipient == Id::zero())
			throw std::invalid_argument("Invalid recipient");

		return Value(keypair, recipient, nonce, sequenceNumber, data);
	}

    static Id calculateId(const Id& publicKey, const Blob& nonce, const std::vector<uint8_t>& data);

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

    const Signature::PrivateKey& getPrivateKey() const {
        return privateKey;
    }

    int getSequenceNumber() const noexcept {
        return sequenceNumber;
    }

    const Blob& getNonce() const  noexcept{
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
    // friend class SqliteStorage;
    // friend class FindValueResponse;
    // friend class StoreValueRequest;
    // friend class Node;

    Value(const Id& publicKey, const Blob& privateKey, const Id& recipient, const Blob& nonce,
        int sequenceNumber, const std::vector<uint8_t>& signature, const std::vector<uint8_t>& data);
    Value(const Signature::KeyPair& keypair, const Id& recipient, const Blob& nonce,
        int sequenceNumber, const std::vector<uint8_t>& data);

    std::vector<uint8_t> getSignData() const;

    Id publicKey {};
    Id recipient {};
    Signature::PrivateKey privateKey {};
    Blob nonce {};
    std::vector<uint8_t> signature {};
    std::vector<uint8_t> data {};
    int32_t sequenceNumber {0};
};
}
}
