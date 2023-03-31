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

#include <memory>

#include "messages/find_peer_request.h"
#include "messages/find_peer_response.h"
#include "task/peer_lookup.h"
#include "kclosest_nodes.h"

namespace elastos {
namespace carrier {

void PeerLookup::prepare() {
    const int MAX_ENTRIES =  Constants::MAX_ENTRIES_PER_BUCKET * 2;
    auto kClosetNodes = std::make_shared<KClosestNodes>(getDHT(), getTarget(), MAX_ENTRIES);
    kClosetNodes->fill();
    auto nodes = kClosetNodes->asNodeList();
    addCandidates(nodes);
}

void PeerLookup::update() {
    while (canDoRequest()) {
        auto candidate = getNextCandidate();
        if (!candidate)
            break;

        auto request = std::make_shared<FindPeerRequest>(getTarget());
        request->setWant4(getDHT().getType() == DHT::Type::IPV4);
        request->setWant6(getDHT().getType() == DHT::Type::IPV6);

        try {
            sendCall(candidate, request, [&](Sp<RPCCall> call) {
                candidate->setSent();
            });
        } catch (const std::exception& e) {
            log->error("Error on sending 'findPeer' request: " + std::string(e.what()));
        }
    }
}

void PeerLookup::callResponsed(RPCCall* call, Sp<Message> message) {
    if (!call->matchesId() ||
        message->getType() != Message::Type::RESPONSE ||
        message->getMethod() != Message::Method::FIND_PEER) {
        return;
    }

    LookupTask::callResponsed(call, message);

    auto response = std::static_pointer_cast<FindPeerResponse>(message);
    bool hasPeers {false };

    auto peers4 = response->getPeers4();
    if (!peers4.empty()) {
        resultHandler(peers4, this);    // peers4 is spliced in this call.
        hasPeers = true;
    }
    auto peers6 = response->getPeers6();
    if (!peers6.empty()) {
        resultHandler(peers6, this);    // peers6 is spliced in this call.
        hasPeers = true;
    }

    if (!hasPeers) {
        const auto& nodes = response->getNodes(getDHT().getType());
        if (!nodes.empty())
            addCandidates(nodes);
    }
}

}
}
