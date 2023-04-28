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

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "utils/time.h"

#include "rpccall.h"
#include "rpcserver.h"
#include "constants.h"
#include "kbucket_entry.h"
#include "task/candidate_node.h"

namespace elastos {
namespace carrier {

RPCCall::RPCCall(DHT& _dht, Sp<NodeInfo> _target, Sp<Message> _request)
    : dht(_dht), target(_target), request(_request),
    stateChangeHandler([](RPCCall*, State, State){}),
    responseHandler([](RPCCall*, Sp<Message>&){}),
    stallHandler([](RPCCall*){}),
    timeoutHandler([](RPCCall*){}) {

    request->setRemote(target->getId(), target->getAddress());

    if (auto kbEntry = std::dynamic_pointer_cast<KBucketEntry>(_target)) {
        sourceWasKnownReachable = kbEntry->isReachable();
    } else if (auto candidateNode = std::dynamic_pointer_cast<CandidateNode>(_target)) {
        sourceWasKnownReachable = candidateNode->isReachable();
    } else {
        sourceWasKnownReachable = false;
    }
}

void RPCCall::updateState(State currentState) {
    auto prevState {this->state};
    this->state = currentState;

    stateChangeHandler(this, prevState, currentState);

    switch (currentState) {
    case State::TIMEOUT:
        timeoutHandler(this);
        break;
    case State::STALLED:
        stallHandler(this);
        break;
    case State::RESPONDED:
        responseHandler(this, response);
        break;
    default:
        break;
    }
}

void RPCCall::sent(RPCServer* server) {
    sentTime = currentTimeMillis();
    updateState(State::SENT);

    scheduler = std::ref(server->getScheduler());
    // spread out the stalls by +- 1ms to reduce lock contention
    // TODO:: Improve the time
    // int smear = ThreadLocalRandom.current().nextInt(-1000, 1000);
    // timeoutTimer = scheduler.schedule(this::checkTimeout,
    //         expectedRTT * 1000 + smear, TimeUnit.MICROSECONDS);
    timeoutTimer = scheduler->get().add(std::bind(&RPCCall::checkTimeout, this), 2000);
}

void RPCCall::responsed(Sp<Message> response) {
    assert(response != nullptr);
    assert(response->getType() == Message::Type::RESPONSE ||
           response->getType() == Message::Type::ERR);

    if (timeoutTimer != nullptr)
        timeoutTimer->cancel();

    this->response = response;
    this->responseTime = currentTimeMillis();

    switch(response->getType()) {
    case Message::Type::RESPONSE:
        updateState(State::RESPONDED);
        break;
    case Message::Type::ERR:
        updateState(State::ERR);
        break;
    default:
        throw std::runtime_error("Unexpected message type received.");
        break;
    }
}

void RPCCall::checkTimeout() {
    if (state != State::SENT && state != State::STALLED)
        return;

    int elapsed = currentTimeMillis() - sentTime;
    int remaining = Constants::RPC_CALL_TIMEOUT_MAX - elapsed;

    if (remaining > 0) {
        updateState(State::STALLED);
        // re-schedule for failed
        //TODO:: need check the time is right
        if (scheduler != std::nullopt)
            timeoutTimer = scheduler->get().add(std::bind(&RPCCall::checkTimeout, this), remaining);
    } else {
        updateState(State::TIMEOUT);
    }
}

} // namespace carrier
} // namespace elastos

