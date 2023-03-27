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
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

// std
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

// carrier
#include <carrier.h>
#include "utils.h"
#include "node_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(NodeTester);

void NodeTester::setUp() {
    std::string pwd = getenv("PWD");
    Utils::removeStorage(pwd + "/temp1");
    Utils::removeStorage(pwd + "/temp2");

    auto b1 = DefaultConfiguration::Builder {};
    auto ipAddresses = Utils::getLocalIpAddresses();

    b1.setIPv4Address(ipAddresses);
    b1.setListeningPort(42222);
    b1.setStoragePath(pwd + "/temp1");

    node1 = std::make_shared<Node>(b1.build());
    node1->start();

#ifndef TEST_JAVA
    auto b2 = DefaultConfiguration::Builder {};
    b2.setIPv4Address(ipAddresses);
    b2.setListeningPort(42223);
    b2.setStoragePath(pwd + "/temp2");

    node2 = std::make_shared<Node>(b2.build());
    node2->start();
#endif
}

void NodeTester::testLocalNode() {
    auto ni1 = NodeInfo {node1->getId(), Utils::getLocalIpAddresses(), node1->getPort()};
    node2->bootstrap(ni1);

#if 1
    std::cout << "Trying to find Node " << node2->getId() << std::endl;
    auto future = node1->findNode(node2->getId());
    auto ni2 = future.get();
    std::cout << "ni2 size: " << ni2.size() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Node not found!", ni2.size());
    for (auto ni: ni2) {
        std::cout << "ni: " << *ni << std::endl;
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
    std::cout << "Trying to find Value with Id: " << value->getId() << std::endl;
    auto future2 = node1->findValue(value->getId());
    auto val = future2.get();
    CPPUNIT_ASSERT(*value == *val);
    CPPUNIT_ASSERT_MESSAGE("Value not found!", val != nullptr);
    std::cout << "Value: " << static_cast<std::string>(*val) << std::endl;
#endif

#if 1
    std::cout << "----------" << std::endl;
    std::cout << "Trying to announce peer " << std::endl;
    auto peerId = Id::random();
    auto future3 = node2->announcePeer(peerId, 42244);
    auto result2 = future3.get();
    CPPUNIT_ASSERT_MESSAGE("Announce peer failed!", result2);
    std::cout << "Announce peer succeeeed." << std::endl;
#endif

#if 1
    std::cout << "----------" << std::endl;
    std::cout << "Trying to find peer with Id: " << peerId << std::endl;

    //The node2(announcePeer node) can't save the peer now (the same as Java), so new the node3 for test
    auto b3 = DefaultConfiguration::Builder {};
    std::string path = getenv("PWD");
    std::string _path = path + "/temp3";
    Utils::removeStorage(_path);

    b3.setStoragePath(_path);
    b3.setIPv4Address(Utils::getLocalIpAddresses());
    b3.setListeningPort(42224);

    node3 = std::make_shared<Node>(b3.build());
    node3->start();
    node3->bootstrap(ni1);

    auto future4 = node3->findPeer(peerId, 1);
    auto peers = future4.get();
    CPPUNIT_ASSERT_MESSAGE("Peer not found!", !peers.empty());
    for (auto& peer: peers) {
        std::cout << "Peer: " << peer << std::endl;
    }

    node3->stop();
#endif
}

#ifdef TEST_JAVA
void NodeTester::testJavaNode() {
    //Maybe set to java node ip
    Id javaId("MzDfxDmCpgX6J9DtvttUsXDyTDwNJKKAmWaUW4XGRfs");
    auto nij = NodeInfo {javaId, Utils::getLocalIpAddresses(), 39001};
    node1->bootstrap(nij);

#if 1
    std::cout << "Trying to find Node " << javaId << std::endl;
    auto future = node1->findNode(javaId);
    auto ni2 = future.get();
    std::cout << "ni2 size: " << ni2.size() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Node not found!", ni2.size());
    for (auto ni: ni2) {
        std::cout << "ni: " << *ni << std::endl;
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
    std::cout << "Trying to find Value with Id: " << value->getId() << std::endl;
    auto future2 = node1->findValue(value->getId());
    auto val = future2.get();
    CPPUNIT_ASSERT_MESSAGE("Value not found!", val != nullptr);
    std::cout << "Value: " << static_cast<std::string>(*val) << std::endl;
#endif

#if 1
    std::cout << "----------" << std::endl;
    std::cout << "Trying to announce peer " << std::endl;
    auto peerId = Id::random();
    auto future3 = node1->announcePeer(peerId, 42244);
    auto result2 = future3.get();
    CPPUNIT_ASSERT_MESSAGE("Announce peer failed!", result2);
    std::cout << "Announce peer succeeeed." << std::endl;
#endif

#if 1
    std::cout << "----------" << std::endl;
    std::cout << "Trying to find peer with Id: " << peerId << std::endl;

    //The node2(announcePeer node) can't save the peer now (the same as Java), so new the node3 for test
    auto b3 = DefaultConfiguration::Builder {};
    std::string path = getenv("PWD");
    Utils::removeStorage(path + "/temp3");

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
        std::cout << "Peer: " << peer << std::endl;
    }

    node3->stop();
#endif
}
#endif

void NodeTester::tearDown() {
    if (node2 != nullptr)
        node2->stop();

    if (node1 != nullptr)
        node1->stop();

    std::string pwd = getenv("PWD");
    Utils::removeStorage(pwd + "/temp1");
    Utils::removeStorage(pwd + "/temp2");
    Utils::removeStorage(pwd + "/temp3");
}
}  // namespace test
