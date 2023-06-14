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
#include "find_peer_response.h"

namespace elastos {
namespace carrier {

int FindPeerResponse::estimateSize() const {
    int size = LookupResponse::estimateSize();

    for (const auto&  peer : peers4) {
        size += peer->estimateSize();
    }

    for (const auto& peer : peers6) {
        size += peer->estimateSize();
    }

    return size;
}

void FindPeerResponse::_serialize(nlohmann::json& json) const {
    if (!peers4.empty())
        serializePeers(json, Message::KEY_RES_PEERS4, peers4);
    if (!peers6.empty())
        serializePeers(json, Message::KEY_RES_PEERS6, peers6);
}

void FindPeerResponse::serializePeers(nlohmann::json& object, const std::string& fieldName, const std::list<Sp<PeerInfo>>& peers) const {
    auto array = nlohmann::json::array();
    for (const auto& peer: peers)
        array.push_back(*peer);

    object[fieldName] = array;
}

void FindPeerResponse::_parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName == KEY_RES_PEERS4) {
        parsePeers(object, peers4, AF_INET);
    } else if (fieldName == Message::KEY_RES_PEERS6) {
        parsePeers(object, peers6, AF_INET6);
    } else {
        throw std::invalid_argument("invalid find peer response message");
    }
}

void FindPeerResponse::parsePeers(const nlohmann::json& object, std::list<Sp<PeerInfo>>& peers, int family) {
    if (!object.is_array())
        throw std::invalid_argument("Invalid response peers message");

    for (const auto& elem : object) {
        auto peer = std::make_shared<PeerInfo>(elem.get<PeerInfo>());
        peer->setFamily(family);
        peers.emplace_back(peer);
    }
}

#ifdef MSG_PRINT_DETAIL
void FindPeerResponse::_toString(std::stringstream& ss) const {
    if (!peers4.empty()) {
        ss << "\n    peer4:\n";
        for (const auto& peer: peers4) {
            ss <<""      "" << *peer << "\n";
        }
    }
    if (!peers6.empty()) {
        ss << "\n    peer6:\n";
        for (const auto& peer: peers4) {
            ss << ""      "" << *peer << "\n";
        }
    }
}
#else
void FindPeerResponse::_toString(std::stringstream& ss) const {
    if (!peers4.empty()) {
        ss << "p4:";
        bool first_peer4 {true};
        for (const auto& peer: peers4) {
            ss << (first_peer4 ? "" : ",") << "[" << *peer << "]";
            first_peer4 = false;
        }
        ss << ",";
    }
    if (!peers6.empty()) {
        ss << "p6:";
        bool first_peer6 {true};
        for (const auto& peer: peers6) {
            ss << (first_peer6 ? "" : ",") << "[" << *peer << "]";
            first_peer6 = false;
        }
    }
}
#endif

} // namespace carrier
} // namespace elastos
