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
#include <utf8proc.h>

#include "carrier/peer_info.h"
#include "carrier/socket_address.h"
#include "utils/hex.h"

namespace elastos {
namespace carrier {

PeerInfo::PeerInfo(const Id& peerId, const Blob& privateKey, const Id& nodeId, const Id& origin, uint16_t port,
			const std::string& alternativeURL, const std::vector<uint8_t>& signature) {

    if (peerId == Id::zero())
		throw std::invalid_argument("Invalid peer id");

    if (privateKey && privateKey.size() != Signature::PrivateKey::BYTES)
        throw std::invalid_argument("Invalid private key");

    if (nodeId == Id::zero())
        throw std::invalid_argument("Invalid node id");

    if (port <= 0 || port > 65535)
        throw std::invalid_argument("Invalid port");

    if (signature.size() != Signature::BYTES)
        throw std::invalid_argument("Invalid signature");

    this->publicKey = peerId;
    this->privateKey = privateKey;
    this->nodeId = nodeId;
    this->delegated = origin != Id::zero();
    this->origin = this->delegated ? origin : nodeId;
    this->port = port;
    if (!alternativeURL.empty())
        this->alternativeURL = (char *)utf8proc_NFC((unsigned char *)(alternativeURL.c_str()));
    this->signature = signature;
}

PeerInfo::PeerInfo(const Signature::KeyPair& keypair, const Id& nodeId, const Id& origin, uint16_t port,
			const std::string& alternativeURL) {

    if (nodeId == Id::zero())
        throw std::invalid_argument("Invalid node id");

    if (port <= 0 || port > 65535)
        throw std::invalid_argument("Invalid port");

    this->publicKey = Id(keypair.publicKey());
    this->privateKey = keypair.privateKey().blob();
    this->nodeId = nodeId;
    this->delegated = origin != Id::zero();
    this->origin = this->delegated ? origin : nodeId;
    this->port = port;
    if (!alternativeURL.empty())
        this->alternativeURL = (char *)utf8proc_NFC((unsigned char *)(alternativeURL.c_str()));
    this->signature = Signature::sign(getSignData(), keypair.privateKey());
}

bool PeerInfo::operator==(const PeerInfo& other) const {
    return publicKey == other.publicKey && nodeId == other.nodeId && origin == other.origin
                && port == other.port && alternativeURL == other.alternativeURL && signature == other.signature;
}

PeerInfo::operator std::string() const {
    std::stringstream ss;
    // ss.str().reserve(80);
    ss << "<" << nodeId.toBase58String() << ",";

    if (isDelegated())
        ss<< origin.toBase58String() << ",";
    ss << std::to_string(port);
    if (hasAlternativeURL())
        ss << "," << alternativeURL;
    ss << ">";

    return ss.str();
}

std::ostream& operator<< (std::ostream& os, const PeerInfo& pi) {
    os << static_cast<std::string>(pi);
    return os;
}

std::vector<uint8_t> PeerInfo::getSignData() const {
    auto size = Id::BYTES * 2  + sizeof(port) + alternativeURL.size();

    std::vector<uint8_t> toSign {};
    toSign.reserve(size);
    toSign.insert(toSign.begin(), nodeId.cbegin(), nodeId.cend());
    toSign.insert(toSign.end(), origin.cbegin(), origin.cend());
    toSign.insert(toSign.end(), (uint8_t*)(&port), (uint8_t*)(&port) + sizeof(port));
    const uint8_t* ptr = (const uint8_t*)alternativeURL.c_str();
    toSign.insert(toSign.end(), ptr, ptr + strlen(alternativeURL.c_str()));
    return toSign;
}

bool PeerInfo::isValid() const {
    if (signature.size() != Signature::BYTES)
			return false;

    Signature::PublicKey pk = publicKey.toSignatureKey();
    return Signature::verify(getSignData(), signature, pk);
}


}
}
