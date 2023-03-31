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

#include "carrier/prefix.h"
#include "utils/hex.h"

namespace elastos {
namespace carrier {

Id Prefix::last() const {
    auto p = Prefix(MAX_ID, depth);
    auto trailingBits = p.distance(MAX_ID);
    return this->distance(trailingBits);
}

Prefix Prefix::getParent() const {
    auto parent = Prefix(*this);
    if (depth == -1)
        return parent;

    // set last bit to zero
    parent.setTail(parent.depth--);
    return parent;
}

Prefix Prefix::splitBranch(bool highBranch) const {
    auto branch = Prefix(*this);
    int _depth = ++branch.depth;
    if (highBranch)
        branch.getData()[_depth / 8] |= 0x80 >> (_depth % 8);
    else
        branch.getData()[_depth / 8] &= ~(0x80 >> (_depth % 8));

    return branch;
}

bool Prefix::isSiblingOf(const Prefix& other) const {
    return depth == other.depth && Id::bitsEqual(*this, other, depth - 1);
}

/**
 * Generates a random Id that has falls under this prefix
 */
Id Prefix::createRandomId() const {
    Id id = Id::random();
    bitsCopy(*this, id, depth);
    return id;
}

bool Prefix::equals(const Prefix& p) const {
    if (depth != p.depth)
        return false;

    return Id::operator==(static_cast<Id>(p));
}

std::string Prefix::toBinaryString() const {
    if (depth == -1)
        return "all";

    std::string repr {};
    auto b = data();
    for (int i = 0; i <= depth; i++) {
        repr.append(1, (b[i >> 3] & (0x80 >> (i & 0x07))) != 0 ? '1' : '0');
        if ((i & 0x03) == 0x03)
            repr.append(1, ' ');
    }
    repr.append("...");
    return repr;
}

std::string Prefix::toString() const {
    if (depth == -1)
        return "all";

    auto _data = Hex::encode(data(), (depth + 8) >> 3);
    return _data + "/" + std::to_string(depth);
}

}
}
