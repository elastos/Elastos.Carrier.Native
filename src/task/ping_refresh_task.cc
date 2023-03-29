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

#include "dht.h"
#include "task.h"
#include "kbucket.h"
#include "kbucket_entry.h"
#include "routing_table.h"
#include "messages/ping_request.h"
#include "ping_refresh_task.h"

namespace elastos {
namespace carrier {

void PingRefreshTask::addBucket(Sp<KBucket> bucket) {
    this->bucket = bucket;
    this->bucket->updateRefreshTimer();

    for (const auto& entry : this->bucket->getEntries()) {
        if (entry->needsPing() || checkAll || removeOnTimeout)
            todo.emplace_back(entry);
    }
}

void PingRefreshTask::callTimeout(RPCCall* call) {
    if (!removeOnTimeout)
        return;

    // CAUSION:
    // Should not use the original bucket object,
    // because the routing table is dynamic, maybe already changed.
    const Id& nodeId = call->getTargetId();
    log->debug("Removing invalid entry from cache.");
    getDHT().getRoutingTable().remove(nodeId);
}

void PingRefreshTask::update() {
    while (!todo.empty() && canDoRequest()) {
        auto candidateNode = todo.front();

        if (!checkAll && !candidateNode->needsPing()) {
            // Entry already updated during the task running
            todo.pop_front();
            continue;
        }

        auto request = std::make_shared<PingRequest>();
        try {
            sendCall(candidateNode, request, [&](Sp<RPCCall>&) {
                todo.pop_front();
            });
        } catch (const std::exception& e) {
            log->error("Error on sending 'pingRequest' request: " + std::string(e.what()));
        }
    }
}

}
}
