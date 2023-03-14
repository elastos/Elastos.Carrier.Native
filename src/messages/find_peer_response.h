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

#pragma once

#include <list>
#include "lookup_response.h"
#include "carrier/peer_info.h"

namespace elastos {
namespace carrier {

class FindPeerResponse : public LookupResponse {
public:
    FindPeerResponse(int txid) : LookupResponse(Message::Method::FIND_PEER, txid) {}
    FindPeerResponse() : FindPeerResponse(0) {}

    void setPeers4(const std::list<Sp<PeerInfo>>& peers) {
        this->peers4 = peers;
    }

    const std::list<Sp<PeerInfo>>& getPeers4() const {
        return peers4;
    }

    void setPeers6(const std::list<Sp<PeerInfo>>& peers) {
        this->peers6 = peers;
    }

    const std::list<Sp<PeerInfo>>& getPeers6() const {
        return peers6;
    }

    int estimateSize() const override;

protected:
    void _serialize(nlohmann::json& json) const override;
    void _parse(const std::string& fieldName, nlohmann::json& object) override;
    void _toString(std::stringstream& ss) const override;

private:
    void serializePeers(nlohmann::json& object, const std::string& fieldName, const std::list<Sp<PeerInfo>>& peers) const;
    void parsePeers(const nlohmann::json& object, std::list<Sp<PeerInfo>>& to);

    std::list<Sp<PeerInfo>> peers4 = {};
    std::list<Sp<PeerInfo>> peers6 = {};
};

}
}
