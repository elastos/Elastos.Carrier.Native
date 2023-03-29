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

#include "utils/list.h"
#include "task.h"

namespace elastos {
namespace carrier {

class DHT;
class KBucket;
class KBucketEntry;

class PingRefreshTask : public Task {
public:
    enum class Options {
        checkAll,
        removeOnTimeout,
        probeCache,
    };

    PingRefreshTask(DHT* dht, Sp<KBucket> bucket, const std::vector<Options>& options) : Task(dht, "PingRefresh") {
        checkAll = vector_contains(options, Options::checkAll);
        removeOnTimeout = vector_contains(options, Options::removeOnTimeout);
        probeCache = vector_contains(options, Options::probeCache);

        addBucket(bucket);
    }

    void addBucket(Sp<KBucket> bucket);

protected:
    void callTimeout(RPCCall* call) override;
    void update() override;
    bool isDone() const override {
        return todo.empty() && Task::isDone();
    }

private:
    Sp<KBucket> bucket;
    std::list<Sp<KBucketEntry>> todo {};

    bool checkAll { false };
    bool probeCache {false };
    bool removeOnTimeout { false };
};

}
}
