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

#pragma once

#include <string>
#include <vector>

#include "carrier/id.h"
#include "carrier/peer_info.h"
#include "message.h"

namespace elastos {
namespace carrier {

class AnnouncePeerRequest : public Message {
public:
    AnnouncePeerRequest()
        : Message(Message::Type::REQUEST, Message::Method::ANNOUNCE_PEER) {}

    AnnouncePeerRequest(const PeerInfo& peer, int token)
            : Message(Message::Type::REQUEST, Message::Method::ANNOUNCE_PEER){
		setPeer(peer);
		setToken(token);
    }

    int getToken() const {
        return token;
    }

    void setToken(int token) {
        this->token = token;
    }

    void setPeer(const PeerInfo& peer) {
		peerId = peer.getId();
		nodeId = peer.getNodeId();
		port = peer.getPort();
		if (peer.hasAlternativeURL())
			alternativeURL = peer.getAlternativeURL();
		signature = peer.getSignature();
	}

	PeerInfo getPeer() {
		if (nodeId == Id::zero())
			nodeId = getId();

		return PeerInfo::of(peerId, nodeId, getId(), port, alternativeURL, signature);
	}

    const Id& getTarget() const {
        return peerId;
    }

    int estimateSize() const override;


protected:
    void serializeInternal(nlohmann::json& root) const override;
    void parse(const std::string& fieldName, nlohmann::json& object) override;
    void toString(std::stringstream& ss) const override;

private:
    int token;
    Id peerId;
    Id nodeId; // Optional, only for the delegated peers
    uint16_t port;
    std::string alternativeURL {};
    std::vector<uint8_t> signature {};
};

}
}


