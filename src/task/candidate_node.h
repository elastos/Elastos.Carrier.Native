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

#include "carrier/node_info.h"
#include "utils/time.h"
#include "kbucket_entry.h"

namespace elastos {
namespace carrier {

class CandidateNode: public NodeInfo {
public:
    CandidateNode(const NodeInfo& ni): NodeInfo(ni) {
        if (const auto entry = dynamic_cast<const KBucketEntry*>(&ni)) {
            reachable = entry->isReachable();
        }
    }

    void setSent() {
        this->lastSent = currentTimeMillis();
        this->pinged++;
    }

    void clearSent() {
        this->lastSent = 0;
    }

    int getPinged() const {
        return pinged;
    }

    void setReplied() {
        this->lastReply = currentTimeMillis();
    }

    void setToken(int token) {
        this->token = token;
    }

    int getToken() const {
        return token;
    }

    bool isReachable() const {
        return reachable;
    }

    bool isUnreachable() const {
        return pinged >= 3;
    }

    bool isInFlight() const {
        return lastSent != 0;
    }

    bool isEligible() const {
        // No pending request and timeout < 3 times
        return lastSent == 0 && pinged < 3;
    }

private:
    uint64_t lastSent  {0};     /* the timestamp of last unanswered request */
    uint64_t lastReply {0};     /* the timestamp of last reply */

    bool reachable {false};
    bool acked {false};        /* whether they acked our announcement */
    int  pinged {0};

    int token {0};
};

} // namespace carrier
} // namespace elastos
