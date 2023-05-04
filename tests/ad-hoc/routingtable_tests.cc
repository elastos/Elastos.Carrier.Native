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

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>

// std
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <chrono>
#include <thread>

// carrier
#include <carrier.h>
#include <utils.h>
#include "routingtable_tests.h"

using namespace std::chrono_literals;
using namespace elastos::carrier;
extern bool stopped;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(RoutingTableTester);

#define TEST_COUNT 16

void RoutingTableTester::setUp() {
    dataDir = Utils::getPwdStorage("/routingtable_test_data");
    Utils::removeStorage(dataDir);
    nodes.clear();
}

void RoutingTableTester::testRoutingTable() {
    //Maybe set to java node ip
    Id javaId("MzDfxDmCpgX6J9DtvttUsXDyTDwNJKKAmWaUW4XGRfs");
    auto nij = NodeInfo {javaId, Utils::getLocalIpAddresses(), 39001};

    auto builder = DefaultConfiguration::Builder {};
    auto ipAddresses = Utils::getLocalIpAddresses();
    builder.setIPv4Address(ipAddresses);
    uint16_t port = 42222;

    for (int i = 0; i < TEST_COUNT; i++) {
        std::cout << "-- RoutingTableTester create node: " << i << std::endl;
        builder.setListeningPort(port++);
        builder.setStoragePath(dataDir + "/node" + std::to_string(i));

        auto node = std::make_shared<Node>(builder.build());
        nodes.emplace_back(node);
        node->start();
        node->bootstrap(nij);
    }

    while (!stopped) {
        std::this_thread::sleep_for(1000ms);
    }
}

void RoutingTableTester::tearDown() {
    for (auto node : nodes) {
        node->stop();
    }
    nodes.clear();
}
}  // namespace test
