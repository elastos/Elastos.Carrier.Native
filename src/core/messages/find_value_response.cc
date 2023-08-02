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

#include <nlohmann/json.hpp>

#include "crypto/hex.h"
#include "message.h"
#include "serializers.h"
#include "find_value_response.h"

namespace elastos {
namespace carrier {

void FindValueResponse::setValue(const Value& value) {
    if (value.isMutable()) {
        this->publicKey = value.getPublicKey();
        this->nonce = value.getNonce();
        this->signature = value.getSignature();
        this->sequenceNumber = value.getSequenceNumber();
        if (value.isEncrypted())
            this->recipient = value.getRecipient();
    }
    this->value = value.getData();
}

Value FindValueResponse::getValue() const {
    return Value::of(publicKey.has_value() ? publicKey.value().blob(): Blob(), {},
        recipient.has_value()? recipient.value().blob() : Blob(),
        nonce.has_value() ? nonce.value().blob() : Blob(), sequenceNumber,
        signature.has_value() ? signature.value(): Blob(), Blob(value));
}

void FindValueResponse::_serialize(nlohmann::json& object) const {
    if (publicKey.has_value()) {
        object[Message::KEY_RES_PUBLICKEY] = publicKey.value();

        if (recipient.has_value())
            object[Message::KEY_RES_RECIPIENT] = recipient.value();

        object[Message::KEY_RES_NONCE] = nlohmann::json::binary_t {{nonce.value().cbegin(), nonce.value().cend()}};
        if (sequenceNumber >= 0)
            object[Message::KEY_RES_SEQ] = sequenceNumber;

        object[Message::KEY_RES_SIGNATURE] = nlohmann::json::binary_t {signature.value()};
    }

    if (!value.empty())
        object[Message::KEY_RES_VALUE] = nlohmann::json::binary_t {value};
}

void FindValueResponse::_parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName == Message::KEY_RES_PUBLICKEY) {
        publicKey = object.get<Id>();
    } else if (fieldName == Message::KEY_RES_RECIPIENT) {
        recipient = object.get<Id>();
    } else if (fieldName == Message::KEY_RES_NONCE) {
        nonce = CryptoBox::Nonce(Blob(object.get_binary()));
    } else if (fieldName == Message::KEY_RES_SEQ) {
        sequenceNumber = object.get<int>();
    } else if (fieldName == Message::KEY_RES_SIGNATURE) {
        signature = object.get_binary();
    } else if (fieldName == Message::KEY_RES_VALUE) {
        value = object.get_binary();
    } else {
        throw std::invalid_argument("Unknown field: " + fieldName);
    }
}

void FindValueResponse::_toString(std::stringstream& ss) const {
    if (publicKey.has_value()) {
        ss << ",k:" << publicKey.value();

        if (recipient.has_value())
            ss << ",rec:" << recipient.value();

        ss << ",n:" << Hex::encode(nonce.value().blob());
        if (sequenceNumber >= 0)
            ss << ",seq:" << std::to_string(sequenceNumber);
        ss << ",sig:" << Hex::encode(signature.value());

    }
    if (!value.empty())
        ss << ",v:" << Hex::encode(value);
}

}
}
