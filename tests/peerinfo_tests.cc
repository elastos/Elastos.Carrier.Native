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

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <carrier/id.h>
#include <carrier/peer_info.h>

#include "utils.h"
#include "peerinfo_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(PeerInfoTests);

void
PeerInfoTests::setUp() {
}

void
PeerInfoTests::testPeerInfo4() {
    auto id1 = Id::random();
    std::string address1 = "251.251.251.251";
    auto peer1 = PeerInfo(id1, address1, 65535);
    CPPUNIT_ASSERT(peer1.isIPv4());
    CPPUNIT_ASSERT(!peer1.isIPv6());
    CPPUNIT_ASSERT_EQUAL(id1, peer1.getNodeId());
    CPPUNIT_ASSERT_EQUAL(65535, peer1.getPort());
    CPPUNIT_ASSERT(Utils::addressEquals(address1,  peer1.getAddress().host()));

    nlohmann::json object1 = peer1;
    PeerInfo peer2 = object1;

    CPPUNIT_ASSERT_EQUAL(peer1.getNodeId(), peer2.getNodeId());
    CPPUNIT_ASSERT_EQUAL(peer1.getPort(), peer2.getPort());
    CPPUNIT_ASSERT_EQUAL(peer1.getAddress().toString(), peer2.getAddress().toString());

    auto id2 = Id::random();
    auto address2 = "192.168.1.2";
    auto peer3 = PeerInfo(id2, address2, 1232);
    CPPUNIT_ASSERT(peer3.isIPv4());
    CPPUNIT_ASSERT(!peer3.isIPv6());
    CPPUNIT_ASSERT_EQUAL(id2, peer3.getNodeId());
    CPPUNIT_ASSERT_EQUAL(1232, peer3.getPort());
    CPPUNIT_ASSERT(Utils::addressEquals(address2,  peer3.getAddress().host()));

    nlohmann::json object2 = peer3;
    PeerInfo peer4 = object2;

    CPPUNIT_ASSERT_EQUAL(peer3.getNodeId(), peer4.getNodeId());
    CPPUNIT_ASSERT_EQUAL(peer3.getPort(), peer4.getPort());
    CPPUNIT_ASSERT_EQUAL(peer3.getAddress().toString(), peer4.getAddress().toString());
}

void
PeerInfoTests::testPeerInfo6() {
    auto id1 = Id::random();
    std::string address1 = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";
    auto peer1 = PeerInfo(id1, address1, 65535);
    CPPUNIT_ASSERT(!peer1.isIPv4());
    CPPUNIT_ASSERT(peer1.isIPv6());
    CPPUNIT_ASSERT_EQUAL(id1, peer1.getNodeId());
    CPPUNIT_ASSERT_EQUAL(65535, peer1.getPort());
    CPPUNIT_ASSERT(Utils::addressEquals(address1, peer1.getAddress().host()));

    nlohmann::json object1 = peer1;
    PeerInfo peer2 = object1;

    CPPUNIT_ASSERT_EQUAL(peer1.getNodeId(), peer2.getNodeId());
    CPPUNIT_ASSERT_EQUAL(peer1.getPort(), peer2.getPort());
    CPPUNIT_ASSERT_EQUAL(peer1.getAddress().toString(), peer2.getAddress().toString());

    auto id2 = Id::random();
    auto address2 = "2001:0db8:85a3:8070:6543:8a2e:0370:7334";
    auto peer3 = PeerInfo(id2, address2, 1232);
    CPPUNIT_ASSERT(!peer3.isIPv4());
    CPPUNIT_ASSERT(peer3.isIPv6());
    CPPUNIT_ASSERT_EQUAL(id2, peer3.getNodeId());
    CPPUNIT_ASSERT_EQUAL(1232, peer3.getPort());
    CPPUNIT_ASSERT(Utils::addressEquals(address2, peer3.getAddress().host()));

    nlohmann::json object2 = peer3;
    PeerInfo peer4 = object2;

    CPPUNIT_ASSERT_EQUAL(peer3.getNodeId(), peer4.getNodeId());
    CPPUNIT_ASSERT_EQUAL(peer3.getPort(), peer4.getPort());
    CPPUNIT_ASSERT_EQUAL(peer3.getAddress().toString(), peer4.getAddress().toString());
}

void
PeerInfoTests::tearDown() {
}
}  // namespace test
