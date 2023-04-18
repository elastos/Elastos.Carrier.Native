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

#include "carrier/id.h"
#include "closest_set.h"
#include "closest_candidates.h"
#include "task.h"

namespace elastos {
namespace carrier {

class LookupTask : public Task {
public:
    LookupTask(DHT* dht, const Id& id, const std::string& taskName)
        : Task(dht, taskName), target(id),
        closestSet(target, Constants::MAX_ENTRIES_PER_BUCKET),
        closestCandidates(target, Constants::MAX_ENTRIES_PER_BUCKET * 3) {}

    const Id& getTarget() const {
        return target;
    }

    const Sp<CandidateNode> getCandidate(const Id& id) const {
        return closestCandidates.get(id);
    }

    const ClosestSet& getClosestSet() const {
        return closestSet;
    }

protected:
    void addCandidates(const std::list<Sp<NodeInfo>>& nodes);

    Sp<CandidateNode> removeCandidate(const Id& id) {
        return closestCandidates.remove(id);
    }

    Sp<CandidateNode> getNextCandidate() const {
        return closestCandidates.next();
    }

    void addClosest(Sp<CandidateNode> candidateNode) {
        closestSet.add(candidateNode);
    }

    bool isDone() const override;
    void callResponsed(RPCCall* call, Sp<Message> response) override;
    void callError(RPCCall* call) override;
    void callTimeout(RPCCall* call) override;

private:
    bool isBogonAddress(const SocketAddress& addr) const;
    bool isSelfAddress(const SocketAddress& addr) const;

    Id target;
    ClosestSet closestSet;
    ClosestCandidates closestCandidates;
};

} // namespace carrier
} // namespace elastos
