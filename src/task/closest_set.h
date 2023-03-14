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

#include <map>
#include "candidate_node.h"

namespace elastos {
namespace carrier {

class ClosestSet {
public:
    ClosestSet(const Id& _target, int _capacity)
        : target(_target), capacity(_capacity) {}

    bool reachedCapacity() const {
        return closest.size() >= capacity;
	}

	int size() const {
        return closest.size();
	}

    Sp<CandidateNode> get(const Id& id) {
        return closest[id];
    }

    bool contains(const Id& id) const {
        return closest.find(id) != closest.end();
    }

    void add(const Sp<CandidateNode>& cn) {
        closest[cn->getId()] = cn;
        if (closest.size() > capacity) {
            const auto last = std::prev(closest.cend())->second;
            closest.erase(closest.cend());

            if (last == cn) {
                insertAttemptsSinceTailModification++;
            } else {
                insertAttemptsSinceTailModification = 0;
            }
        }

        const auto head = closest.cbegin()->second;
        if (head == cn) {
            insertAttemptsSinceHeadModification = 0;
        } else {
            insertAttemptsSinceHeadModification++;
        }
    }

    void removeCandidate(const Id& id) {
        if (!closest.empty())
            closest.erase(id);
	}

    const std::list<Sp<CandidateNode>> getEntries() const {
        std::list<Sp<CandidateNode>> entries {};
        for (const auto& item: closest) {
            entries.push_back(item.second);
        }
        return entries;
    }

    Id tail() const {
        if (closest.empty())
            return target.distance(Id::MAX_ID);

        return (std::prev(closest.cend()))->first;
	}

	Id head() const {
        if (closest.empty())
            return target.distance(Id::MAX_ID);

        return closest.cbegin()->first;
	}

	bool isEligible() const {
		return reachedCapacity() && insertAttemptsSinceTailModification > capacity;
	}

private:
    const Id& target;
    int capacity;

    std::map<Id, Sp<CandidateNode>> closest {};

    int insertAttemptsSinceTailModification {0};
	int insertAttemptsSinceHeadModification {0};
};

} // namespace carrier
} // namespace elastos

