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
#include <iostream>
#include <string>
#include <cctype>
#include <memory>
#include <algorithm>
#include <random>

#include <carrier.h>

#include "utils/hex.h"
#include "prefix_tests.h"

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(PrefixTests);

typedef elastos::carrier::Id             Id;
typedef elastos::carrier::Hex            Hex;
typedef elastos::carrier::Prefix         Prefix;

void
PrefixTests::setUp() {
}

void PrefixTests::testIsPrefixOf() {
    Id id = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");
    Prefix prefix = Prefix(id, 64);

    CPPUNIT_ASSERT(prefix.isPrefixOf(id));

    id = Id("0x4833af415161cbd0f3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");
    CPPUNIT_ASSERT(prefix.isPrefixOf(id));
    id = Id("0x4833af415161cbd0ffffffffffffffffffffffffffffffffffffffffffffffff");
    CPPUNIT_ASSERT(prefix.isPrefixOf(id));
    id = Id("0x4833af415161cbd1f3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");
    CPPUNIT_ASSERT(prefix.isPrefixOf(id) == false);
}

void PrefixTests::testIsSplitable() {
    for (int i = -1; i < ID_BITS - 2; i++) {
        Id id = Id::random();
        Prefix p = Prefix(id, i);
        CPPUNIT_ASSERT(p.isSplittable());
    }

    Id id = Id::random();
    Prefix p = Prefix(id, ID_BITS - 1);
    CPPUNIT_ASSERT(p.isSplittable() == false);
}

void PrefixTests::testIsSiblingOf() {
    Id id  = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");
    Id id2 = Id("0x4833af415161cbd0a3ef8faa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");
    Id id3 = Id("0x4833af415161cbd0a3ef93aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");

    Prefix p = Prefix(id, 84);
    Prefix p2 = Prefix(id2, 84);
    Prefix p3 = Prefix(id3, 84);

    CPPUNIT_ASSERT(p2.isSiblingOf(p));
    CPPUNIT_ASSERT(p3.isSiblingOf(p) == false);
}

void PrefixTests::testFirstAndLast() {
    for (int i = 0; i < ID_BITS - 1; i++) {
        Id id = Id::random();

        Prefix p = Prefix(id, i);

        auto first = p.first();
        auto last = p.last();

        CPPUNIT_ASSERT(p.isPrefixOf(first));
        CPPUNIT_ASSERT(p.isPrefixOf(last));
    }
}

void PrefixTests::testGetParent() {
    Id id = Id::random();

    Prefix prefix = Prefix(id, -1);
    auto parent = prefix.getParent();
    CPPUNIT_ASSERT(prefix == parent);

    for (int i = 0; i < ID_BITS; i++) {
        id = elastos::carrier::Id::MAX_ID;

        prefix = Prefix(id, i);
        parent = prefix.getParent();

        CPPUNIT_ASSERT(prefix.getDepth() == parent.getDepth() + 1);
        CPPUNIT_ASSERT(parent.isPrefixOf(prefix));
        CPPUNIT_ASSERT(Id::bitsEqual(parent, prefix, i - 1));
        CPPUNIT_ASSERT(Id::bitsEqual(parent, prefix, i) == false);
    }
}

void PrefixTests::testCreateRandomId() {
    for (int i = -1; i < ID_BITS; i++) {
        Id id = Id::random();
        Prefix prefix = Prefix(id, i);

        Id rid = prefix.createRandomId();

        CPPUNIT_ASSERT(prefix.isPrefixOf(id));
        CPPUNIT_ASSERT(prefix.isPrefixOf(rid));
        CPPUNIT_ASSERT(Id::bitsEqual(id, rid, i));

        std::cout << i << ": " << id << std::endl;
        std::cout << "pre: " << prefix << std::endl;
        std::cout << "rid: " << rid << std::endl;
    }
}

void PrefixTests::testSplitBranch() {
    for  (int i = -1; i < ID_BITS - 1; i++) {
        Id id = Id::random();
        auto p = Prefix(id, i);
        auto l = p.splitBranch(false);
        auto h = p.splitBranch(true);

        CPPUNIT_ASSERT(p.isPrefixOf(l));
        CPPUNIT_ASSERT(p.isPrefixOf(h));

        auto parent = l.getParent();
        CPPUNIT_ASSERT(p == parent);
        parent = h.getParent();
        CPPUNIT_ASSERT(p == parent);

        CPPUNIT_ASSERT(Id::bitsEqual(l, h, p.getDepth()));
        CPPUNIT_ASSERT(Id::bitsEqual(l, h, p.getDepth() + 1) == false);
    }
}

void PrefixTests::testGetCommonPrefix() {
/*
    for (int depth = -1; depth < ID_BITS; depth++) {
        Id id = Id::random();
        Prefix p = Prefix(id, depth);

        std::random_device rd;
        std::uniform_int_distribution<int> dist(0, 128);
        int n = dist(rd);

        std::vector<Id> ids {};
        for (int i = 0; i < n; i++)
            ids.push_back(p.createRandomId());

        Prefix cp = Prefix::getCommonPrefix(ids);
        CPPUNIT_ASSERT(p == cp);
    }
*/
}

void
PrefixTests::tearDown() {
}
}  // namespace test
