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
#include <nlohmann/json.hpp>
#include <carrier/id.h>
#include <carrier/node_info.h>

#include "utils.h"
#include "../src/serializers.h"
#include "nodeinfo_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(NodeInfoTests);

void
NodeInfoTests::setUp() {
}

void
NodeInfoTests::testNodeInfo4() {
    auto id1 = Id::random();
    std::string address1 = "251.251.251.251";
    auto node1 = NodeInfo(id1, address1, 65535);
    CPPUNIT_ASSERT_EQUAL(id1, node1.getId());
    CPPUNIT_ASSERT_EQUAL(65535, node1.getPort());
    CPPUNIT_ASSERT(Utils::addressEquals(address1,  node1.getAddress().host()));

    nlohmann::json object1 = node1;
    NodeInfo node2 = object1;

    CPPUNIT_ASSERT_EQUAL(node1.getId(), node2.getId());
    CPPUNIT_ASSERT_EQUAL(node1.getPort(), node2.getPort());
    CPPUNIT_ASSERT_EQUAL(node1.getAddress().toString(), node2.getAddress().toString());

    auto id2 = Id::random();
    auto address2 = "192.168.1.2";
    auto node3 = NodeInfo(id2, address2, 1232);
    CPPUNIT_ASSERT_EQUAL(id2, node3.getId());
    CPPUNIT_ASSERT_EQUAL(1232, node3.getPort());
    CPPUNIT_ASSERT(Utils::addressEquals(address2,  node3.getAddress().host()));

    nlohmann::json object2 = node3;
    NodeInfo node4 = object2;

    CPPUNIT_ASSERT_EQUAL(node3.getId(), node4.getId());
    CPPUNIT_ASSERT_EQUAL(node3.getPort(), node4.getPort());
    CPPUNIT_ASSERT_EQUAL(node3.getAddress().toString(), node4.getAddress().toString());
}

void
NodeInfoTests::testNodeInfo6() {
    auto id1 = Id::random();
    std::string address1 = "2001:0db8:85a3:8070:6543:8a2e:0370:7334";
    auto node1 = NodeInfo(id1, address1, 65535);
    CPPUNIT_ASSERT_EQUAL(id1, node1.getId());
    CPPUNIT_ASSERT_EQUAL(65535, node1.getPort());
    CPPUNIT_ASSERT(Utils::addressEquals(address1, node1.getAddress().host()));

    nlohmann::json object1 = node1;
    NodeInfo node2 = object1;

    CPPUNIT_ASSERT_EQUAL(node1.getId(), node2.getId());
    CPPUNIT_ASSERT_EQUAL(node1.getPort(), node2.getPort());
    CPPUNIT_ASSERT_EQUAL(node1.getAddress().toString(), node2.getAddress().toString());

    auto id2 = Id::random();
    auto address2 = "2001:0db8:85a3:0000:0000:8a2e:0370:7332";
    auto node3 = NodeInfo(id2, address2, 1232);
    CPPUNIT_ASSERT_EQUAL(id2, node3.getId());
    CPPUNIT_ASSERT_EQUAL(1232, node3.getPort());
    CPPUNIT_ASSERT(Utils::addressEquals(address2, node3.getAddress().host()));

    nlohmann::json object2 = node3;
    NodeInfo node4 = object2;

    CPPUNIT_ASSERT_EQUAL(node3.getId(), node4.getId());
    CPPUNIT_ASSERT_EQUAL(node3.getPort(), node4.getPort());
    CPPUNIT_ASSERT_EQUAL(node3.getAddress().toString(), node4.getAddress().toString());
}

void
NodeInfoTests::tearDown() {
}
}  // namespace test
