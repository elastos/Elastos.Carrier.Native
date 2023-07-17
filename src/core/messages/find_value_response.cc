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

#include "utils/hex.h"
#include "message.h"
#include "serializers.h"
#include "find_value_response.h"

namespace elastos {
namespace carrier {

void FindValueResponse::setValue(const Value& value) {
    this->publicKey = value.getPublicKey();
    this->recipient = value.getRecipient();
    this->nonce = value.getNonce();
    this->signature = value.getSignature();;
    this->sequenceNumber = value.getSequenceNumber();
    this->value = value.getData();
}

Value FindValueResponse::getValue() const {
    return Value::of(publicKey, recipient, nonce, sequenceNumber, signature, value);
}

void FindValueResponse::_serialize(nlohmann::json& object) const {
    if (publicKey != Id::zero())
        object[Message::KEY_RES_PUBLICKEY] = publicKey;

    if (recipient != Id::zero())
        object[Message::KEY_RES_RECIPIENT] = recipient;

    if (nonce.size() > 0)
        object[Message::KEY_RES_NONCE] = nlohmann::json::binary_t {{nonce.cbegin(), nonce.cend()}};

    if (sequenceNumber >= 0)
        object[Message::KEY_RES_SEQ] = sequenceNumber;

    if (!signature.empty())
        object[Message::KEY_RES_SIGNATURE] = nlohmann::json::binary_t {signature};

    if (!value.empty())
        object[Message::KEY_RES_VALUE] = nlohmann::json::binary_t {value};
}

void FindValueResponse::_parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName == Message::KEY_RES_PUBLICKEY) {
        publicKey = object.get<Id>();
    } else if (fieldName == Message::KEY_RES_RECIPIENT) {
        recipient = object.get<Id>();
    } else if (fieldName == Message::KEY_RES_NONCE) {
        nonce = object.get_binary();
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
    if (publicKey != Id::zero())
        ss << ",k:" << publicKey;

    if (recipient != Id::zero())
        ss << ",rec:" << recipient;

    if (nonce.size() > 0)
        ss << ",n:" << Hex::encode(nonce);

    if (sequenceNumber >= 0)
        ss << ",seq:" << std::to_string(sequenceNumber);

    if (!signature.empty())
        ss << ",sig:" << Hex::encode(signature);

    if (!value.empty())
        ss << ",v:" << Hex::encode(value);
}

}
}
