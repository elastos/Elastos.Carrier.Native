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

#include "utils/time.h"
#include "utils/list.h"
#include "task/peer_lookup.h"
#include "task/peer_announce.h"
#include "task/value_announce.h"
#include "task/task.h"
#include "task/candidate_node.h"

#include "messages/message.h"
#include "dht.h"

namespace elastos {
namespace carrier {

std::atomic<int> Task::nextTaskId(0);
std::vector<RPCCall::State> Task::callStatesTobeUpdate = {
    RPCCall::State::RESPONDED,
    RPCCall::State::ERROR,
    RPCCall::State::STALLED,
    RPCCall::State::TIMEOUT
};

bool Task::setState(std::vector<State> expected, State newState) {
    auto it = std::find_if(expected.begin(), expected.end(), [&](const State &s) {
        return s == this->state;
    });
    if(it == expected.end())
        return false;

    this->state = newState;
    return true;
}

void Task::addListener(TaskListener listener) {
    // listener is added after the task already terminated, thus it won't get the
    // event, trigger it manually
    if (isTerminal())
        listener(this);

    listeners.push_back(listener);
}

void Task::removeListener(TaskListener listener) {
    // TODO:
}

void Task::start() {
    if (setState({State::INITIAL, State::QUEUED}, State::RUNNING)) {
        log->debug("Task starting: {}", static_cast<std::string>(*this));
        this->startTime = currentTimeMillis();

        prepare();

        serializedUpdate();
    }
}

void Task::cancel() {
    if (setState({State::INITIAL, State::QUEUED, State::RUNNING}, State::CANCELED)) {
        this->finishTime = currentTimeMillis();
        log->debug("Task canceled: {}", static_cast<std::string>(*this));

        notifyCompletionListeners();
    }

    if (!!nested)
        nested->cancel();
}

void Task::finish() {
    if (setState({State::INITIAL, State::QUEUED, State::RUNNING}, State::FINISHED)) {
        this->finishTime = currentTimeMillis();
        log->debug("Task finished: {}", static_cast<std::string>(*this));

        notifyCompletionListeners();
    }
}

void Task::notifyCompletionListeners() {
    for (auto& listener : listeners) {
        listener(this);
    }
}

void Task::clearInFlight() {
    if (inFlight.empty())
        return;

    for (auto& [key, call] : inFlight) {
        call->addStateChangeHandler([](RPCCall*, RPCCall::State, RPCCall::State) {});
    }
    inFlight.clear();
}

// TODO: CHECK ME!!!
void Task::serializedUpdate() {
    int current = ++lock;

    // another thread is executing
    if(current > 1)
        return;

    log->trace("Task update: {}", static_cast<std::string>(*this));
    do {
        if(isDone())
            finish();

        if (canDoRequest() && !isFinished()) {
            update();
            // check again in case todo-queue has been drained by update()
            if(isDone())
                finish();
        }

        //current = lock.addAndGet(Math.negateExact(current));
        lock -= current;
        current = lock;
    } while(current > 0);

    if (isFinished()) {
        dht.getTaskManager().removeTask(this);
    }
}

// TODO: CHECK ME!!!
bool Task::sendCall(Sp<NodeInfo> node, Sp<Message> request, std::function<void(Sp<RPCCall>&)> modifyCallBeforeSubmit) {
    if (!canDoRequest())
        return false;

    auto removeCall = [](std::map<std::size_t, Sp<RPCCall>>& maps, RPCCall* call) {
        auto it = maps.find(call->hash());
        if (it != maps.end())
            maps.erase(it);
    };

    auto call = std::make_shared<RPCCall>(dht, node, request);
#if defined(MSG_PRINT_DETAIL)
    call->setName(name);
#endif
    call->addStateChangeHandler([&](RPCCall* c, RPCCall::State previous, RPCCall::State current) {
        switch (current) {
        case RPCCall::State::SENT:
            callSent(c);
            break;

        case RPCCall::State::RESPONDED:
            removeCall(inFlight, c);
            if (!isFinished()) {
                callResponsed(c, c->getResponse());
            }
            break;

        case RPCCall::State::ERROR:
            removeCall(inFlight, c);
            if (!isFinished())
                callError(c);
            break;

        case RPCCall::State::TIMEOUT:
            removeCall(inFlight, c);
            if (!isFinished())
                callTimeout(c);
            break;

        default:
            break;
        }

        if (vector_contains(callStatesTobeUpdate, current))
            serializedUpdate();
    });

    modifyCallBeforeSubmit(call);
    inFlight[call->hash()] = call;

    log->debug("Task#{} sending call to {}", getTaskId(), static_cast<std::string>(*node), request->getRemoteAddress().toString());
    // asyncify since we're under a lock here
    dht.getServer().sendCall(call);
    return true;
}

Task::operator std::string() const {
    std::stringstream ss;
    ss.str().reserve(1024);

    ss << className() << "#" << getTaskId();
    if (!name.empty())
        ss << "[" << name << "]";

    ss << " DHT: " << dht.getTypeName()
        << ", state: " << stateName[(int)state];

    return ss.str();
}

} // namespace carrier
} // namespace elastos
