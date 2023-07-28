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

#include "utils/misc.h"
#include "utils/hex.h"
#include "crypto/shasum.h"
#include "exceptions/state_error.h"
#include "serializers.h"

namespace elastos {
namespace carrier {

Value::Value(const Blob& publicKey, const Blob& privateKey, const Blob& recipient, const Blob& nonce,
        int sequenceNumber, const Blob& signature, const Blob& data) {

    if (!publicKey.empty()) {
        if (!privateKey.empty() && privateKey.size() != Signature::PrivateKey::BYTES)
            throw std::invalid_argument("Invalid private key");

        if (nonce.size() != CryptoBox::Nonce::BYTES)
            throw std::invalid_argument("Invalid nonce");

        if (sequenceNumber < 0)
            throw std::invalid_argument("Invalid sequence number");

        if (signature.size() != Signature::BYTES)
            throw std::invalid_argument("Invalid signature");

        this->publicKey = std::optional<Id>(publicKey);
        if (!privateKey.empty())
            this->privateKey = std::optional<Signature::PrivateKey>(privateKey);
        if (!recipient.empty())
            this->recipient = std::optional<Id>(recipient);
        this->nonce = std::optional<CryptoBox::Nonce>(nonce);
        this->signature = std::optional<std::vector<uint8_t>>(std::vector<uint8_t>(signature.cbegin(), signature.cend()));
        this->sequenceNumber = sequenceNumber;
    }

    if (data.empty())
        throw std::invalid_argument("Invalid data");

    this->data = std::vector<uint8_t>(data.cbegin(), data.cend());
}

Value::Value(const Signature::KeyPair& keypair, const std::optional<Id>& recipient,
        const CryptoBox::Nonce& nonce, int sequenceNumber, const std::vector<uint8_t>& data) {
    if (sequenceNumber < 0)
        throw std::invalid_argument("Invalid sequence number");

    if (data.empty())
        throw std::invalid_argument("Invalid data");

    this->publicKey = std::optional<Id>(keypair.publicKey());
    this->privateKey = std::optional<Signature::PrivateKey>(keypair.privateKey());

    this->nonce = nonce;
    this->sequenceNumber = sequenceNumber;

    if (recipient.has_value()) {
        CryptoBox::PublicKey recipientPk = recipient->toEncryptionKey();
        CryptoBox::PrivateKey ownerSk = CryptoBox::PrivateKey::fromSignatureKey(keypair.privateKey());

        this->recipient = std::optional<Id>(recipient);
        this->data = CryptoBox::encrypt(data, nonce, recipientPk, ownerSk);
    } else {
        this->data = data;
    }

    const auto signData = Signature::sign(getSignData(), keypair.privateKey());
    this->signature = std::optional<std::vector<uint8_t>>(std::vector<uint8_t>(signData.cbegin(), signData.cend()));
}

Id Value::calculateId(const std::optional<Id>& publicKey, const std::optional<CryptoBox::Nonce>& nonce,
        const std::vector<uint8_t>& data) {
    auto sha = SHA256();
    if (publicKey.has_value()) {
        sha.update(publicKey->blob());
        sha.update(nonce->blob());
    } else {
        sha.update(data);
    }

    return Id(sha.digest());
}

std::vector<uint8_t> Value::getSignData() const {
    auto size = (isEncrypted() ? Id::BYTES : 0)  + CryptoBox::Nonce::BYTES +
            sizeof(sequenceNumber) + data.size();

    std::vector<uint8_t> toSign(size);
    if (isEncrypted())
        toSign.insert(toSign.end(), recipient.value().cbegin(), recipient.value().cend());

    toSign.insert(toSign.end(), nonce.value().cbegin(), nonce.value().cend());
    toSign.insert(toSign.end(), (uint8_t*)(&sequenceNumber), (uint8_t*)(&sequenceNumber) + sizeof(sequenceNumber));
    toSign.insert(toSign.end(), data.begin(), data.end());
    return toSign;
}

bool Value::isValid() const {
    if (data.empty())
        return false;

    if (isMutable()) {
        assert(nonce.has_value() && nonce.value().size() == CryptoBox::Nonce::BYTES);
        assert(signature.has_value() && signature.value().size() == Signature::BYTES);

        auto pk = publicKey->toSignatureKey();
        return Signature::verify(getSignData(), signature.value(), pk);
    }

    return true;
}

Value Value::update(const std::vector<uint8_t>& data) {
    if (!isMutable())
        throw StateError("Immutable value " + getId().toBase58String());

    if (!hasPrivateKey())
        throw StateError("Not the owner of the value " + getId().toBase58String());

    auto kp = Signature::KeyPair(getPrivateKey());
    return Value(kp, recipient, nonce.value(), sequenceNumber + 1, data);
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
        ss << ",publicKey:" << static_cast<std::string>(publicKey.value());
        ss << ",nonce: " << Hex::encode(nonce.value().blob());
    }
    if (isEncrypted())
        ss << ",recipient:" << static_cast<std::string>(recipient.value());
    if (isSigned())
        ss << ",sig:" << Hex::encode(signature.value());

    ss << ",seq:" << std::to_string(sequenceNumber);
    ss << ",data:" << Hex::encode(data);

    return ss.str();
}

} // namespace carrier
} // namespace elastos
