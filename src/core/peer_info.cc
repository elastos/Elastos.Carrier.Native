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

#include "carrier/peer_info.h"
#include "carrier/socket_address.h"
#include "utils/hex.h"

namespace elastos {
namespace carrier {

PeerInfo::PeerInfo(const Blob& id, const Blob& pid, uint16_t _port, const std::string& _alt, const Blob& sig, int _family)
    : nodeId(id), proxyId(pid), port(_port), alt(_alt), family(_family) {
    setSignature(sig);
    if (proxyId != Id::zero()) {
        proxied = true;
    }
}

PeerInfo::PeerInfo(const Blob& id, uint16_t port, const std::string& alt, const Blob& sig, int family) {
    PeerInfo(id, {}, port, alt, sig, family);
}

PeerInfo::PeerInfo(const Id& id, const Id& pid, uint16_t _port, const std::string& _alt,  const std::vector<uint8_t>& sig, int _family)
    : nodeId(id), proxyId(pid), port(_port), alt(_alt), signature(sig), family(_family) {
    if (proxyId != Id::zero()) {
        proxied = true;
    }
}

PeerInfo::PeerInfo(const PeerInfo& pi)
    : nodeId(pi.nodeId), proxyId(pi.proxyId), port(pi.port), alt(pi.alt), signature(pi.signature), family(pi.family), proxied(pi.proxied) {}


bool PeerInfo::operator==(const PeerInfo& other) const {
    return nodeId == other.nodeId && proxyId == other.proxyId && port == other.port
                && alt == other.alt && signature == other.signature && family == other.family;
}

PeerInfo::operator std::string() const {
    std::stringstream ss;
    ss.str().reserve(80);
    ss << "<" << nodeId.toBase58String();
    if (proxied)
        ss << "," << proxyId.toBase58String();
    ss << "," << std::to_string(port);
    if (!alt.empty())
        ss << "," << alt;
    ss << ",sig:" << Hex::encode(signature)
        << ">";

    return ss.str();
}

std::ostream& operator<< (std::ostream& os, const PeerInfo& pi) {
    os << static_cast<std::string>(pi);
    return os;
}

std::vector<uint8_t> PeerInfo::getSignData(const Id& nodeId, const Id& proxyId, uint16_t port, const std::string& alt) {
    bool proxied = false;
    if (proxyId != Id::zero()) {
        proxied = true;
    }
    auto size = nodeId.size() + sizeof(port) + alt.size();
    if (proxied)
        size += proxyId.size();

    std::vector<uint8_t> toSign {};
    toSign.reserve(size);
    toSign.insert(toSign.begin(), nodeId.cbegin(), nodeId.cend());
    if (proxied)
        toSign.insert(toSign.end(), proxyId.cbegin(), proxyId.cend());

    toSign.insert(toSign.end(), (uint8_t*)(&port), (uint8_t*)(&port) + sizeof(port));
    const uint8_t* ptr = (const uint8_t*)alt.c_str();
    toSign.insert(toSign.end(), ptr, ptr + strlen(alt.c_str()));
    return toSign;
}

bool PeerInfo::verifySignature() const {
    std::vector<uint8_t> toVerify = getSignData(nodeId, proxyId, port, alt);

    Signature::PublicKey sender {};
    if (proxied)
        sender = Signature::PublicKey(proxyId.blob());
    else
        sender = Signature::PublicKey(nodeId.blob());

   return sender.verify(signature, toVerify);
}

bool PeerInfo::isValid() const {
    // assert(!signature.empty());
    return verifySignature();
}


}
}
