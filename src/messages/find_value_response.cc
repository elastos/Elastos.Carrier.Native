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
#include "find_value_response.h"

namespace elastos {
namespace carrier {

void FindValueResponse::_serialize(nlohmann::json& object) const {
    if (value->isMutable())
        object[Message::KEY_RES_PUBLICKEY] = value->getPublicKey();
    if (value->isEncrypted())
        object[Message::KEY_RES_RECIPIENT] = value->getRecipient();
    if (value->isMutable())
        object[Message::KEY_RES_NONCE] = nlohmann::json::binary_t {{value->getNonce().cbegin(), value->getNonce().cend()}};
    if (value->isSigned())
        object[Message::KEY_RES_SIGNATURE] = nlohmann::json::binary_t {value->getSignature()};
    if (value->getSequenceNumber() >= 0)
        object[Message::KEY_RES_SEQ] = value->getSequenceNumber();
    object[Message::KEY_RES_VALUE] = nlohmann::json::binary_t {value->getData()};
}

void FindValueResponse::_parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName == Message::KEY_RES_PUBLICKEY) {
        value->setPublicKey(object);
    } else if (fieldName == Message::KEY_RES_RECIPIENT) {
        value->setRecipient(object);
    } else if (fieldName == Message::KEY_RES_NONCE) {
       value->setNonce(object);
    } else if (fieldName == Message::KEY_RES_SIGNATURE) {
        value->setSignature(object);
    } else if (fieldName == Message::KEY_RES_SEQ) {
        value->setSequenceNumber(object.get<uint16_t>());
    } else if (fieldName == Message::KEY_RES_VALUE) {
        value->setData(object);
    } else {
        throw std::invalid_argument("Unknown field: " + fieldName);
    }
}

void FindValueResponse::_toString(std::stringstream& ss) const {
    if (value->isMutable())
        ss << ",k:" << value->getPublicKey();
    if (value->isEncrypted())
        ss << ",rec:" << value->getRecipient();
    if (value->isMutable())
        ss << ",n:" << Hex::encode(value->getNonce().bytes(), value->getNonce().size());
    if (value->isSigned())
        ss << ",sig:" << Hex::encode(value->getSignature());
    if (value->getSequenceNumber() >= 0)
        ss << ",seq:" << std::to_string(value->getSequenceNumber());
    ss << ",v:" << Hex::encode(value->getData());
}

}
}
