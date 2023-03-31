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

#include "carrier/configuration.h"
#include "carrier/prefix.h"
#include "carrier/node.h"

#include "utils/list.h"

#include "dht.h"
#include "kbucket.h"
#include "kbucket_entry.h"
#include "routing_table.h"
#include "kclosest_nodes.h"

namespace elastos {
namespace carrier {

KClosestNodes::KClosestNodes(DHT& _dht, const Id& _id, int _maxEntries)
    : KClosestNodes(_dht, _id, _maxEntries, [](const Sp<KBucketEntry>& entry) {
        return entry->isEligibleForNodesList();
}) {}

KClosestNodes::KClosestNodes(DHT& _dht, const Id& _id, int _maxEntries, std::function<bool(const Sp<KBucketEntry>&)> _filter)
    : dht(_dht), target(_id), maxEntries(_maxEntries), filter(_filter) {}

void KClosestNodes::insertEntries(Sp<KBucket> bucket) {
    for (const auto& entry: bucket->getEntries()) {
        if (filter(entry))
            entries.emplace_back(entry);
    }
}

void KClosestNodes::shave() {
    int overshoot = entries.size() - maxEntries;
    if (overshoot <= 0)
        return;

    entries.sort([&](const Sp<KBucketEntry> &a, const Sp<KBucketEntry>& b) {
        return target.threeWayCompare(a->getId(), b->getId()) < 0;
    });

    auto it = entries.begin();
    std::advance(it, entries.size() - overshoot);

    std::list<Sp<KBucketEntry>> obsolete {};
    obsolete.splice(obsolete.begin(), entries, it, entries.end());
    // Here obsolete list resource would be freed along with all kbucketEntry
    // inside.
}

void KClosestNodes::fill(bool includeSelf) {
    auto& buckets = dht.getRoutingTable().getBuckets();
    int idx = RoutingTable::indexOf(buckets, target);
    auto kb = list_get(buckets,idx);
    insertEntries(kb);

    int low = idx;
    int high = idx;
    while (entries.size() < maxEntries) {
        Sp<KBucket> lowBucket {};
        Sp<KBucket> highBucket {};

        if (low > 0)
            lowBucket = list_get(buckets, low - 1);

        if (high < buckets.size() - 1)
            highBucket = list_get(buckets, high + 1);

        if (!lowBucket && !highBucket)
            break;

        if (!lowBucket) {
            high++;
            insertEntries(highBucket);
        } else if (!highBucket) {
            low--;
            insertEntries(lowBucket);
        } else {
            int dir = target.threeWayCompare(lowBucket->getPrefix().last(), highBucket->getPrefix().first());
            if (dir < 0) {
                low--;
                insertEntries(lowBucket);
            } else if (dir > 0) {
                high++;
                insertEntries(highBucket);
            } else {
                low--;
                high++;
                insertEntries(lowBucket);
                insertEntries(highBucket);
            }
        }
    }

    if (entries.size() < maxEntries) {
        for (const auto& bootstrapNode : dht.getNode().getConfig()->getBootstrapNodes()) {
            if (dht.canUseSocketAddress(bootstrapNode->getAddress()))
                entries.push_back(std::static_pointer_cast<KBucketEntry>(bootstrapNode));
        }
    }

    if (entries.size() < maxEntries && includeSelf) {
        const auto& sockAddr = dht.getOrigin();
        entries.push_back(std::make_shared<KBucketEntry>(dht.getNode().getId(), sockAddr));
    }
    shave();
}

}
}
