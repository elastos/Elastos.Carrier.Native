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
PeerInfoTests::testPeerInfo() {
    auto id1 = Id::random();
    auto pid1 = Id::random();
    uint16_t port = 65535;
    std::vector<uint8_t> sig1(64, 1);
    std::string address1 = "251.251.251.251";
    auto peer1 = PeerInfo::of(id1, pid1, port, address1, sig1);

    CPPUNIT_ASSERT_EQUAL(id1, peer1.getNodeId());
    CPPUNIT_ASSERT_EQUAL(pid1, peer1.getNodeId());
    CPPUNIT_ASSERT_EQUAL(port, peer1.getPort());
    CPPUNIT_ASSERT_EQUAL(address1, peer1.getAlternativeURL());
    CPPUNIT_ASSERT(sig1 == peer1.getSignature());
    CPPUNIT_ASSERT(peer1.isDelegated());

    // nlohmann::json object1 = peer1;
    // PeerInfo peer2 = object1;

    // CPPUNIT_ASSERT(peer1 ==  peer2);
}

void
PeerInfoTests::tearDown() {
}
}  // namespace test
