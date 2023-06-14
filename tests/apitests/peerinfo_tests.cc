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
#include <carrier.h>

#include "utils.h"
#include "serializers.h"
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
    auto pid1 = Id::random();
    uint16_t port = 65535;
    std::vector<uint8_t> sig1(64, 1);
    std::string address1 = "251.251.251.251";
    auto peer1 = PeerInfo(id1, pid1, port, address1, sig1);
    CPPUNIT_ASSERT(peer1.isIPv4());
    CPPUNIT_ASSERT(!peer1.isIPv6());
    CPPUNIT_ASSERT_EQUAL(id1, peer1.getNodeId());
    CPPUNIT_ASSERT_EQUAL(pid1, peer1.getProxyId());
    CPPUNIT_ASSERT_EQUAL(port, peer1.getPort());
    CPPUNIT_ASSERT_EQUAL(address1, peer1.getAlt());
    CPPUNIT_ASSERT(sig1 == peer1.getSignature());
    CPPUNIT_ASSERT(peer1.isUsedProxy());

    nlohmann::json object1 = peer1;
    PeerInfo peer2 = object1;

    CPPUNIT_ASSERT_EQUAL(peer1.getNodeId(), peer2.getNodeId());
    CPPUNIT_ASSERT_EQUAL(peer1.getPort(), peer2.getPort());
    CPPUNIT_ASSERT_EQUAL(peer1.getProxyId(), peer2.getProxyId());
    CPPUNIT_ASSERT(peer1.getSignature() == peer2.getSignature());
    CPPUNIT_ASSERT_EQUAL(peer1.getAlt(), peer2.getAlt());
    CPPUNIT_ASSERT_EQUAL(peer1.getFamily(), peer2.getFamily());

    CPPUNIT_ASSERT(peer1 ==  peer2);
}

void
PeerInfoTests::testPeerInfo6() {
    auto id1 = Id::random();
    uint16_t port = 65535;
    std::vector<uint8_t> sig1(64, 2);
    std::string address1 = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";
    auto peer1 = PeerInfo(id1, Id(), port, address1, sig1, AF_INET6);
    CPPUNIT_ASSERT(!peer1.isIPv4());
    CPPUNIT_ASSERT(peer1.isIPv6());
    CPPUNIT_ASSERT_EQUAL(id1, peer1.getNodeId());
    CPPUNIT_ASSERT_EQUAL(Id::zero(), peer1.getProxyId());
    CPPUNIT_ASSERT_EQUAL(port, peer1.getPort());
    CPPUNIT_ASSERT_EQUAL(address1, peer1.getAlt());
    CPPUNIT_ASSERT(sig1 == peer1.getSignature());
    CPPUNIT_ASSERT(!peer1.isUsedProxy());

    nlohmann::json object1 = peer1;
    PeerInfo peer2 = object1;

    CPPUNIT_ASSERT_EQUAL(peer1.getNodeId(), peer2.getNodeId());
    CPPUNIT_ASSERT_EQUAL(peer1.getProxyId(), peer2.getProxyId());
    CPPUNIT_ASSERT_EQUAL(peer1.getPort(), peer2.getPort());
    CPPUNIT_ASSERT_EQUAL(peer1.getAlt(), peer2.getAlt());
    CPPUNIT_ASSERT(peer1.getSignature() == peer2.getSignature());

    // CPPUNIT_ASSERT(peer1 ==  peer2); the family don't serialize
}

void
PeerInfoTests::tearDown() {
}
}  // namespace test
