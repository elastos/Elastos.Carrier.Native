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

#include <functional>
#include <map>
#include <chrono>
#include "utils/time.h"

namespace elastos {
namespace carrier {

using clock = std::chrono::steady_clock;

class Scheduler {
public:
    class Job {
    public:
        Job(std::function<void()>&& f) : do_(std::move(f)) {}

        void setFixedDelay(long _fixedDelay) {
            fixedDelay = _fixedDelay;
        }

        void cancel() {
            do_ = {};
            fixedDelay = 0;
        }

        explicit operator bool() const {
            return (bool)do_;
        }

        void operator()() const {
            do_();
        }

    private:
        std::function<void()> do_;
        long fixedDelay = 0;
        friend class Scheduler;
    };

    Sp<Scheduler::Job> add(std::function<void()>&& job_func, long delay, long fixedDelay = 0) {
        auto job = std::make_shared<Job>(std::move(job_func));
        add(job, delay, fixedDelay);
        return job;
    }

    void add(const Sp<Scheduler::Job>& job, long delay, long fixedDelay = 0) {
        job->setFixedDelay(fixedDelay);
        uint64_t time = currentTimeMillis() + delay;
        if (time != std::numeric_limits<uint64_t>::max())
            timers.emplace(time, job);
    }

    void edit(Sp<Scheduler::Job>& job, long delay, long fixedDelay = 0) {
        if (not job) {
            return;
        }
        // std::function move doesn't garantee to leave the object empty.
        // Force clearing old value.
        auto task = std::move(job->do_);
        job->cancel();
        job = add(std::move(task), delay, fixedDelay);
    }

    uint64_t run() {
        while (not timers.empty()) {
            auto timer = timers.begin();
            /*
             * Running jobs scheduled before "now" prevents run+rescheduling
             * loops before this method ends. It is garanteed by the fact that a
             * job will at least be scheduled for "now" and not before.
             */
            if (timer->first > now)
                break;

            auto job = std::move(timer->second);
            if (*job)
                (*job)();

            if (job->fixedDelay > 0) {
                edit(job, job->fixedDelay, job->fixedDelay);
            }

            timers.erase(timer);
        }
        return getNextJobTime();
    }

    inline uint64_t getNextJobTime() const {
        return timers.empty() ? std::numeric_limits<uint64_t>::max() : timers.begin()->first;
    }

    inline const uint64_t& time() const { return now; }
    inline uint64_t syncTime() { return (now = currentTimeMillis()); }
    inline void syncTime(const uint64_t& n) { now = n; }

private:
    uint64_t now {currentTimeMillis()};
    std::multimap<uint64_t, Sp<Job>> timers {}; /* the jobs ordered by time */
};

}
}
