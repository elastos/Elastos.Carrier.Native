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
#include "utils/time.h"
#include "kbucket.h"
#include "routing_table.h"
#include "carrier/node.h"
#include "dht.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace elastos {
namespace carrier {

int RoutingTable::indexOf(const std::list<Sp<KBucket>>& bucketsRef, const Id& id) {
    int low = 0;
    int mid = 0;
    int cmp = 0;
    int high = bucketsRef.size() - 1;

    while (low <= high) {
        mid = (low + high) >> 1;
        auto bucket = list_get(bucketsRef, mid);
        cmp = id.compareTo(bucket->getPrefix());
        if (cmp > 0)
            low = mid + 1;
        else if (cmp < 0)
            high = mid - 1;
        else  // match the current bucket
            return mid;
    }

    return cmp < 0 ? mid - 1 : mid;
}

void RoutingTable::_put(const Sp<KBucketEntry>& entry) {
    auto& nodeId = entry->getId();
    auto bucket = getBucket(nodeId);

    while (_needsSplit(bucket, entry)) {
        _split(bucket);
        bucket = getBucket(nodeId);
    }

    bucket->_put(entry);
}

void RoutingTable::_remove(const Id& id) {
    auto bucket = getBucket(id);
    auto toRemove = bucket->get(id);
    if (toRemove != nullptr)
        bucket->_removeIfBad(toRemove, true);
}

void RoutingTable::_onTimeout(const Id& id) {
    getBucket(id)->_onTimeout(id);
}

void RoutingTable::_onSend(const Id& id) {
    getBucket(id)->_onSend(id);
}

bool RoutingTable::_needsSplit(const Sp<KBucket>& bucket, const Sp<KBucketEntry>& newEntry) {
    if (!bucket->getPrefix().isSplittable()
        || !bucket->isFull()
        || !newEntry->isReachable()
        || bucket->exists(newEntry->getId())
        || bucket->needsReplacement())
        return false;

    auto high = bucket->getPrefix().splitBranch(true);
    return high.isPrefixOf(newEntry->getId());
}

void RoutingTable::_modify(const std::vector<Sp<KBucket>>& toRemove, const std::vector<Sp<KBucket>>& toAdd) {
    std::list<Sp<KBucket>> newBuckets(getBuckets());

    if (!toRemove.empty())
        list_removeAll(newBuckets, toRemove);
    if (!toAdd.empty())
        list_addAll(newBuckets, toAdd);

    assert(newBuckets.size() != 0);

    auto sortKBucket = [](Sp<KBucket> a, Sp<KBucket> b) -> bool {
        return a->compareTo(b) < 0;
    };
    newBuckets.sort(sortKBucket);
    setBuckets(newBuckets);
}

void RoutingTable::_split(const Sp<KBucket>& bucket) {
    assert(bucket.get());

    auto& prefix = bucket->getPrefix();
    const Prefix& pl = prefix.splitBranch(false);
    Sp<KBucket> l = std::make_shared<KBucket>(pl, isHomeBucket(pl));
    const Prefix& ph = prefix.splitBranch(true);
    Sp<KBucket> h = std::make_shared<KBucket>(ph, isHomeBucket(ph));

    for (auto& entry: bucket->getEntries()) {
        if (l->getPrefix().isPrefixOf(entry->getId()))
            l->_put(entry);
        else
            h->_put(entry);
    }

    std::vector<Sp<KBucket>> toRemove {bucket};
    std::vector<Sp<KBucket>> toAdd {l, h};

    _modify(toRemove, toAdd);
}

void RoutingTable::_mergeBuckets() {
    int i = 0;

    // perform bucket merge operations where possible
    while (true) {
        i++;
        if (i < 1)
            continue;

        auto& bucketsRef = getBuckets();
        if (i >= bucketsRef.size())
            break;

        Sp<KBucket> b1 = list_get(bucketsRef, i - 1);
        Sp<KBucket> b2 = list_get(bucketsRef, i);

        if (b1->getPrefix().isSiblingOf(b2->getPrefix())) {
            b1->getEntries();

            auto getEffectiveSize = [](Sp<KBucket> bucket) {
                int count = 0;
                for (auto& entry: bucket->getEntries()) {
                    if (!entry->removableWithoutReplacement())
                        count++;

                }
                return count;
            };
            int effectiveSize1 = getEffectiveSize(b1);
            int effectiveSize2 = getEffectiveSize(b2);;

            // check if the buckets can be merged without losing any effective entries
            if (effectiveSize1 + effectiveSize2 <= Constants::MAX_ENTRIES_PER_BUCKET) {
                // Insert into a new bucket directly, no splitting to avoid
                // fibrillation between merge and split operations
                auto parent = b1->getPrefix().getParent();
                auto newBucket = std::make_shared<KBucket>(parent, isHomeBucket(parent));

                for (auto& entry: b1->getEntries()) {
                    newBucket->_put(entry);
                }
                for (auto& entry: b2->getEntries()) {
                    newBucket->_put(entry);
                }

                std::vector<Sp<KBucket>> toRemove {b1, b2};
                std::vector<Sp<KBucket>> toAdd {newBucket};
                _modify(toRemove, toAdd);

                i -= 2;
            }
        }
    }
}

/**
 * Check if a buckets needs to be refreshed, and refresh if necessary.
 */
void RoutingTable::_maintenance() {
    uint64_t now = currentTimeMillis();

    // don't spam the checks if we're not receiving anything.
    // we don't want to cause too many stray packets somewhere in a network
    // if (!isRunning() && now - timeOfLastPingCheck < Constants.BOOTSTRAP_MIN_INTERVAL)
    if (now - timeOfLastPingCheck < Constants::ROUTING_TABLE_MAINTENANCE_INTERVAL)
        return;

    timeOfLastPingCheck = now;

    _mergeBuckets();

    const Id& localId = dht.getNode().getId();
    auto bootstrapIds = dht.getBootstrapIds();

    for (auto& bucket : getBuckets()) {
        std::list<Sp<KBucketEntry>> entries = bucket->getEntries();
        auto wasFull = entries.size() >= Constants::MAX_ENTRIES_PER_BUCKET;
        for (auto& entry : entries) {
            // remove really old entries, ourselves and bootstrap nodes if the bucket is full
            if (entry->getId() == localId || (wasFull && vector_contains(bootstrapIds, entry->getId()))) {
                bucket->_removeIfBad(entry, true);
                continue;
            }

            // Fix the wrong entries
            if (!bucket->getPrefix().isPrefixOf(entry->getId())) {
                bucket->_removeIfBad(entry, true);
                put(entry);
            }
        }

        bool refreshNeeded = bucket->needsToBeRefreshed();
        if (refreshNeeded) {
            auto name =  "Refreshing Bucket - " + bucket->getPrefix().toString();
            tryPingMaintenance(bucket, {PingRefreshTask::Options::probeCache}, name);
        }
    }
}

void RoutingTable::tryPingMaintenance(Sp<KBucket> bucket, const std::vector<PingRefreshTask::Options>& options, const std::string& name) {
    assert(bucket);
    assert(!name.empty());

    if (maintenanceTasks.count(bucket) > 0)
        return;

    auto task = std::make_shared<PingRefreshTask>(&dht, bucket, options);
    task->setName(name);
    task->addListener([=](Task* t) {
        maintenanceTasks.erase(bucket);
    });

    maintenanceTasks[bucket] = task;
    dht.getTaskManager().add(task);
}

void RoutingTable::fillBuckets() {
    for (auto& bucket: getBuckets()) {
        int num = bucket->size();

        // just try to fill partially populated buckets
        // not empty ones, they may arise as artifacts from deep splitting
        if (num < Constants::MAX_ENTRIES_PER_BUCKET) {
            bucket->updateRefreshTimer();

            auto completeHandler = ([](Sp<NodeInfo>) {});
            auto task = dht.findNode(bucket->getPrefix().createRandomId(), completeHandler);
            task->setName("Filling Bucket - " + bucket->getPrefix().toString());
        }
    }
}

void RoutingTable::load(const std::string& path) {
    assert(!path.empty());

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return;

    std::vector<uint8_t> data{};
    if (!file.bad()) {
        auto length = file.rdbuf()->pubseekoff(0, std::ios_base::end);
        if (length == 0) {
            file.close();
            return;
        }

        data.resize(length);
        file.rdbuf()->pubseekoff(0, std::ios_base::beg);
        file.read(reinterpret_cast<char*>(data.data()), length);
    }

    try {
        nlohmann::json root = nlohmann::json::from_cbor(data);

        long timestamp = root.at("timestamp").get<long>();
        auto nodes = root.at("entries");
        for (auto &node : nodes)
           _put(KBucketEntry::fromJson(node));

        log->info("Loaded {} entries from persistent file. it was {} min old.",
            nodes.size(), (currentTimeMillis() - timestamp) / (60 * 1000));
    } catch (const std::exception& e) {
        log->error("read file '{}' error: {}", path, e.what());
    }

    file.close();
}

void RoutingTable::save(const std::string& path) {
    assert(!path.empty());

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
        return;

    if (getNumBucketEntries() == 0) {
        log->trace("Skip to save the empty routing table.");
        return;
    }

    nlohmann::json entries = nlohmann::json::array();
    for (auto& bucket : getBuckets()) {
        for (auto& entry : bucket->getEntries()) {
            entries.push_back(entry->toJson());
        }
    }

    nlohmann::json root = nlohmann::json::object();
    root["timestamp"] = currentTimeMillis();
    root["entries"] = entries;

    auto data = nlohmann::json::to_cbor(root);
    file.write(reinterpret_cast<char*>(data.data()), data.size());
    file.close();
}

bool RoutingTable::isHomeBucket(const Prefix& prefix) const {
    return prefix.isPrefixOf(dht.getNode().getId());
}

std::string RoutingTable::toString() const {
    std::string str {};

    auto buckets = getBuckets();
    str.append("buckets: ")
        .append(std::to_string(buckets.size()))
        .append(" / entries: ")
        .append(std::to_string(getNumBucketEntries()))
        .append(1, '\n');

    for (auto& bucket : buckets) {
        str.append(bucket->toString()).append(1, '\n');
    }
    return str;
}

}
}
