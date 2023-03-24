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

#include "utils/hex.h"
#include "serializers.h"
#include "store_value_request.h"

namespace elastos {
namespace carrier {

void StoreValueRequest::serializeInternal(nlohmann::json& root) const {
    nlohmann::json object = nlohmann::json::object();

    object[Message::KEY_REQ_TOKEN] = token;
    if (value->isMutable())
        object[Message::KEY_REQ_PUBLICKEY] = value->getPublicKey();
    if (value->isEncrypted())
        object[Message::KEY_REQ_RECIPIENT] = value->getRecipient();
    if (value->isMutable())
        object[Message::KEY_REQ_NONCE] = nlohmann::json::binary_t {{value->getNonce().cbegin(), value->getNonce().cend()}};
    if (value->isSigned())
        object[Message::KEY_REQ_SIGNATURE] = nlohmann::json::binary_t {value->getSignature()};
    if (value->getSequenceNumber() >= 0)
        object[Message::KEY_REQ_SEQ] = value->getSequenceNumber();
    if (expectedSequenceNumber >= 0)
        object[Message::KEY_REQ_CAS] = expectedSequenceNumber;

    object[Message::KEY_REQ_VALUE] = nlohmann::json::binary_t {value->getData()};

    Message::serializeInternal(root);
    root[getKeyString()] = object;
}

void StoreValueRequest::parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName != Message::KEY_REQUEST || !object.is_object())
        throw std::invalid_argument("Invalid request message");

    for (const auto& [key, object] : object.items()) {
        if (key == Message::KEY_REQ_PUBLICKEY) {
            value->publicKey = object.get<Id>();
        } else if (key == Message::KEY_REQ_RECIPIENT) {
            value->recipient = object.get<Id>();
        } else if (key == Message::KEY_REQ_NONCE) {
            auto _nonce = object.get_binary();
            value->nonce = CryptoBox::Nonce(_nonce.data(), _nonce.size());
        } else if (key == Message::KEY_REQ_SIGNATURE) {
            value->signature = object.get_binary();
        } else if (key == Message::KEY_REQ_SEQ) {
            value->sequenceNumber = object.get<uint16_t>();
        } else if (key == Message::KEY_REQ_CAS) {
            object.get_to(expectedSequenceNumber);
        } else if (key == Message::KEY_REQ_TOKEN) {
            object.get_to(token);
        } else if (key == Message::KEY_RES_VALUE) {
            value->data = object.get_binary();
        } else {
            throw std::invalid_argument("Unknown field: " + key);
        }
    }
}

#ifdef MSG_PRINT_DETAIL
void StoreValueRequest::toString(std::stringstream& ss) const {
    ss << "\nRequest: ";

    if (value->isMutable())
        ss << "\n    PublicKey: " << value->getPublicKey();
    if (value->isEncrypted())
        ss << "\n    Recipient: " << value->getRecipient();
    if (value->isMutable())
        ss << "\n    Nonce:" << Hex::encode(value->getNonce().bytes(), value->getNonce().size());
    if (value->isSigned())
        ss << "\n    Signature: " << Hex::encode(value->getSignature());
    if (expectedSequenceNumber >= 0)
        ss << "\n    ExpectedSequenceNumber: " << std::to_string(expectedSequenceNumber);
    if (value->getSequenceNumber() >= 0)
        ss << "\n    SequenceNumber: " << std::to_string(value->getSequenceNumber());

    ss << "\n    Token: " << std::to_string(token)
        << "\n    Value: " << Hex::encode(value->getData());
}
#else
void StoreValueRequest::toString(std::stringstream& ss) const {
    bool hasPublicKey {false};
    ss << ",q:{";

    if (value->isMutable()) {
        ss << ",k:" << static_cast<std::string>(value->getPublicKey());
        hasPublicKey = true;
    }
    if (value->isEncrypted())
        ss << ",rec:" << static_cast<std::string>(value->getRecipient());
    if (value->isMutable())
        ss << ",n:" << Hex::encode(value->getNonce().bytes(), value->getNonce().size());
    if (value->isSigned())
        ss << ",sig:" << Hex::encode(value->getSignature());
    if (expectedSequenceNumber >= 0)
        ss << ",cas:" << std::to_string(expectedSequenceNumber);
    if (value->getSequenceNumber() >= 0)
        ss << ",seq:" << std::to_string(value->getSequenceNumber());

    if (hasPublicKey)
        ss << ",";

    ss << "tok:" << std::to_string(token)
        << ",v:" << Hex::encode(value->getData())
        << "}";
}
#endif

}
}
