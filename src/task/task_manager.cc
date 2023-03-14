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

#include "utils/time.h"
#include "task.h"
#include "task_manager.h"

namespace elastos {
namespace carrier {

void TaskManager::add(Sp<Task> task, bool prior) {
    if (canceling)
        return;

    std::unique_lock<std::mutex> lk(taskman_mtx);

    if (task->getState() == Task::State::RUNNING) {
        running.emplace_back(task);
        return;
    }

    if (!task->setState(Task::State::INITIAL, Task::State::QUEUED))
        return;

    if (prior)
        queued.emplace_front(task);
    else
        queued.emplace_back(task);
}

void TaskManager::dequeue() {
    std::unique_lock<std::mutex> lk(taskman_mtx);

    while (true) {
        if (!canStartTask() || queued.empty())
            break;

        auto task = queued.front();
        queued.pop_front();

        if (task->isFinished())
            continue;

        running.emplace_back(task);

        task->start();
    }
}

void TaskManager::cancelAll() {
    std::unique_lock<std::mutex> lk(taskman_mtx);
    canceling = true;

    auto it = running.begin();
    while (it != running.end()) {
        auto task = *it;
        it++;
        task->cancel();
    }

    it = queued.begin();
    while (it != queued.end()) {
        auto task = *it;
        it++;
        task->cancel();
    }

    canceling = false;
}

void TaskManager::removeTask(Task* t) {
    running.remove_if([t](Sp<Task> task){ return task.get() == t; });
    queued.remove_if([t](Sp<Task> task){ return task.get() == t; });
}

}
}
