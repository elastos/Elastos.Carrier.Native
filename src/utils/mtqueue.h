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

#include <list>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

namespace elastos {
namespace carrier {

template <class T>
class MTQueue
{
private:
    mutable std::mutex mut;
    mutable std::condition_variable data_cond;

    using queue_type = std::queue<T>;
    queue_type data_queue;

public:
    using value_type = typename queue_type::value_type;
    using container_type = typename queue_type::container_type;

    MTQueue() = default;
    MTQueue(const MTQueue &) = delete;
    MTQueue &operator=(const MTQueue &) = delete;

    /*
     * A constructor that uses an iterator as a parameter applies to all container objects
     * */
    template <typename _InputIterator>
    MTQueue(_InputIterator first, _InputIterator last)
    {
        for (auto itor = first; itor != last; ++itor)
        {
            data_queue.push(*itor);
        }
    }
    explicit MTQueue(const container_type &c) : data_queue(c) {}

    /*
     * Use the constructor with an initialization list as a parameter
     * */
    MTQueue(std::initializer_list<value_type> list)
        : MTQueue(list.begin(), list.end()) {}

    void push(const value_type &new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }

    void add(const value_type &new_value) {
        push(new_value);
    }

    value_type pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return nullptr;
        auto value = std::move(data_queue.front());
        data_queue.pop();
        return value;
    }

    value_type poll() {
        return pop();
    }

    value_type peek() const {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return nullptr;
        auto value = data_queue.front();
        return value;
    }

    auto empty() const -> decltype(data_queue.empty()) {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

    auto size() const -> decltype(data_queue.size()) {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }
};

} // namespace carrier
} // namespace elastos
