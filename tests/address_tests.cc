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

#include "utils.h"
#include "carrier/socket_address.h"
#include "address_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(AddressTests);

void
AddressTests::setUp() {
}

void
AddressTests::testGlobalUnicastMatcher() {
    CPPUNIT_ASSERT(SocketAddress("8.8.8.8", 0).isGlobalUnicast());
    CPPUNIT_ASSERT(SocketAddress("2001:4860:4860::8888", 0).isGlobalUnicast());
    // wildcard
    CPPUNIT_ASSERT(false == SocketAddress("0.0.0.0", 0).isGlobalUnicast());
    CPPUNIT_ASSERT(false == SocketAddress("::0", 0).isGlobalUnicast());
    // loopback
    CPPUNIT_ASSERT(false == SocketAddress("127.0.0.15", 0).isGlobalUnicast());
    CPPUNIT_ASSERT(false == SocketAddress("::1", 0).isGlobalUnicast());
    // private/LL
    CPPUNIT_ASSERT(false == SocketAddress("192.168.13.47", 0).isGlobalUnicast());
    CPPUNIT_ASSERT(false == SocketAddress("169.254.1.0", 0).isGlobalUnicast());
    CPPUNIT_ASSERT(false == SocketAddress("fe80::", 0).isGlobalUnicast());
    // ULA
    CPPUNIT_ASSERT(false == SocketAddress("::1", 0).isGlobalUnicast());
    CPPUNIT_ASSERT(false == SocketAddress("fc00::", 0).isGlobalUnicast());
    CPPUNIT_ASSERT(false == SocketAddress("fd00::", 0).isGlobalUnicast());
}

void
AddressTests::testIsBogon() {
    CPPUNIT_ASSERT(false == SocketAddress("151.101.2.132", 1234).isBogon());
    CPPUNIT_ASSERT(SocketAddress("192.168.1.1", 1234).isBogon());
    CPPUNIT_ASSERT(SocketAddress("10.0.0.1", 1234).isBogon());
    CPPUNIT_ASSERT(SocketAddress("127.0.0.1", 1234).isBogon());
}

void
AddressTests::tearDown() {
}
}  // namespace test
