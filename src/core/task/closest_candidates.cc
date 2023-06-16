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

#include <map>
#include <utility>
#include <iterator>

#include "candidate_node.h"
#include "closest_candidates.h"

namespace elastos {
namespace carrier {

const Sp<CandidateNode>& ClosestCandidates::get(const Id& id) const {
    const auto it = std::find_if(closest.begin(), closest.end(), [&](const Sp<CandidateNode>& node) {
        return id == node->getId();
    });
    if (it != closest.end())
        return *it;

    static const Sp<CandidateNode> nullPtr = nullptr;
    return nullPtr;
}

const Sp<CandidateNode> ClosestCandidates::remove(const Id& id) {
    const auto it = std::find_if(closest.begin(), closest.end(), [&](const Sp<CandidateNode>& node) {
        return id == node->getId();
    });

    if (it != closest.end()) {
        const Sp<CandidateNode> removed = std::move(*it);
        closest.erase(it);
        return removed;
    }
    return nullptr;
}

const Sp<CandidateNode> ClosestCandidates::next() const {
    std::vector<Sp<CandidateNode>> candidates;
    candidates.reserve(closest.size());

    for (const auto& node: closest) {
        if (node->isEligible())
            candidates.emplace_back(node);
    }

    if (!candidates.empty()) {
        std::sort(candidates.begin(), candidates.end(), [&](const Sp<CandidateNode>& a, const Sp<CandidateNode>& b){
            return candidateOrder(a,b) < 0;
        });
        return candidates.front();
    }
    return nullptr;
}

const Id ClosestCandidates::head() const {
    if (closest.empty()) {
        return target.distance(Id::MAX_ID);
    } else {
        return closest.front()->getId();
    }
}

const Id ClosestCandidates::tail() const {
    if (closest.empty()) {
        return target.distance(Id::MAX_ID);
    } else {
        return closest.back()->getId();
    }
}

int ClosestCandidates::candidateOrder(const Sp<CandidateNode>& a, const Sp<CandidateNode>& b) const {
    int comparison = a->getPinged() - b->getPinged();
    if (comparison != 0)
        return comparison < 0 ? -1: 1;

    return target.threeWayCompare(a->getId(), b->getId());
}

void ClosestCandidates::add(const std::list<Sp<NodeInfo>>& candidates) {
    std::unique_lock<std::mutex> lock(closest_mtx);
    std::list<Sp<CandidateNode>> filtered;

    for (const auto& item: candidates) {
        if (!dedup_ids.insert(item->getId()).second)
            continue;
        if (!dedups_addrs.insert(item->getAddress()).second)
            continue;

        filtered.emplace_back(std::make_shared<CandidateNode>(*item));
    }

    filtered.sort([&](const std::shared_ptr<CandidateNode> &node1, const std::shared_ptr<CandidateNode>& node2) {
        return target.threeWayCompare(node1->getId(), node2->getId()) < 0;
    });

    closest.merge(filtered, [&](const Sp<CandidateNode>& a, const Sp<CandidateNode>& b) {
        return target.threeWayCompare(a->getId(), b->getId()) < 0;
    });

    if (closest.size() > capacity) {
        int count = 0;
        std::list<Sp<CandidateNode>> toRemove {};
        for (const auto& item : closest) {
            if (!item->isInFlight()) {
                toRemove.push_back(item);
            }
        }

        if (toRemove.size() > capacity)  {
            toRemove.sort([&](const Sp<CandidateNode>& a, const Sp<CandidateNode>& b) {
                return candidateOrder(a, b) < 0;
            });

            auto vi = toRemove.begin();
            std::advance(vi, capacity);
            for(; vi != toRemove.end(); vi++)
                remove((*vi)->getId());
        }
    }
}

} // namespace carrier
} // namespace elastos
