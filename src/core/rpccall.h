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

#pragma once

#include <optional>
#include <functional>
#include "carrier/node_info.h"
#include "messages/message.h"
#include "scheduler.h"

namespace elastos {
namespace carrier {

class RPCServer;
class DHT;

class RPCCall {
public:
    enum class State {
        UNSENT,
        SENT,
        STALLED,
        TIMEOUT,
        CANCELED,
        ERROR,
        RESPONDED
    };

    using StateChangeHandler = std::function<void(RPCCall*, State, State)>;
    using ResponseHandler = std::function<void(RPCCall*, Sp<Message>&)>;
    using StallHandler = std::function<void(RPCCall*)>;
    using TimeoutHandler = std::function<void(RPCCall*)>;

    RPCCall(DHT& dht, Sp<NodeInfo> _target, Sp<Message> _request);
    RPCCall(DHT* dht, Sp<NodeInfo> _target, Sp<Message> _request): RPCCall(*dht, _target, _request) {}

    DHT& getDHT() const noexcept {
        return dht;
    }

    const Id& getTargetId() const noexcept {
        return target->getId();
    }

    const Sp<NodeInfo>& getTarget() const noexcept {
        return target;
    }

    bool matchesId() const noexcept {
        return response->getId() == target->getId();
    }

    bool matchesAddress() const noexcept {
        return response->getOrigin() == request->getRemoteAddress();
    }

    const Sp<Message>& getRequest() const noexcept {
        return request;
    }

    const Sp<Message>& getResponse() const noexcept {
        return response;
    }

    long getSentTime() const noexcept {
        return sentTime;
    }

    long getResponseTime() const noexcept {
        return responseTime;
    }

    State getState() const noexcept {
        return state;
    }

    bool isPending() const noexcept {
        // TODO: return state.ordinal() < State::TIMEOUT.ordinal();
        return false;
    }

    void addStateChangeHandler(StateChangeHandler handler) noexcept {
        this->stateChangeHandler = handler;
    }

    void addResponseHandler(ResponseHandler handler) noexcept {
        this->responseHandler = handler;
    }

    void addStallHandler(StallHandler handler) noexcept {
        this->stallHandler = handler;
    }

    void addTimeoutHandler(TimeoutHandler handler) noexcept {
        this->timeoutHandler = handler;
    }

    void updateState(State state);
    void sent(RPCServer* server);
    void responsed(Sp<Message> response);

    void responseSocketMismatch() {
        // TODO: responseSocketMismatch = true;
    }

    void failed() {
        updateState(State::TIMEOUT);
    }

    void cancel() {
        if (timeoutTimer != nullptr)
            timeoutTimer->cancel();

        updateState(State::CANCELED);
    }

    void stall() {
        if (state == State::SENT)
            updateState(State::STALLED);
    }

    void checkTimeout();

    size_t hash() {
        if (hashValue == 0)
            hashValue = std::hash<RPCCall*>()(this);

        return hashValue;
    }

#if defined(MSG_PRINT_DETAIL)
    void setName(const std::string& name) {
        this->name = name;
    }
    const std::string& getName() const {
        return name;
    }
#endif

private:
    DHT& dht;
    Sp<NodeInfo> target;

    Sp<Message> request {};
    Sp<Message> response {};

    bool sourceWasKnownReachable {false};

    uint64_t sentTime = std::numeric_limits<uint64_t>::max();
    uint64_t responseTime = std::numeric_limits<uint64_t>::max();

    State state {State::UNSENT};

    StateChangeHandler stateChangeHandler;
    ResponseHandler responseHandler;
    StallHandler stallHandler;
    TimeoutHandler timeoutHandler;

    std::optional<std::reference_wrapper<Scheduler>> scheduler {std::nullopt};
    Sp<Scheduler::Job> timeoutTimer {};

    size_t hashValue {0};

#if defined(MSG_PRINT_DETAIL)
    std::string name {};
#endif
};
}
}
