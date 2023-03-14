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

#include "carrier/value.h"
#include "utils/misc.h"
#include "utils/hex.h"

namespace elastos {
namespace carrier {

Id Value::calculateId(const Value& value) {
    auto sha = SHA256();

    if (!static_cast<bool>(value.getPublicKey())) {
        sha.update(value.data);
    } else {
        sha.update(value.publicKey.data(), value.publicKey.size());
        sha.update(value.nonce.bytes(), value.nonce.size());
    }

    auto digest = sha.digest();
    return Id(digest.data(), digest.size());
}

void Value::createSignature() {
    auto seqNum = htonl(this->sequenceNumber);
    auto size = nonce.size() + sizeof(seqNum) + data.size();
    if (isEncrypted())
        size += recipient.size();

    std::vector<uint8_t> toSign {};
    toSign.reserve(size);
    if (isEncrypted())
        toSign.insert(toSign.end(), recipient.cbegin(), recipient.cend());

    toSign.insert(toSign.end(), nonce.cbegin(), nonce.cend());
    toSign.insert(toSign.end(), (uint8_t*)(&seqNum), (uint8_t*)(&seqNum) + sizeof(seqNum));
    toSign.insert(toSign.end(), data.begin(), data.end());

    this->signature = privateKey.sign(toSign);
}

bool Value::verifySignature() const {
    auto seqNum = htonl(this->sequenceNumber);
    auto size = nonce.size() + sizeof(seqNum) + data.size();
    if (isEncrypted())
        size += recipient.size();

    std::vector<uint8_t> toVerify {};
    toVerify.reserve(size);
    if (isEncrypted())
        toVerify.insert(toVerify.end(), recipient.cbegin(), recipient.cend());

    toVerify.insert(toVerify.end(), nonce.cbegin(), nonce.cend());
    toVerify.insert(toVerify.end(), (uint8_t*)(&seqNum), (uint8_t*)(&seqNum) + sizeof(seqNum));
    toVerify.insert(toVerify.end(), data.begin(), data.end());

    auto sender = publicKey.toKey();
    return sender->verify(signature, toVerify);
}

bool Value::isValid() const {
    assert(!data.empty());

    if (!isMutable())
        return true;

    assert(!signature.empty());
    assert(static_cast<bool>(nonce));

    return verifySignature();
}

Sp<Value> Value::create(const std::vector<uint8_t>& data) {
    assert(!data.empty());

    auto value = std::make_shared<Value>();
    value->data = data;
    return value;
}

Sp<Value> Value::createSigned(const std::vector<uint8_t>& data) {
    assert(!data.empty());

    auto value = std::make_shared<Value>();

    auto keypair = Signature::KeyPair();
    value->privateKey = keypair.privateKey();
    value->publicKey = keypair.publicKey();

    value->nonce.random();
    value->data = data;

    value->createSignature();
    return value;
}

Sp<Value> Value::createEncrypted(const Id& target, const std::vector<uint8_t>& data) {
    assert(!data.empty());

    auto value = std::make_shared<Value>();

    value->recipient = Id(target.data(), ID_BYTES);

    auto keypair = Signature::KeyPair();
    value->privateKey = keypair.privateKey();
    value->publicKey = keypair.publicKey();
    value->nonce.random();

    auto encryptionSK = CryptoBox::PrivateKey::fromSignatureKey(value->privateKey);
    auto encryptionPK = CryptoBox::PublicKey::fromSignatureKey(*target.toKey());
    value->data = CryptoBox::encrypt(data, value->nonce, encryptionPK, encryptionSK);

    value->createSignature();
    return value;
}

Sp<Value> Value::updateValue(const Sp<Value> oldValue, const std::vector<uint8_t>& newData) {
    assert(oldValue != nullptr);
    assert(oldValue->isMutable());
    assert(!oldValue->getData().empty());
    assert(!newData.empty());

    auto newValue = std::make_shared<Value>();
    newValue->publicKey = oldValue->publicKey;
    newValue->privateKey = oldValue->privateKey;
    newValue->nonce = oldValue->nonce;
    newValue->sequenceNumber = oldValue->sequenceNumber + 1;

    if (oldValue->isEncrypted()) {
        newValue->recipient = oldValue->recipient;
        auto sk = CryptoBox::PrivateKey::fromSignatureKey(newValue->privateKey);
        auto pk = CryptoBox::PublicKey::fromSignatureKey(*newValue->recipient.toKey());
        newValue->data = CryptoBox::encrypt(newData, newValue->nonce, pk, sk);
    } else {
        newValue->data = newData;
    }

    newValue->createSignature();
    return newValue;
}

bool Value::operator==(const Value& other) const {
    return (publicKey == other.publicKey && recipient == other.recipient &&
        signature == other.signature && nonce == other.nonce &&
        data == other.data && sequenceNumber == other.sequenceNumber);
}

Value::operator std::string() const {
    std::stringstream ss;
    ss.str().reserve(256);

    ss << "id:" << getId();
    if (isMutable()) {
        ss << ",publicKey:" << static_cast<std::string>(publicKey);
        ss << ",nonce: " << Hex::encode(nonce.bytes(), nonce.size());
    }
    if (isEncrypted())
        ss << ",recipient:" << static_cast<std::string>(recipient);
    if (isSigned())
        ss << ",sig:" << Hex::encode(signature);

    ss << ",seq:" << std::to_string(sequenceNumber);
    ss << ",data:" << Hex::encode(data);

    return ss.str();
}

} // namespace carrier
} // namespace elastos
