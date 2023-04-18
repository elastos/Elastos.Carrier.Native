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

#include "carrier/node.h"
#include "messages/lookup_response.h"
#include "task/closest_candidates.h"
#include "task/lookup_task.h"
#include "constants.h"
#include "dht.h"

namespace elastos {
namespace carrier {

void LookupTask::addCandidates(const std::list<Sp<NodeInfo>>& nodes) {
    std::list<Sp<NodeInfo>> candidates {};

    for(const auto& node: nodes) {
        if (getDHT().isSelfAddress(node->getAddress()) ||
            isBogonAddress(node->getAddress()) ||
            getDHT().getNode().isLocalId(node->getId()) ||
            closestSet.contains(node->getId())) {
            continue;
        }
        candidates.push_back(node);
    }

    if (!candidates.empty())
        closestCandidates.add(candidates);
}

bool LookupTask::isDone() const {
    return Task::isDone() &&
        (closestCandidates.size() == 0 ||
            (closestSet.isEligible() &&
                target.threeWayCompare(closestSet.tail(), closestCandidates.head()) <= 0));
}

void LookupTask::callError(RPCCall* call) {
    closestCandidates.remove(call->getTargetId());
}

void LookupTask::callTimeout(RPCCall* call) {
    auto candidateNode = std::static_pointer_cast<CandidateNode>(call->getTarget());
    if (candidateNode->isUnreachable()) {
        closestCandidates.remove(candidateNode->getId());
        return;
    }

    // Clear the sent time-stamp and make it available again for the next retry
    candidateNode->clearSent();
}

void LookupTask::callResponsed(RPCCall* call, Sp<Message> response) {
    auto candidateNode = removeCandidate(call->getTargetId());
    if (candidateNode == nullptr)
        return;

    candidateNode->setReplied();
    candidateNode->setToken((std::static_pointer_cast<LookupResponse>(response))->getToken());
    addClosest(candidateNode);
}

bool LookupTask::isBogonAddress(const SocketAddress &addr) const {
#ifdef CARRIER_DEVELOPMENT
    return !addr.isAnyUnicast();
#else
    return addr.isBogon();
#endif
}


} // namespace carrier
} // namespace elastos
