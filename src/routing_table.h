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
#include <atomic>
#include <functional>
#include <memory>
#include <map>

#include "carrier/id.h"
#include "carrier/node_info.h"
#include "carrier/log.h"
#include "carrier/types.h"

#include "utils/random_generator.h"
#include "utils/mtqueue.h"
#include "task/ping_refresh_task.h"
#include "kbucket.h"

namespace elastos {
namespace carrier {

class DHT;
class KBucket;
class Task;
class Operation;

class RoutingTable {
public:
    RoutingTable(DHT& dht): dht(dht) {
        buckets.emplace_back(std::make_shared<KBucket>(Prefix {}, true));
        log = Logger::get("RoutingTable");
    }

    const std::list<Sp<KBucket>>& getBuckets() const noexcept {
        return buckets;
    }

    void setBuckets(const std::list<Sp<KBucket>>& buckets) noexcept {
        this->buckets = buckets;
    }

    const DHT& getDHT() const noexcept {
        return dht;
    }

    int size() const noexcept {
        return getBuckets().size();
    }

    const Sp<KBucket> getBucket(int index) const noexcept {
        return list_get(getBuckets(), index);
    }
    const Sp<KBucket> getBucket(const Id& id) const noexcept {
        return list_get(getBuckets(), indexOf(getBuckets(), id));
    }

    const Sp<KBucketEntry> getEntry(const Id& id) const noexcept {
        return getBucket(id)->get(id);
    }

    static int indexOf(const std::list<Sp<KBucket>>& bucketsRef, const Id& id);

    int getNumBucketEntries() const noexcept {
        int num {0};
        for (const auto& bucket: getBuckets()) {
            num += bucket->size();
        }
        return num;
    }

    Sp<KBucketEntry> getRandomEntry() const {
        const auto& bucketsRef = getBuckets();
        auto it = std::next(bucketsRef.begin(), RandomGenerator<int>(1, bucketsRef.size())());
        return (it != bucketsRef.end()) ? (*it)->random(): nullptr;
    }

    bool isHomeBucket(const Prefix& prefix) const;

    void _refreshOnly(Sp<KBucketEntry> toRefresh) {
        getBucket(toRefresh->getId())->_update(toRefresh);
    }

    void put(const Sp<KBucketEntry>& entry) {
        _put(entry);
    }

    void remove(const Id& id) {
        _remove(id);
    }

    void onSend(const Id& id) {
        _onSend(id);
    }

    void onTimeout(const Id& id) {
        _onTimeout(id);
    }

    void maintenance() {
        _maintenance();
    }

    void fillBuckets();

    void load(const std::string&);
    void save(const std::string&);

    void tryPingMaintenance(Sp<KBucket> bucket, const std::vector<PingRefreshTask::Options>& options, const std::string& name);

private:
    void _put(const Sp<KBucketEntry>& entry);
    void _remove(const Id& id);
    void _onTimeout(const Id& id);
    void _onSend(const Id& id);

    bool _needsSplit(const Sp<KBucket>& bucket, const Sp<KBucketEntry>& newEntry);
    void _modify(const std::vector<Sp<KBucket>>& toRemove, const std::vector<Sp<KBucket>>& toAdd);
    void _split(const Sp<KBucket>& bucket);
    void _mergeBuckets();

    /**
     * Check if a buckets needs to be refreshed, and refresh if necessary.
     */
    void _maintenance();

    DHT& dht;
    std::list<Sp<KBucket>> buckets {};

    long timeOfLastPingCheck {0};

    std::atomic_bool writeLock {false};
    std::map<Sp<KBucket>, Sp<Task>> maintenanceTasks{};

    Sp<Logger> log;
};

} // namespace carrier
} // namespace elastos
