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
#include <functional>
#include <vector>
#include <algorithm>

namespace elastos {
namespace carrier {

template <class T>
inline T list_get(const std::list<T>& list, int index) {
    auto it = std::next(list.begin(), index);
        return *it;
}

template <class T>
inline void list_removeAll(std::list<T>& list, const std::vector<T>& toRemove) {
    for (auto it = toRemove.begin(); it != toRemove.end(); ++it) {
        auto list_it = std::find(list.begin(), list.end(), *it);
        if (list_it != list.end()) {
            list.erase(list_it);
        }
    }
}

template <class T>
inline void list_addAll(std::list<T>& list, const std::vector<T>& toAdd) {
    for (auto it = toAdd.begin(); it != toAdd.end(); ++it) {
        list.emplace_back(*it);
    }
}

template <class T>
inline bool vector_contains(const std::vector<T>& vector, const T& val) {
    auto it = std::find(vector.begin(), vector.end(), val);
    return it != vector.end();
}

} // namespace carrier
} // namespace elastos
