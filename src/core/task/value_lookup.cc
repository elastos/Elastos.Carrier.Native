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

#include "carrier/value.h"
#include "messages/find_value_request.h"
#include "messages/find_value_response.h"
#include "utils/log.h"
#include "kclosest_nodes.h"
#include "value_lookup.h"

namespace elastos {
namespace carrier {

void ValueLookup::prepare() {
    const int MAX_ENTRIES =  Constants::MAX_ENTRIES_PER_BUCKET * 2;
    auto kClosetNodes = std::make_shared<KClosestNodes>(getDHT(), getTarget(), MAX_ENTRIES);
    kClosetNodes->fill();
    auto nodes = kClosetNodes->asNodeList();
    addCandidates(nodes);
}

void ValueLookup::update() {
    while (canDoRequest()) {
        auto candidate = getNextCandidate();
        if (!candidate)
            return;

        auto request = std::make_shared<FindValueRequest>(getTarget());
        request->setWant4(getDHT().getType() == DHT::Type::IPV4);
        request->setWant6(getDHT().getType() == DHT::Type::IPV6);

        if (expectedSequence != -1)
            request->setSequenceNumber(expectedSequence);

        try {
            sendCall(candidate, request, [&](Sp<RPCCall> call) {
                candidate->setSent();
            });
        } catch (const std::exception& e) {
            log->error("Error on sending 'findValue' request: " + std::string(e.what()));
        }
    }
}

void ValueLookup::callResponsed(RPCCall* call, Sp<Message> message) {
    if ((!call->matchesId() ||
        message->getType() != Message::Type::RESPONSE ||
        message->getMethod() != Message::Method::FIND_VALUE)) {
        return;
    }

    LookupTask::callResponsed(call, message);

    auto response = std::dynamic_pointer_cast<FindValueResponse>(message);
    if (!response->getData().empty()) {
        auto value = response->getValue();
        auto id = value->getId();
        if (!(id == getTarget())) {
            log->warn("Responsed value id {} mismatched with expected {}", static_cast<std::string>(id), static_cast<std::string>(getTarget()));
            return;
        }
        if (!value->isValid()) {
            log->warn("Responsed value {} is invalid, signature mismatch", static_cast<std::string>(id));
            return;
        }

        if (expectedSequence >= 0 && value->getSequenceNumber() < expectedSequence) {
            log->warn("Responsed value {} is outdated, sequence {}, expected {}",
                    static_cast<std::string>(id), value->getSequenceNumber(), expectedSequence);
            return;
        }

        resultHandler(value, this);
    } else {
        auto nodes = response->getNodes(getDHT().getType());
        if (!nodes.empty())
            addCandidates(nodes);
    }
}

}
}
