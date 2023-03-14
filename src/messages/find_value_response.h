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

#include "carrier/value.h"
#include "carrier/types.h"

#include "lookup_response.h"
#include "message.h"

namespace elastos {
namespace carrier {

class FindValueResponse : public LookupResponse {
public:
    FindValueResponse(int txid, const Sp<Value> _value)
        : LookupResponse(Message::Method::FIND_VALUE, txid), value(_value) {}

    FindValueResponse()
        : FindValueResponse(0, std::make_shared<Value>()) {}

    const Id& getPublicKey() const {
        return value->getPublicKey();
    }

    void setPublicKey(const Id& publicKey) {
        value->setPublicKey(publicKey.asBlob());
    }

    const Id& getRecipient() const {
        return value->getRecipient();
    }

    void setRecipient(const Id& recipient) {
        value->setRecipient(recipient.asBlob());
    }

    const CryptoBox::Nonce& getNonce() const {
        return value->getNonce();
    }

    void setNonce(CryptoBox::Nonce& nonce) {
        value->setNonce(Blob(nonce.bytes(), nonce.size()));
    }

    const std::vector<std::uint8_t>& getSignature() const {
        return value->getSignature();
    }

    void setSignature(const std::vector<std::uint8_t>& signature) {
        value->setSignature(Blob(signature.data(), signature.size()));
    }

    int getSequenceNumber() const {
        return value->getSequenceNumber();
    }

    void setSequenceNumber(int sequenceNumber) {
        value->setSequenceNumber(sequenceNumber);
    }

    const std::vector<std::uint8_t>& getData() const {
        return value->getData();
    }

    void setData(const std::vector<std::uint8_t>& data) {
        value->setData(Blob(data.data(), data.size()));
    }

    Sp<Value> getValue() const {
        return value;
    }

    int estimateSize() const override {
        return LookupResponse::estimateSize() + 195 + value->getData().size();
    }

protected:
    void _serialize(nlohmann::json& object) const override;
    void _parse(const std::string& field, nlohmann::json& object) override;
    void _toString(std::stringstream& ss) const override;

private:
    Sp<Value> value;
};

}
}
