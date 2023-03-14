/*
 * Copyright (c) 2022 - 2023 Elastos Foundation
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

#include "message.h"
#include "carrier/value.h"

namespace elastos {
namespace carrier {

class StoreValueRequest : public Message {
public:
    StoreValueRequest(const Sp<Value> _value)
        : Message(Message::Type::REQUEST, Message::Method::STORE_VALUE), value(_value) {}

    StoreValueRequest(): StoreValueRequest(std::make_shared<Value>()) {}

    int getToken() const noexcept {
        return token;
    }

    void setToken(int token) noexcept {
        this->token = token;
    }

    int getExpectedSequenceNumber() const noexcept {
        return expectedSequenceNumber;
    }

    const Sp<Value> getValue() const noexcept {
        return value;
    }

    int estimateSize() const override {
        return Message::estimateSize() + 208 + value->getData().size();
    }

protected:
    void serializeInternal(nlohmann::json& root) const override;
    void parse(const std::string& fieldName, nlohmann::json& object) override;
    void toString(std::stringstream& str) const override;

private:
    int token {0};
    int expectedSequenceNumber {-1};
    Sp<Value> value;
};

} // namespace carrier
} // namespace elastos
