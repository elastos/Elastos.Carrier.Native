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

// carrier
#include <carrier.h>
#include <utils.h>
#include "node_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(NodeTester);

void NodeTester::setUp() {
    dataDir = Utils::getPwdStorage("node_test_data");
    Utils::removeStorage(dataDir);

    auto b1 = DefaultConfiguration::Builder {};
    auto ipAddresses = Utils::getLocalIpAddresses();

    b1.setIPv4Address(ipAddresses);
    b1.setListeningPort(42222);
    b1.setStoragePath(dataDir + Utils::PATH_SEP + "temp1");

    node1 = std::make_shared<Node>(b1.build());
    node1->start();
}

void NodeTester::testNode() {
    //Maybe set to java node ip
    Id javaId("MzDfxDmCpgX6J9DtvttUsXDyTDwNJKKAmWaUW4XGRfs");
    auto nij = NodeInfo {javaId, Utils::getLocalIpAddresses(), 39001};
    node1->bootstrap(nij);

#if 1
    std::cout << "Trying to find Node " << javaId.toBase58String() << std::endl;
    auto future = node1->findNode(javaId);
    auto ni2 = future.get();
    std::cout << "ni2 size: " << ni2.size() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Node not found!", ni2.size());
    for (auto ni: ni2) {
        std::cout << "ni: " << static_cast<std::string>(*ni) << std::endl;
    }
#endif

#if 1
    std::cout << "----------" << std::endl;
    std::vector<uint8_t> blob({0,1,2,3,4});
    Sp<Value> value = node1->createValue(blob);
    std::cout << "Trying to Sotre Value " << std::endl;
    auto future1 = node1->storeValue(value);
    auto result = future1.get();
    CPPUNIT_ASSERT_MESSAGE("Store value failed!", result);
    std::cout << "Store value succeeeed." << std::endl;
#endif

#if 1
    std::cout << "----------" << std::endl;
    std::cout << "Trying to find Value with Id: " << value->getId().toBase58String() << std::endl;
    auto future2 = node1->findValue(value->getId());
    auto val = future2.get();
    CPPUNIT_ASSERT_MESSAGE("Value not found!", val != nullptr);
    std::cout << "Value: " << static_cast<std::string>(*val) << std::endl;
#endif

#if 1
    std::cout << "----------" << std::endl;
    std::cout << "Trying to announce peer " << std::endl;
    auto peerId = Id::random();
    auto future3 = node1->announcePeer(peerId, 42244, "testNode");
    auto result2 = future3.get();
    CPPUNIT_ASSERT_MESSAGE("Announce peer failed!", result2);
    std::cout << "Announce peer succeeeed." << std::endl;
#endif

#if 1
    std::cout << "----------" << std::endl;
    std::cout << "Trying to find peer with Id: " << peerId.toBase58String() << std::endl;

    //The node2(announcePeer node) can't save the peer now (the same as Java), so new the node3 for test
    auto b3 = DefaultConfiguration::Builder {};
    std::string _path = dataDir + Utils::PATH_SEP + "temp3";
    Utils::removeStorage(_path);

    b3.setStoragePath(_path);
    b3.setIPv4Address(Utils::getLocalIpAddresses());
    b3.setListeningPort(42224);

    node3 = std::make_shared<Node>(b3.build());
    node3->start();
    node3->bootstrap(nij);

    auto future4 = node3->findPeer(peerId, 1);
    auto peers = future4.get();
    CPPUNIT_ASSERT_MESSAGE("Peer not found!", !peers.empty());
    for (auto& peer: peers) {
        CPPUNIT_ASSERT(peer->isValid());
        std::cout << "Peer: " << static_cast<std::string>(*peer) << std::endl;
    }

    node3->stop();
#endif
}

void NodeTester::tearDown() {

    if (node1 != nullptr)
        node1->stop();

    Utils::removeStorage(dataDir);
}
}  // namespace test
