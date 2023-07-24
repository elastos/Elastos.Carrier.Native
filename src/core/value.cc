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

#include <sstream>

#include "carrier/value.h"

#include "utils/check.h"
#include "utils/misc.h"
#include "utils/hex.h"
#include "serializers.h"

namespace elastos {
namespace carrier {

Value::Value(const Blob& publicKey, const Blob& privateKey, const Blob& recipient, const Blob& nonce,
        int sequenceNumber, const Blob& signature, const Blob& data) {

    if (!publicKey.empty()) {
        if (privateKey && privateKey.size() != Signature::PrivateKey::BYTES)
            throw std::invalid_argument("Invalid private key");

        if (nonce.size() != CryptoBox::Nonce::BYTES)
            throw std::invalid_argument("Invalid nonce");

        if (sequenceNumber < 0)
            throw std::invalid_argument("Invalid sequence number");

        if (signature.size() != Signature::BYTES)
            throw std::invalid_argument("Invalid signature");

        this->publicKey = Id(publicKey);
        if (privateKey)
            this->privateKey = Signature::PrivateKey(privateKey);
        if (recipient)
            this->recipient = Id(recipient);
        this->nonce = CryptoBox::Nonce(nonce);
        this->sequenceNumber = sequenceNumber;
        this->signature = std::vector<uint8_t>(signature.cbegin(), signature.cend());
    }

    if (data.empty())
        throw std::invalid_argument("Invalid data");

    this->data = std::vector<uint8_t>(data.cbegin(), data.cend());
}

Value::Value(const Signature::KeyPair& keypair, const Id& recipient, const CryptoBox::Nonce& nonce,
        int sequenceNumber, const std::vector<uint8_t>& data) {
    if (nonce.size() != CryptoBox::Nonce::BYTES)
        throw std::invalid_argument("Invalid nonce");

    if (sequenceNumber < 0)
        throw std::invalid_argument("Invalid sequence number");

    if (data.empty())
        throw std::invalid_argument("Invalid data");

    this->publicKey = Id(keypair.publicKey());
    this->privateKey = keypair.privateKey();
    this->recipient = recipient;
    this->nonce = nonce;
    this->sequenceNumber = sequenceNumber;

    if (isEncrypted()) {
        CryptoBox::PublicKey recipientPk = recipient.toEncryptionKey();
        CryptoBox::PrivateKey ownerSk = CryptoBox::PrivateKey::fromSignatureKey(keypair.privateKey());

        this->data = CryptoBox::encrypt(data, nonce, recipientPk, ownerSk);
    } else {
        this->data = data;
    }

    this->signature = Signature::sign(getSignData(), keypair.privateKey());
}

Id Value::calculateId(const Id& publicKey, const CryptoBox::Nonce& nonce, const std::vector<uint8_t>& data) {
    auto sha = SHA256();

    if (!static_cast<bool>(publicKey)) {
        sha.update(data);
    } else {
        sha.update(publicKey.blob());
        sha.update(nonce.blob());
    }

    auto digest = sha.digest();
    return Id(digest);
}

std::vector<uint8_t> Value::getSignData() const {
    auto size = (isEncrypted() ? Id::BYTES : 0)  + CryptoBox::Nonce::BYTES +
            sizeof(sequenceNumber) + data.size();

    std::vector<uint8_t> toSign(size);
    if (isEncrypted())
        toSign.insert(toSign.end(), recipient.cbegin(), recipient.cend());

    toSign.insert(toSign.end(), nonce.cbegin(), nonce.cend());
    toSign.insert(toSign.end(), (uint8_t*)(&sequenceNumber), (uint8_t*)(&sequenceNumber) + sizeof(sequenceNumber));
    toSign.insert(toSign.end(), data.begin(), data.end());
    return toSign;
}

bool Value::isValid() const {
    if (data.empty())
        return false;

    if (isMutable()) {
        if (nonce.size() != CryptoBox::Nonce::BYTES)
            return false;

        if (signature.size() != Signature::BYTES)
            return false;

        Signature::PublicKey pk = publicKey.toSignatureKey();

        return Signature::verify(getSignData(), signature, pk);
    }

    return true;
}

Value Value::update(const std::vector<uint8_t>& data) {
    if (!isMutable())
        throw  illegal_state("Immutable value " + getId().toBase58String());

    if (!hasPrivateKey())
        throw illegal_state("Not the owner of the value " + getId().toBase58String());

    Signature::KeyPair kp = Signature::KeyPair(getPrivateKey());
    return Value(kp, recipient, nonce, sequenceNumber + 1, data);
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
        ss << ",nonce: " << Hex::encode(nonce.blob());
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
