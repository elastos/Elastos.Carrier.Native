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

#include "carrier/node_info.h"
#include "kbucket_entry.h"
#include "kclosest_nodes.h"

#include "messages/find_node_request.h"
#include "messages/find_node_response.h"
#include "node_lookup.h"

namespace elastos {
namespace carrier {

void NodeLookup::prepare() {
    // if we're bootstrapping start from the bucket that has the greatest possible
    // distance from ourselves so we discover new things along the (longer) path
    const Id knsTarget = bootstrap ? getTarget().distance(Id::MAX_ID) : getTarget();

    // delay the filling of the todo list until we actually start the task
    auto kClosestNodes = std::make_shared<KClosestNodes>(
        getDHT(), knsTarget, Constants::MAX_ENTRIES_PER_BUCKET * 2,
        [](const Sp<KBucketEntry> entry) {
            return entry->isEligibleForNodesList();
        }
    );
    kClosestNodes->fill();
    auto nodes = kClosestNodes->asNodeList();
    addCandidates(nodes);
}

void NodeLookup::update() {
    while (canDoRequest()) {
        auto candidate = getNextCandidate();
        if (!candidate)
            return;

        auto request = std::make_shared<FindNodeRequest>(getTarget(), wantToken);
        request->setWant4(getDHT().getType() == DHT::Type::IPV4);
        request->setWant6(getDHT().getType() == DHT::Type::IPV6);

        try {
            sendCall(candidate, request, [&](Sp<RPCCall> call) {
                candidate->setSent();
            });
        } catch (const std::exception& e) {
            log->error("Error on sending 'findNode' request: " + std::string(e.what()));
        }
    }
}

void NodeLookup::callResponsed(RPCCall* call, Sp<Message> response) {
    if (!call->matchesId() ||
        response->getType() != Message::Type::RESPONSE ||
        response->getMethod() != Message::Method::FIND_NODE) {
        return;
    }

    LookupTask::callResponsed(call, response);

    auto findNodeResponse = std::static_pointer_cast<FindNodeResponse>(response);
    auto nodes = findNodeResponse->getNodes(getDHT().getType());
    if (!nodes.empty())
        addCandidates(nodes);
}

}
}
