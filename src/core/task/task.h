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

#include <list>

#include "utils/time.h"

#include "constants.h"
#include "rpccall.h"
#include "carrier/log.h"

namespace elastos {
namespace carrier {

class DHT;
class Task;

using TaskListener = std::function<void (Task *)>;

class Task {
friend class CallListener;
public:
    enum class State {
        INITIAL,
        QUEUED,
        RUNNING,
        FINISHED,
        CANCELED
    };

    Task(DHT* _dht, const std::string& taskName): dht(*_dht) {
        taskId = nextTaskId++;
        log = Logger::get(taskName);
    }

    ~Task() {
        clearInFlight();
    }

    int getTaskId() const {
        return taskId;
    }

    void setName(std::string name) {
        this->name = name;
    }

    const std::string& getName() const {
        return name;
    }

    bool setState(State expected, State newState) {
        std::vector<State> expects {expected};
        return setState(std::move(expects), newState);
    }

    bool setState(std::vector<State> expected, State newState);
    State getState() const {
        return state;
    }

    void setNestedTask(std::shared_ptr<Task> nested) {
        this->nested = nested;
    }
    std::shared_ptr<Task> getNestedTask() const {
        return nested;
    }

    DHT& getDHT() const {
        return dht;
    }

    void addListener(TaskListener listener);
    void removeListener(TaskListener listener);

    void start();
    void cancel();

    bool isCanceled() const {
        return state == State::CANCELED;
    }
    bool isFinished() const {
        return isTerminal();
    }

    uint64_t getStartTime() const {
        return startTime;
    }

    uint64_t getFinishedTime() const {
        return finishTime;
    }

    uint64_t age() const {
        return currentTimeMillis() - startTime;
    }

    int compareTo(Task &t) const {
        return taskId - t.taskId;
    }

    operator std::string() const;

protected:
    bool canDoRequest() const {
        return inFlight.size() < Constants::MAX_CONCURRENT_TASK_REQUESTS;
    }

    bool sendCall(Sp<NodeInfo> node, Sp<Message> request, std::function<void(Sp<RPCCall>&)> modifyCallBeforeSubmit);

    virtual void callSent(RPCCall* call) {}
    virtual void callResponsed(RPCCall* call, Sp<Message> response) {}
    virtual void callError(RPCCall* call) {}
    virtual void callTimeout(RPCCall* call) {}

    virtual void prepare() {}
    virtual void update() {}

    virtual bool isDone() const {
        return inFlight.empty() || isFinished();
    }

    virtual std::string className() const {
        return "Task";
    }

    Sp<Logger> log;
    DHT& dht;

private:
    bool isTerminal() const {
        return state == State::FINISHED || state == State::CANCELED;
    }

    void serializedUpdate();
    void finish();
    void notifyCompletionListeners();
    void clearInFlight();

    friend class TaskManager;

    int taskId {};
    std::string name {};
    State state { State::INITIAL };
    std::shared_ptr<Task> nested {};

    uint64_t startTime {};
    uint64_t finishTime {};

    std::map<std::size_t, Sp<RPCCall>> inFlight {};
    std::list<TaskListener> listeners {};

    int lock {0};

    static std::atomic<int> nextTaskId;
    static std::vector<RPCCall::State> callStatesTobeUpdate;

    std::vector<std::string> stateName = {"INITIAL", "QUEUED", "RUNNING", "FINISHED", "CANCELED"};
};

} // namespace carrier
} // namespace elastos

