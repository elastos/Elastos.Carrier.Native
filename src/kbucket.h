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
#include <memory>

#include "carrier/prefix.h"
#include "carrier/log.h"
#include "carrier/types.h"
#include "utils/random_generator.h"
#include "constants.h"
#include "kbucket_entry.h"

namespace elastos {
namespace carrier {

class Message;

/**
 * A KBucket is just a list of KBucketEntry objects.
 *
 * The list is sorted by time last seen : The first element is the least
 * recently seen, the last the most recently seen.
 *
 * This is a lock-free k-bucket implementation.
 *
 * CAUTION:
 *   All methods name leading with _ means that method will WRITE the
 *   list, it can only be called inside the routing table's
 *   pipeline processing.
 *
 *   Due the heavy implementation the stream operations are significant
 *   slow than the for-loops. so we should avoid the stream operations
 *   on the KBucket entries and the cache entries, use for-loop instead.
 */
class KBucket {
public:
    KBucket(const Prefix& _prefix, bool isHome): prefix(_prefix), homeBucket(isHome) {
        log = Logger::get("KBucket");
    }

    KBucket(const Prefix& val): KBucket(val, false) {}

    const Prefix& getPrefix() const noexcept {
        return prefix;
    }

    bool isHomeBucket() const noexcept {
        return homeBucket;
    }

    const std::list<Sp<KBucketEntry>>& getEntries() const noexcept {
        return entries;
    }

    int size() const noexcept {
        return getEntries().size();
    }

    bool isFull() const noexcept {
        return getEntries().size() >= Constants::MAX_ENTRIES_PER_BUCKET;
    }

    Sp<KBucketEntry> random() {
        const auto& entriesRef = getEntries();
        if (entriesRef.empty())
            return nullptr;

        auto it = std::next(entriesRef.begin(), RandomGenerator<int>(0, entriesRef.size())());
        return *it;
    }

    Sp<KBucketEntry> get(const Id& id) const noexcept {
        return findAny([&](Sp<KBucketEntry>& entry) {
            return entry->getId() == id;
        });
    }

    Sp<KBucketEntry> find(const Id& id, const SocketAddress& addr) const noexcept {
        return findAny([&](Sp<KBucketEntry>& entry) {
            return entry->getId() == id || entry->getAddress() == addr;
        });
    }

    bool exists(const Id& id) const noexcept {
        return anyMatch([&](Sp<KBucketEntry>& entry) {
            return entry->getId() == id;
        });
    }

    bool needsToBeRefreshed() const {
        uint64_t now = currentTimeMillis();
        return now - lastRefresh > Constants::BUCKET_REFRESH_INTERVAL
            && anyMatch([](Sp<KBucketEntry>& entry) {
                return entry->needsPing();
            });
    }

    bool needsReplacement() {
        return anyMatch([](Sp<KBucketEntry>& entry) {
            return entry->needsReplacement();
        });
    }

    void updateRefreshTimer() noexcept {
        lastRefresh = currentTimeMillis();
    }

    int compareTo(Sp<KBucket> bucket) const {
        return prefix.compareTo(bucket->getPrefix());
    }

    operator std::string() const;

//protected:
    void _put(Sp<KBucketEntry> newEntry);
    void _removeIfBad(Sp<KBucketEntry> toRemove, bool force);

    void _update(Sp<KBucketEntry> toRefresh);
    void _onTimeout(const Id& id);
    void _onSend(const Id&);

private:
    bool _replaceBadEntry(Sp<KBucketEntry> newEntry);
    void _update(Sp<KBucketEntry> toRemove, Sp<KBucketEntry> toInsert);
    void _notifyOfResponse(Sp<Message>&);

    Sp<KBucketEntry> findAny(std::function<bool(Sp<KBucketEntry>&)> predicate) const {
        for (auto entry: getEntries()) {
            if (predicate(entry))
                return entry;
        }
        return nullptr;
    }
    inline bool anyMatch(std::function<bool(Sp<KBucketEntry>&)> predicate) const {
        return findAny(std::move(predicate)) != nullptr;
    }

    void setEntries(const std::list<Sp<KBucketEntry>>& entries) noexcept {
        this->entries = entries;
    }

    const Prefix prefix;
    bool homeBucket { false };

    std::list<Sp<KBucketEntry>> entries {};
    uint64_t lastRefresh {0};

    Sp<Logger> log;
};

} // namespace carrier
} // namespace elastos
