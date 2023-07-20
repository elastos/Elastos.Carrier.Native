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

#include "carrier/value.h"
#include "carrier/crypto_core.h"

#include "message.h"

namespace elastos {
namespace carrier {

class StoreValueRequest : public Message {
public:
    StoreValueRequest()
        : Message(Message::Type::REQUEST, Message::Method::STORE_VALUE) {}

    StoreValueRequest(const Value& value, int _token)
            : Message(Message::Type::REQUEST, Message::Method::STORE_VALUE), token(_token) {
        setValue(value);
    }

    int getToken() const noexcept {
        return token;
    }

    void setToken(int token) noexcept {
        this->token = token;
    }

    int getExpectedSequenceNumber() const noexcept {
        return expectedSequenceNumber;
    }

    void setExpectedSequenceNumber(int expectedSequenceNumber) {
		this->expectedSequenceNumber = expectedSequenceNumber;
	}

    void setValue(const Value& value);
    Value getValue() const;

    bool isMutable() const {
		return static_cast<bool>(publicKey);
	}

    bool isEncrypted() const {
		return static_cast<bool>(recipient);
	}

	Id getValueId() {
		return Value::calculateId(publicKey, nonce, value);
	}

    int estimateSize() const override {
        return Message::estimateSize() + 208 + value.size();
    }

protected:
    void serializeInternal(nlohmann::json& root) const override;
    void parse(const std::string& fieldName, nlohmann::json& object) override;
    void toString(std::stringstream& str) const override;

private:
    int token {0};
    Id publicKey {};
    Id recipient {};
    CryptoBox::Nonce nonce {};
    int sequenceNumber {-1};
	int expectedSequenceNumber {-1};
    std::vector<uint8_t> signature {};
	std::vector<uint8_t> value;
};

} // namespace carrier
} // namespace elastos
