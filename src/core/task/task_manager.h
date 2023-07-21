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

#include <memory>
#include <list>
#include <atomic>

#include "utils/log.h"
#include "constants.h"

namespace elastos {
namespace carrier {

class DHT;
class Task;

class TaskManager {
public:
    TaskManager(): canceling(false) {
        log = Logger::get("TaskManager");
    }

    void add(Sp<Task> task, bool prior);

    inline void add(Sp<Task> task) {
        add(task, false);
    }

    void dequeue();

    inline bool canStartTask() {
        return !canceling && (running.size() <= Constants::MAX_ACTIVE_TASKS);
    }

    void cancelAll();
    void removeTask(Task* t);

private:
    std::list<Sp<Task>> queued {};
    std::list<Sp<Task>> running {};
    bool canceling {false};

    Sp<Logger> log;

    mutable std::mutex taskman_mtx {};
};

} // namespace carrier
} // namespace elastos
