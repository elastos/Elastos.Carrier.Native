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
//#include <algorithm>

// carrier
#include <carrier.h>
#include "utils.h"
#include "node_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(NodeTests);

void NodeTests::setUp() {
    auto path1 = Utils::getPwdStorage("node1");
    auto path2 = Utils::getPwdStorage("node2");
    auto path3 = Utils::getPwdStorage("node3");

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);
    Utils::removeStorage(path3);

    auto ipAddress = Utils::getLocalIpAddresses();

    //create and runnode1
    auto b1 = DefaultConfiguration::Builder {};
    b1.setIPv4Address(ipAddress);
    b1.setListeningPort(32222);
    b1.setStoragePath(path1);

    node1 = std::make_shared<Node>(b1.build());
    node1->start();

    // create and run node2.
    auto b2 = DefaultConfiguration::Builder {};
    b2.setIPv4Address(ipAddress);
    b2.setListeningPort(32224);
    b2.setStoragePath(path2);

    node2 = std::make_shared<Node>(b2.build());
    node2->start();

    // create and run node3.
    auto b3 = DefaultConfiguration::Builder {};
    b3.setIPv4Address(ipAddress);
    b3.setListeningPort(32225);
    b3.setStoragePath(path3);

    node3 = std::make_shared<Node>(b3.build());
    node3->start();

    auto nodeInfo = NodeInfo {node1->getId(), ipAddress, node1->getPort()};
    node2->bootstrap(nodeInfo);
    node3->bootstrap(nodeInfo);
}

void NodeTests::tearDown() {
    if (node1)
        node1->stop();
    if (node2)
        node2->stop();
    if (node3)
        node3->stop();

    auto path1 = Utils::getPwdStorage("node1");
    auto path2 = Utils::getPwdStorage("node2");
    auto path3 = Utils::getPwdStorage("node3");

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);
    Utils::removeStorage(path3);
}

void NodeTests::testFindNode() {
    auto remoteId = node2->getId();
    std::cout << "-----Find node-----" << std::endl;
    std::cout << "Trying to find Node " << remoteId.toString() << std::endl;
    auto future = node1->findNode(remoteId);
    auto result = future.get();
    std::cout << "ni2 size: " << result.size() << std::endl;
    CPPUNIT_ASSERT(result.size());
    for (const auto& item: result) {
        std::cout << "ni: " << item->toString() << std::endl;
    }
}

void NodeTests::testFindValue() {
    auto remoteId = node2->getId();

    std::cout << "-----Store value-----" << std::endl;
    std::vector<uint8_t> data({0, 1, 2, 3, 4});
    auto value = Value::createValue(data);
    std::cout << "Trying to Sotre Value " << std::endl;
    auto future = node1->storeValue(value);
    future.get();
    std::cout << "Store value succeeeed." << std::endl;

	auto signedValue = Value::createSignedValue(data);
    future = node1->storeValue(signedValue);
    future.get();
    std::cout << "Store signedValue succeeeed." << std::endl;

    auto encryptedValue = Value::createEncryptedValue(remoteId, data);
    future = node1->storeValue(encryptedValue);
    future.get();
    std::cout << "Store encryptedValue succeeeed." << std::endl;

    std::cout << "-----Find value-----" << std::endl;
    std::cout << "Trying to find Value with Id: " << value.getId().toString() << std::endl;
    auto future1 = node2->findValue(value.getId());
    auto foundValue = future1.get();
    CPPUNIT_ASSERT(foundValue);
    CPPUNIT_ASSERT(foundValue->isValid());
    std::cout << "Value: " << foundValue->toString() << std::endl;

    std::cout << "Trying to find SignedValue with Id: " << signedValue.getId().toString() << std::endl;
    future1 = node2->findValue(signedValue.getId());
    foundValue = future1.get();
    CPPUNIT_ASSERT(foundValue);
    CPPUNIT_ASSERT(foundValue->isValid());
    std::cout << "SignedValue: " << foundValue->toString() << std::endl;

    std::cout << "Trying to find EncryptedValue with Id: " << encryptedValue.getId().toString() << std::endl;
    future1 = node2->findValue(encryptedValue.getId());
    foundValue = future1.get();
    CPPUNIT_ASSERT(foundValue);
    CPPUNIT_ASSERT(foundValue->isValid());
    std::cout << "EncryptedValue: " << foundValue->toString() << std::endl;


    std::cout << "-----Update value-----" << std::endl;
    std::cout << "Trying to update signedValue with Id: " << signedValue.getId().toString() << std::endl;
    std::vector<uint8_t> data2({5, 6, 7, 8});
    auto updated = signedValue.update(data2);
    future = node1->storeValue(updated);
    future.get();
    std::cout << "Store updatevalue succeeeed." << std::endl;

    std::cout << "Trying to find updatevalue with Id: " << updated.getId().toString() << std::endl;
    future1 = node2->findValue(updated.getId());
    foundValue = future1.get();
    CPPUNIT_ASSERT(foundValue);
    CPPUNIT_ASSERT(foundValue->isValid());
    std::cout << "updated value: " << foundValue->toString() << std::endl;
}

void NodeTests::testFindPeer() {
    std::cout << "-----Announce peer-----" << std::endl;
    std::cout << "Trying to announce peer " << std::endl;

    auto peer1 = PeerInfo::create(node1->getId(), 42244);
    auto future = node1->announcePeer(peer1);
    future.get();
    std::cout << "Announce peer1 succeeeed." << std::endl;

    auto peer2 = PeerInfo::create(node2->getId(), node1->getId(), 42245);
    future = node1->announcePeer(peer2);
    future.get();
    std::cout << "Announce peer2 succeeeed." << std::endl;

    std::cout << "-----Find peer-----" << std::endl;
    //The node2(announcePeer node) can't save the peer now (the same as Java), so new the node3 for test

    std::cout << "Trying to find peer1 with Id: " << peer1.getId().toString() << std::endl;
    auto future1 = node3->findPeer(peer1.getId(), 1);
    auto peers = future1.get();
    CPPUNIT_ASSERT(!peers.empty());
    for (const auto& peer: peers) {
        std::cout << "Peer1: " << peer.toString() << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Peer1 is invalid!", peer.isValid());
    }

    std::cout << "Trying to find peer2 with Id: " << peer2.getId().toString() << std::endl;
    future1 = node3->findPeer(peer2.getId(), 1);
    peers = future1.get();
    CPPUNIT_ASSERT(!peers.empty());
    for (const auto& peer: peers) {
        std::cout << "Peer2: " << peer.toString() << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Peer2 is invalid!", peer.isValid());
    }
}

}  // namespace test
