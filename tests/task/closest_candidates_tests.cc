/*
 * Copyright (c) 2022 Elastos Foundation
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
#include <list>
#include <set>

#include "task/closest_candidates.h"
#include "utils.h"
#include "closest_candidates_tests.h"
#include "carrier/id.h"

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(ClosestCandidatestsTests);

void
ClosestCandidatestsTests::setUp() {
}

void
ClosestCandidatestsTests::testAdd() {
    auto target = Id::random();
    auto cc = ClosestCandidates(target, 16);

    std::list<std::shared_ptr<NodeInfo>> nodes {};
    for (int i = 0; i < 8; i++) {
        std::string addr = "192.168.1." + std::to_string(i+1);
        auto node = std::make_shared<NodeInfo>(Id::random(), addr, 12345);
        nodes.push_back(node);
    }

    cc.add(nodes);

    CPPUNIT_ASSERT_EQUAL(8, cc.size());
    for (auto node : nodes) {
        auto cn = cc.get(node->getId());
        auto n = std::static_pointer_cast<NodeInfo>(cn);
        CPPUNIT_ASSERT_EQUAL(*node, *n);
    }

    // Duplicated node id
    std::list<std::shared_ptr<NodeInfo>> nodes2 {};
    int i = 0;
    for (auto it = nodes.begin(); it != nodes.end() && i < 8; it++, i++) {
        std::string addr = "192.168.1." + std::to_string(i+1);
        auto node = std::make_shared<NodeInfo>((*it)->getId(), addr, 12345);
        nodes2.push_back(node);
    }

    cc.add(nodes2);

    CPPUNIT_ASSERT_EQUAL(8, cc.size());
    for (auto node : nodes) {
        auto cn = cc.get(node->getId());
        auto n = std::static_pointer_cast<NodeInfo>(cn);
        CPPUNIT_ASSERT_EQUAL(*node, *n);
    }

    // Duplicated node address
    std::list<std::shared_ptr<NodeInfo>> nodes3 {};
    for (int i = 0; i < 8; i++) {
        std::string addr = "192.168.1." + std::to_string(i+1);
        auto node = std::make_shared<NodeInfo>(Id::random(), addr, 12345);
        nodes3.push_back(node);
    }

    CPPUNIT_ASSERT_EQUAL(8, cc.size());
    for (auto node : nodes) {
        auto cn = cc.get(node->getId());
        auto n = std::static_pointer_cast<NodeInfo>(cn);
        CPPUNIT_ASSERT_EQUAL(*node, *n);
    }

    // Another 16 good candidates
    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    for (int i = 0; i < 16; i++) {
        std::string addr = "192.168.1." + std::to_string(i+1);
        auto node = std::make_shared<NodeInfo>(Id::random(), addr, 12345);
        nodes4.push_back(node);
    }

    cc.add(nodes4);

    // Check the final result
    std::list<std::shared_ptr<NodeInfo>> all {};
    all.insert(all.end(), nodes.begin(), nodes.end());
    all.insert(all.end(), nodes4.begin(), nodes4.end());
    all.sort([&](const std::shared_ptr<NodeInfo> &node1, const std::shared_ptr<NodeInfo>& node2) {
        return target.threeWayCompare(node1->getId(), node2->getId()) < 0;
    });

    CPPUNIT_ASSERT_EQUAL(16, cc.size());
    i = 0;
    for (auto it = all.begin(); it != all.end() && i << cc.size(); it++, i++) {
        auto node = *it;
        auto cn = cc.get(node->getId());
        auto n = std::static_pointer_cast<NodeInfo>(cn);
        CPPUNIT_ASSERT_EQUAL(*node, *n);
    }
}

void
ClosestCandidatestsTests::testHeadAndTail() {
    auto target = Id::random();

    std::list<std::shared_ptr<NodeInfo>> result {};
    auto cc = ClosestCandidates(target, 16);

    std::list<std::shared_ptr<NodeInfo>> nodes {};
    for (int i = 0; i < 8; i++) {
        std::string addr = "192.168.1." + std::to_string(i+1);
        auto node = std::make_shared<NodeInfo>(Id::random(), addr, 12345);
        nodes.push_back(node);
        result.push_back(node);
        result.sort([&](const std::shared_ptr<NodeInfo> &node1, const std::shared_ptr<NodeInfo>& node2) {
            return target.threeWayCompare(node1->getId(), node2->getId()) < 0;
        });
    }

    cc.add(nodes);

    CPPUNIT_ASSERT_EQUAL(8, cc.size());
    CPPUNIT_ASSERT_EQUAL(result.front()->getId(), cc.head());
    CPPUNIT_ASSERT_EQUAL(result.back()->getId(), cc.tail());

    nodes.clear();
    for (int i = 8; i < 12; i++) {
        std::string addr = "192.168.1." + std::to_string(i+1);
        auto node = std::make_shared<NodeInfo>(Id::random(), addr, 12345);
        nodes.push_back(node);
        result.push_back(node);
        result.sort([&](const std::shared_ptr<NodeInfo> &node1, const std::shared_ptr<NodeInfo>& node2) {
            return target.threeWayCompare(node1->getId(), node2->getId()) < 0;
        });
    }

    cc.add(nodes);

    CPPUNIT_ASSERT_EQUAL(12, cc.size());
    CPPUNIT_ASSERT_EQUAL(result.front()->getId(), cc.head());
    CPPUNIT_ASSERT_EQUAL(result.back()->getId(), cc.tail());

    nodes.clear();
    for (int i = 12; i < 16; i++) {
        std::string addr = "192.168.1." + std::to_string(i+1);
        auto node = std::make_shared<NodeInfo>(Id::random(), addr, 12345);
        nodes.push_back(node);
        result.push_back(node);
        result.sort([&](const std::shared_ptr<NodeInfo> &node1, const std::shared_ptr<NodeInfo>& node2) {
            return target.threeWayCompare(node1->getId(), node2->getId()) < 0;
        });
    }

    cc.add(nodes);

    CPPUNIT_ASSERT_EQUAL(16, cc.size());
    CPPUNIT_ASSERT_EQUAL(result.front()->getId(), cc.head());
    CPPUNIT_ASSERT_EQUAL(result.back()->getId(), cc.tail());

    nodes.clear();

    for (int i = 16; i < 32; i++) {
        std::string addr = "192.168.1." + std::to_string(i+1);
        auto node = std::make_shared<NodeInfo>(Id::random(), addr, 12345);
        nodes.push_back(node);
        result.push_back(node);
    }

    result.sort([&](const std::shared_ptr<NodeInfo> &node1, const std::shared_ptr<NodeInfo>& node2) {
        return target.threeWayCompare(node1->getId(), node2->getId()) < 0;
    });

    for (int i = 0; i < 16; i++)
        result.pop_back();

    cc.add(nodes);

    CPPUNIT_ASSERT_EQUAL(16, cc.size());
    CPPUNIT_ASSERT(16 == result.size());
    CPPUNIT_ASSERT_EQUAL(result.front()->getId(), cc.head());
    CPPUNIT_ASSERT_EQUAL(result.back()->getId(), cc.tail());
}

void
ClosestCandidatestsTests::tearDown() {
}
}

