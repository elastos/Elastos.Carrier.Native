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

#include "def.h"
#include "id.h"

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC Prefix : public Id {
public:
    Prefix() noexcept : Id() {}
    Prefix(const Id& id, int _depth) noexcept : depth(_depth) {
        bitsCopy(id, *this, _depth);
    }
    Prefix(const Prefix& _prefix) noexcept
        : Prefix(_prefix, _prefix.getDepth()) {}

    int getDepth() const noexcept {
        return depth;
    }

    bool isPrefixOf(const Id& id) const noexcept {
        return bitsEqual(*this, id, depth);
    }

    bool isSplittable() const noexcept {
        return depth < (int)ID_BITS - 1;
    }

    Id first() const noexcept {
        return Id(*this);
    }

    Id last() const;

    Prefix getParent() const;
    Prefix splitBranch(bool highBranch) const;

    bool isSiblingOf(const Prefix& other) const;

    Id createRandomId() const;

    std::string toBinaryString() const;
    std::string toString() const;

    bool operator==(const Prefix& prefix) const {
        return equals(prefix);
    }

    operator std::string() const {
        return toString();
    }
private:
    bool equals(const Prefix &prefix) const;

    void setTail(int bit) noexcept {
        getData()[bit >> 3] &= ~(0x80 >> (bit & 0x07));
    }

    void setPos(int pos, uint8_t data) noexcept {
        getData()[pos] = data;
    }

    void setRemainZero(int pos, int j) noexcept {
        getData()[pos] = getData()[pos] & ~(0xff >> j);
    }

    /**
     * identifies the first bit of a key that has to be equal to be considered as
     * covered by this prefix -1 = prefix matches whole keyspace 0 = 0th bit must
     * match 1 = ...
     */
    int depth {-1};
};

} // namespace carrier
} // namespace elastos
