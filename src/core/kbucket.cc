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

#include <iostream>

#include "utils/list.h"
#include "kbucket.h"
#include "messages/message.h"

namespace elastos {
namespace carrier {

void KBucket::_put(Sp<KBucketEntry> newEntry) {
    if (!newEntry)
        return;

    // find existing
    const auto& entriesRef = getEntries();
    for (auto& entry: entriesRef) {
        if (entry->equals(*newEntry)) {
            entry->merge(newEntry);
            return;
        }

        // Node id and address conflict
        // Log the conflict and keep the existing entry
        if (entry->matches(*newEntry)) {
            log->info("New node {} claims same ID or IP as  {}, might be impersonation attack or IP change. " \
                      "ignoring until old entry times out", newEntry->toString(), entry->toString());
            return;
        }
    }

    if (newEntry->isReachable()) {
        if (entriesRef.size() < Constants::MAX_ENTRIES_PER_BUCKET) {
            // insert to the list if it still has room
            _update(nullptr, newEntry);
            return;
        }

        // Try to replace the bad entry
        if (_replaceBadEntry(newEntry))
            return;

        // try to check the youngest entry
        auto youngest = list_get(entriesRef, entriesRef.size() - 1);

        // older entries displace younger ones (although that kind of stuff should
        // probably go through #update directly)
        if (youngest->getCreationTime() > newEntry->getCreationTime()) {
            // Replace the youngest entry
            _update(youngest, newEntry);
            return;
        }
    }
}

bool KBucket::_replaceBadEntry(Sp<KBucketEntry> newEntry) {
    assert(newEntry);

    for (auto& entry: getEntries()) {
        if (entry->needsReplacement()) {
            // bad one to get rid of
            _update(entry, newEntry);
            return true;
        }
    }
    return false;
}

void KBucket::_removeIfBad(Sp<KBucketEntry> toRemove, bool force) {
    assert(toRemove);

    if ((force || toRemove->needsReplacement()) && anyMatch([&](Sp<KBucketEntry>& entry) {
        return entry->getId() == toRemove->getId();
    })) {
        _update(toRemove, nullptr);
    }
}

void KBucket::_update(Sp<KBucketEntry> toRefresh) {
    assert(toRefresh);

    for (auto& entry: getEntries()) {
        if (entry->equals(*toRefresh)) {
            entry->merge(toRefresh);
            return;
        }
    }
}

void KBucket::_update(Sp<KBucketEntry> toRemove, Sp<KBucketEntry> toInsert) {
    if (toInsert != nullptr && anyMatch([&](Sp<KBucketEntry>& entry) {
        return toInsert->matches(*entry);
    })) {
        return;
    }

    std::list<Sp<KBucketEntry>> newEntries { getEntries() };
    bool removed { false };
    bool added { false };

    // removal never violates ordering constraint, no checks required
    if (toRemove != nullptr) {
        newEntries.remove(toRemove);
        removed = true;
    }

    if (toInsert != nullptr) {
        int oldSize = newEntries.size();
        bool wasFull = oldSize >= Constants::MAX_ENTRIES_PER_BUCKET;
        auto youngest = oldSize > 0 ? list_get(newEntries, oldSize - 1) : nullptr;
        bool unorderedInsert = youngest != nullptr && toInsert->getCreationTime() < youngest->getCreationTime();

        added = !wasFull || unorderedInsert;
        if (added) {
            newEntries.push_back(toInsert);
        }

        if (unorderedInsert) {
            auto sortKBucketEntry = [](Sp<KBucketEntry> a, Sp<KBucketEntry> b) -> bool {
                return a->getCreationTime() < b->getCreationTime();
            };
            newEntries.sort(sortKBucketEntry);
        }

    }

    // make changes visible
    if (added || removed) {
        setEntries(newEntries);
    }
}

void KBucket::_notifyOfResponse(Sp<Message>& msg) {
    if (msg->getType() != Message::Type::RESPONSE || !msg->getAssociatedCall())
        return;

    for (auto& entry: getEntries()) {
        if (entry->getId() == msg->getId()) {
            entry->signalResponse();
            return;
        }
    }
}

void KBucket::_onTimeout(const Id& id) {
    for (auto& entry: getEntries()) {
        if (entry->getId() == id) {
            entry->signalRequestTimeout();

            // NOTICE: Test only - merge buckets
            //   remove when the entry needs replacement
            // _removeIfBad(entry, false);

            // NOTICE: Product
            //   only removes the entry if it is bad
            _removeIfBad(entry, false);
            return;
        }
    }
}

void KBucket::_onSend(const Id& id) {
    for (auto& entry: getEntries()) {
        if (entry->getId() == id) {
            entry->signalRequest();
            return;
        }
    }
}

std::string KBucket::toString() const {
    std::stringstream ss;
    ss.str().reserve(1024);

    ss << "Prefix: " << prefix.toString();
    if (isHomeBucket())
        ss << " [Home]";
    ss << "\n";

    const auto& entriesRef = getEntries();
    if (!entriesRef.empty()) {
        ss << "  entries[" << std::to_string(entriesRef.size()) << "]:\n";
        for(const auto& entry: entriesRef)
            ss << "    " << entry->toString() << "\n";
    }
    return ss.str();
}

}
}
