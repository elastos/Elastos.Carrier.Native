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

#include "serializers.h"
#include "message_error.h"
#include "find_peer_response.h"

namespace elastos {
namespace carrier {

int FindPeerResponse::estimateSize() const {
    int size = LookupResponse::estimateSize();
    if (peers.empty())
        return size;

    size += (2 + 2 + 2 + Id::BYTES);
    for (const auto&  pi : peers) {
        size += (2 + 2 + Id::BYTES + 1 + sizeof(uint16_t) + 2 + Signature::BYTES);
        size += pi.isDelegated() ? 2 + Id::BYTES : 1;
        size += pi.hasAlternativeURL() ? 2 + strlen(pi.getAlternativeURL().c_str()) : 1;
    }
    return size;
}

void FindPeerResponse::_serialize(nlohmann::json& json) const {
    if (peers.empty())
        return;

    auto array = nlohmann::json::array();
    array.push_back(peers.front().getId());
    for (const auto& peer: peers) {
        auto ar = nlohmann::json::array();
        ar.push_back(peer.getNodeId());
        if (peer.isDelegated())
            ar.push_back(peer.getOrigin());
        else
            ar.push_back(nullptr);
        ar.push_back(peer.getPort());
        if (peer.hasAlternativeURL())
            ar.push_back(peer.getAlternativeURL());
        else
            ar.push_back(nullptr);
        ar.push_back(nlohmann::json::binary_t(peer.getSignature()));

        array.push_back(ar);
    }

    json[Message::KEY_RES_PEERS] = array;
}

void FindPeerResponse::_parse(const std::string& fieldName, nlohmann::json& object) {
    if (!object.is_array())
        throw MessageError("Invalid response peers message");

    if (fieldName != KEY_RES_PEERS)
        throw MessageError("invalid find peer response message");

    Blob peerId {};
    for (nlohmann::json::iterator it = object.begin(); it != object.end(); ++it) {
        if (!it->is_array()) {
            peerId = it->get_binary();
        } else {
            auto id = it->at(0).get_binary();

            Blob origin {};
            if (!it->at(1).is_null())
                origin =it->at(1).get_binary();

            auto port = it->at(2).get<uint16_t>();

            std::string alt {};
            if (!it->at(3).is_null())
                alt = it->at(3).get<std::string>();

            auto sig = it->at(4).get_binary();

            auto pi = PeerInfo::of(peerId, {}, id, origin, port, alt, sig);
            peers.emplace_back(pi);
        }
    }
}

#ifdef MSG_PRINT_DETAIL
void FindPeerResponse::_toString(std::stringstream& ss) const {
    if (!peers4.empty()) {
        ss << "\n    peers:\n";
        for (const auto& peer: peers) {
            ss <<""      "" << peer << "\n";
        }
    }
}
#else
void FindPeerResponse::_toString(std::stringstream& ss) const {
    if (!peers.empty()) {
        ss << ",p:";
        bool first_peer {true};
        for (const auto& peer: peers) {
            ss << (first_peer ? "" : ",") << "[" << peer << "]";
            first_peer = false;
        }
    }
}
#endif

} // namespace carrier
} // namespace elastos
