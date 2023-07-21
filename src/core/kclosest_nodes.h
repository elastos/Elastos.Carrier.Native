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
#include <algorithm>
#include "carrier/id.h"
#include "carrier/node_info.h"

namespace elastos {
namespace carrier {

class DHT;
class KBucket;
class KBucketEntry;

class KClosestNodes {
public:
    KClosestNodes(DHT& _dht, const Id& _id, int _maxEntries);
    KClosestNodes(DHT& _dht, const Id& _id, int _maxEntries, std::function<bool(const Sp<KBucketEntry>&)> _filter);

    const Id& getTarget() const noexcept {
        return target;
    }

    int size() const noexcept {
        return entries.size();
    }

    void fill(bool includeSelf);
    void fill() {
        fill(false);
    }

    bool isFull() const noexcept {
        return entries.size() >= maxEntries;
    }

    const std::list<Sp<KBucketEntry>>& getEntries() const noexcept {
        return entries;
    }

    std::list<Sp<NodeInfo>> asNodeList() const {
        std::list<Sp<NodeInfo>> nodes;
        for (const auto& entry: entries) {
            nodes.emplace_back(std::static_pointer_cast<NodeInfo>(entry));
        }
        return nodes;
    }

private:
    void insertEntries(Sp<KBucket> bucket);
    void shave();

    DHT& dht;
    Id target;

    std::list<Sp<KBucketEntry>> entries {};
    int maxEntries {0};

    std::function<bool(const Sp<KBucketEntry>&)> filter;
};

} // namespace carrier
} // namespace elastos
