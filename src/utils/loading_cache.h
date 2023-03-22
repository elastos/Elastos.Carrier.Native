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

namespace elastos {
namespace carrier {

template <typename Key, typename Value>
class LocadingCache{
public:
    LocadingCache(int _ttl) : ttl(_ttl) { }

    Value& get(Key key) {
        auto it = cache.find(key);
        if (it == cache.end()) {
            cache[key] = Entry(load(key), ttl);
            it = cache.find(key);
        }
        else {
            it->second.setExpirationTime(ttl);
        }
        return it->second.value;
    };

    void handleExpiration() {
        auto now = currentTimeMillis();
        auto it = cache.begin();
        while (it != cache.end()) {
            auto entry = (*it).second;
            auto cur_it = it++;
            if (entry.expired(now)) {
                onRemoval(entry.value);
                cache.erase(cur_it);
            }
        }
    };

private:
    struct Entry {
        Entry() {};
        ~Entry()=default;

        Entry(const Entry& e) : value(e.value), expiration_time(e.expiration_time) {}
        Entry(Entry&& e) : value(e.value), expiration_time(e.expiration_time) {}

        Entry& operator=(const Entry& o) noexcept {
            value = o.value;
            expiration_time = o.expiration_time;
            return *this;
        }

        Entry& operator=(Entry&& o) noexcept {
            value = o.value;
            expiration_time = o.expiration_time;
            return *this;
        }

        Entry(Value&& _value, int valid): value(_value),
                expiration_time(currentTimeMillis() + valid) {}

        bool expired(uint64_t now) const {
            return now >= expiration_time;
        }

        void setExpirationTime(int valid) {
            expiration_time = currentTimeMillis() + valid;
        }

        Value value;
        uint64_t expiration_time = 0;
    };

    virtual Value load(const Key &key) = 0;
    virtual void onRemoval(const Value &val) = 0;

    std::map<Key, Entry> cache {};
    int ttl;
};

} // namespace carrier
} // namespace elastos
