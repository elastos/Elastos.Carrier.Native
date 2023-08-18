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

#include <string>
#include <carrier.h>
#include "nodeinfo_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(NodeInfoTests);

void NodeInfoTests::test1() {
    auto id = Id::random().blob();
    auto ip = "192.168.1.100";
    auto port = 12345;
    auto address = SocketAddress(ip, port);

    auto node = NodeInfo(id, Blob(address.inaddr(), address.inaddrLength()), port);
    CPPUNIT_ASSERT(node.getId().blob() == id);
    CPPUNIT_ASSERT(node.getAddress() == address);
    CPPUNIT_ASSERT(node.getAddress().host() == ip);
    CPPUNIT_ASSERT(node.getAddress().port() == port);
    CPPUNIT_ASSERT(node.getPort() == port);
    CPPUNIT_ASSERT(node.getVersion() == 0);
    CPPUNIT_ASSERT(node.isIPv4());
    CPPUNIT_ASSERT(!node.isIPv6());
}

void NodeInfoTests::test2() {
    auto id = Id::random().toHexString();
    auto ip = "192.168.1.100";
    auto port = 12345;

    auto node = NodeInfo(id, ip, port);
    CPPUNIT_ASSERT(node.getId().toHexString() == id);
    CPPUNIT_ASSERT(node.getAddress().host() == ip);
    CPPUNIT_ASSERT(node.getAddress().port() == port);
    CPPUNIT_ASSERT(node.getPort() == port);
    CPPUNIT_ASSERT(node.getVersion() == 0);
    CPPUNIT_ASSERT(node.isIPv4());
    CPPUNIT_ASSERT(!node.isIPv6());
}

void NodeInfoTests::test3() {
    auto id = Id::random();
    auto ip = "192.168.1.100";
    auto port = 12345;
    auto version = 9;

    auto node = NodeInfo(id, ip, port);
    node.setVersion(version);
    CPPUNIT_ASSERT(node.getId() == id);
    CPPUNIT_ASSERT(node.getAddress().host() == ip);
    CPPUNIT_ASSERT(node.getAddress().port() == port);
    CPPUNIT_ASSERT(node.getPort() == port);
    CPPUNIT_ASSERT(node.getVersion() == version);
    CPPUNIT_ASSERT(node.isIPv4());
    CPPUNIT_ASSERT(!node.isIPv6());
}

void NodeInfoTests::test4() {
    auto id = Id::random();
    auto ip = "192.168.1.100";
    auto port = 12345;
    auto address = SocketAddress(ip, port);

    auto node = NodeInfo(id, Blob(address.inaddr(), address.inaddrLength()), port);
    CPPUNIT_ASSERT(node.getId() == id);
    CPPUNIT_ASSERT(node.getAddress() == address);
    CPPUNIT_ASSERT(node.getAddress().host() == ip);
    CPPUNIT_ASSERT(node.getAddress().port() == port);
    CPPUNIT_ASSERT(node.getPort() == port);
}

void NodeInfoTests::test5() {
    auto id = Id::random();
    auto ip = "192.168.1.100";
    auto port = 12345;
    auto address = SocketAddress(ip, port);

    auto node = NodeInfo(id, address.addr());
    CPPUNIT_ASSERT(node.getId() == id);
    CPPUNIT_ASSERT(node.getAddress() == address);
    CPPUNIT_ASSERT(node.getAddress().host() == ip);
    CPPUNIT_ASSERT(node.getAddress().port() == port);
    CPPUNIT_ASSERT(node.getPort() == port);
}

void NodeInfoTests::test6() {
    auto id = Id::random();
    auto ip = "192.168.1.100";
    auto port = 12345;
    auto address = SocketAddress(ip, port);

    auto node = NodeInfo(id, address);
    CPPUNIT_ASSERT(node.getId() == id);
    CPPUNIT_ASSERT(node.getAddress() == address);
    CPPUNIT_ASSERT(node.getAddress().host() == ip);
    CPPUNIT_ASSERT(node.getAddress().port() == port);
    CPPUNIT_ASSERT(node.getPort() == port);
}

void NodeInfoTests::testIPv6() {
    auto id = Id::random();
    auto ip = "2001:0db8:85a3:8070:6543:8a2e:0370:7334";
    auto port = 12345;
    auto version = 5;

    auto node = NodeInfo(id, ip, port);
    node.setVersion(version);
    CPPUNIT_ASSERT(node.getId() == id);
    // CPPUNIT_ASSERT(node.getAddress().host() == ip);
    CPPUNIT_ASSERT(node.getAddress().port() == port);
    CPPUNIT_ASSERT(node.getPort() == port);
    CPPUNIT_ASSERT(node.getVersion() == version);
    CPPUNIT_ASSERT(node.isIPv6());
    CPPUNIT_ASSERT(!node.isIPv4());
}

void  NodeInfoTests::testEquals() {
    auto id = Id::random();
    auto ip = "192.168.1.100";
    auto port = 12345;

    auto node1 = NodeInfo(id, ip, port);
    auto node2 = NodeInfo(id, ip, port);
    CPPUNIT_ASSERT(node1.equals(node2));
    CPPUNIT_ASSERT(node1 == node2);

}

void  NodeInfoTests::testMatches1() {
    auto ip = "192.168.1.100";
    auto port = 12345;

    auto node1 = NodeInfo(Id::random(), ip, port);
    auto node2 = NodeInfo(Id::random(), ip, port);
    CPPUNIT_ASSERT(node1.matches(node2));
    CPPUNIT_ASSERT(!node1.equals(node2));
    CPPUNIT_ASSERT(node1 != node2);
}

void  NodeInfoTests::testMatches2() {
    auto id = Id::random();

    auto node1 = NodeInfo(id, "192.168.1.100", 12345);
    auto node2 = NodeInfo(id, "192.168.1.101", 12346);
    CPPUNIT_ASSERT(node1.matches(node2));
    CPPUNIT_ASSERT(!node1.equals(node2));
    CPPUNIT_ASSERT(node1 != node2);
}

}  // namespace test
