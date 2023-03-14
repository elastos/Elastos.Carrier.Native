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

#include "carrier/id.h"
#include "carrier/node_info.h"
#include "utils/time.h"
#include "constants.h"

namespace elastos {
namespace carrier {

/**
 * Entry in a KBucket, it basically contains an IP address of a node, the UDP
 * port of the node and a node id.
 */
class KBucketEntry: public NodeInfo {
friend class KBucket;
public:
    KBucketEntry(const Id& id, const SocketAddress& addr): NodeInfo(id, addr) {
        created = currentTimeMillis();
        lastSeen = created;
        failedRequests = 0;
    }

    KBucketEntry(const Id& id, const SocketAddress& addr, int version)
        : KBucketEntry(id, addr)  {
        this->setVersion(version);
    }

    KBucketEntry(const NodeInfo& node)
        : KBucketEntry(node.getId(), node.getAddress()) {}

    uint64_t getCreationTime() const noexcept {
        return created;
    }

    uint64_t getLastSeen() const noexcept {
        return lastSeen;
    }

    uint64_t getLastSend() const noexcept {
        return lastSend;
    }

    int getFailedRequests() const noexcept {
        return failedRequests;
    }

    bool isReachable() const noexcept {
        return reachable;
    }

    bool isNeverContacted() const noexcept {
        return lastSend == 0;
    }

    bool isEligibleForNodesList() const noexcept {
        // 1 timeout can occasionally happen. should be fine to hand it out as long as
        // we've verified it at least once
        return reachable && failedRequests < 3;
    }

    bool isEligibleForLocalLookup() const noexcept {
        // allow implicit initial ping during lookups
        // TODO: make this work now that we don't keep unverified entries in the main bucket
        return (reachable && failedRequests <= 3) || failedRequests <= 0;
    }

    long backoffWindowEnd() const;
    bool withinBackoffWindow() const {
        return withinBackoffWindow(currentTimeMillis());
    }

    bool needsPing() const noexcept {
        uint64_t now = currentTimeMillis();

        // don't ping if recently seen to allow NAT entries to time out
        // see https://arxiv.org/pdf/1605.05606v1.pdf for numbers
        // and do exponential backoff after failures to reduce traffic
        if ((now - lastSeen) < 30 * 1000 || withinBackoffWindow(now))
            return false;

        return failedRequests != 0 || (now - lastSeen) > Constants::KBUCKET_OLD_AND_STALE_TIME;
    }

    // old entries, e.g. from routing table reload
    bool oldAndStale() const noexcept {
        return failedRequests > Constants::KBUCKET_OLD_AND_STALE_TIMEOUTS &&
            currentTimeMillis() - lastSeen > Constants::KBUCKET_OLD_AND_STALE_TIME;
    }

    bool removableWithoutReplacement() noexcept {
        // some non-reachable nodes may contact us repeatedly, bumping the last seen
        // counter. they might be interesting to keep around so we can keep track of the
        // backoff interval to not waste pings on them
        // but things we haven't heard from in a while can be discarded

        bool seenSinceLastPing = lastSeen > lastSend;
        return failedRequests > Constants::KBUCKET_MAX_TIMEOUTS && !seenSinceLastPing;
    }

    void signalResponse() noexcept{
        lastSeen = currentTimeMillis();
        failedRequests = 0;
        reachable = true;
    }

    void signalRequest() noexcept {
        lastSend = currentTimeMillis();
    }

    void mergeRequestTime(uint64_t requestSent) noexcept {
        lastSend = std::max<uint64_t>(lastSend, requestSent);
    }

    /**
     * Should be called to signal that a request to this peer has timed out;
     */
    void signalRequestTimeout() noexcept {
        if (failedRequests <= 0)
            failedRequests = 1;
        else
            failedRequests++;
    }

    static Sp<KBucketEntry> fromJson(nlohmann::json& json);
    nlohmann::json toJson() const;

    std::string toString() const;

protected:
    bool needsReplacement() const noexcept {
        return (failedRequests > 1 && !reachable) ||
            (failedRequests > Constants::KBUCKET_MAX_TIMEOUTS && oldAndStale());
    }

    void merge(Sp<KBucketEntry> other);

    void setReachable(bool newValue) noexcept {
        reachable = newValue;
    }

    bool match(const KBucketEntry& other) const {
        return NodeInfo::match(other);
    }
    bool equals(const KBucketEntry& other) const {
        return NodeInfo::equals(static_cast<NodeInfo>(other));
    }

    bool operator==(const KBucketEntry& other) const {
        return equals(other);
    }

private:
    bool withinBackoffWindow(uint64_t now) const;

    uint64_t created  {0};
    uint64_t lastSeen {0};
    uint64_t lastSend {0};

    bool reachable {false};
    int failedRequests {0};
};

} // namespace carrier
} // namespace elastos
