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

    auto remoteId = javaId;

#if 1
    std::cout << "-----Find node-----" << std::endl;
    std::cout << "Trying to find Node " << remoteId.toBase58String() << std::endl;
    auto future = node1->findNode(remoteId);
    auto ni2 = future.get();
    std::cout << "ni2 size: " << ni2.size() << std::endl;
    CPPUNIT_ASSERT_MESSAGE("Node not found!", ni2.size());
    for (auto ni: ni2) {
        std::cout << "ni: " << static_cast<std::string>(*ni) << std::endl;
    }
#endif

#if 1
    std::cout << "-----Store value-----" << std::endl;
    std::vector<uint8_t> data({0, 1, 2, 3, 4});
    auto value = Value::createValue(data);
    std::cout << "Trying to Sotre Value " << std::endl;
    auto future1 = node1->storeValue(value);
    future1.get();
    std::cout << "Store value succeeeed." << std::endl;

	auto signedValue = Value::createSignedValue(data);
    future1 = node1->storeValue(signedValue);
    future1.get();
    std::cout << "Store signedValue succeeeed." << std::endl;

    auto recipientId = remoteId;
    auto encryptedValue = Value::createEncryptedValue(recipientId, data);
    future1 = node1->storeValue(encryptedValue);
    future1.get();
    std::cout << "Store encryptedValue succeeeed." << std::endl;
#endif

#if 1
    std::cout << "-----Find value-----" << std::endl;
    std::cout << "Trying to find Value with Id: " << value.getId().toBase58String() << std::endl;
    auto future2 = node1->findValue(value.getId());
    auto val = future2.get();
    CPPUNIT_ASSERT_MESSAGE("Value not found!", val != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Value is invalid!", val->isValid());
    std::cout << "Value: " << static_cast<std::string>(*val) << std::endl;

    std::cout << "Trying to find SignedValue with Id: " << signedValue.getId().toBase58String() << std::endl;
    future2 = node1->findValue(signedValue.getId());
    val = future2.get();
    CPPUNIT_ASSERT_MESSAGE("SignedValue not found!", val != nullptr);
    CPPUNIT_ASSERT_MESSAGE("SignedValue is invalid!", val->isValid());
    std::cout << "SignedValue: " << static_cast<std::string>(*val) << std::endl;

    std::cout << "Trying to find EncryptedValue with Id: " << encryptedValue.getId().toBase58String() << std::endl;
    future2 = node1->findValue(encryptedValue.getId());
    val = future2.get();
    CPPUNIT_ASSERT_MESSAGE("EncryptedValue not found!", val != nullptr);
    CPPUNIT_ASSERT_MESSAGE("EncryptedValue is invalid!", val->isValid());
    std::cout << "EncryptedValue: " << static_cast<std::string>(*val) << std::endl;
#endif

#if 1
    std::cout << "-----Update value-----" << std::endl;
    std::cout << "Trying to update signedValue with Id: " << signedValue.getId().toBase58String() << std::endl;
    std::vector<uint8_t> data1({5, 6, 7, 8});
    auto updatevalue = signedValue.update(data1);
    future1 = node1->storeValue(updatevalue);
    future1.get();
    std::cout << "Store updatevalue succeeeed." << std::endl;

    std::cout << "Trying to find updatevalue with Id: " << updatevalue.getId().toBase58String() << std::endl;
    future2 = node1->findValue(updatevalue.getId());
    val = future2.get();
    CPPUNIT_ASSERT_MESSAGE("updatevalue not found!", val != nullptr);
    CPPUNIT_ASSERT_MESSAGE("updatevalue is invalid!", val->isValid());
    std::cout << "updatevalue: " << static_cast<std::string>(*val) << std::endl;
#endif

#if 1
    std::cout << "-----Announce peer-----" << std::endl;
    std::cout << "Trying to announce peer " << std::endl;

    auto peer = PeerInfo::create(node1->getId(), 42244);
    auto future3 = node1->announcePeer(peer);
    future3.get();
    std::cout << "Announce peer1 succeeeed." << std::endl;

    Id nodeId = remoteId;
    auto peer2 = PeerInfo::create(nodeId, node1->getId(), 42245);
    future3 = node1->announcePeer(peer2);
    future3.get();
    std::cout << "Announce peer2 succeeeed." << std::endl;
#endif

#if 1
    std::cout << "-----Find peer-----" << std::endl;

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

    std::cout << "Trying to find peer1 with Id: " << peer.getId().toBase58String() << std::endl;
    auto future4 = node3->findPeer(peer.getId(), 1);
    auto peers = future4.get();
    CPPUNIT_ASSERT_MESSAGE("Peer1 not found!", !peers.empty());
    for (auto& peer: peers) {
        std::cout << "Peer1: " << static_cast<std::string>(peer) << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Peer1 is invalid!", peer.isValid());
    }

    std::cout << "Trying to find peer2 with Id: " << peer2.getId().toBase58String() << std::endl;
    future4 = node3->findPeer(peer2.getId(), 1);
    peers = future4.get();
    CPPUNIT_ASSERT_MESSAGE("Peer2 not found!", !peers.empty());
    for (auto& peer: peers) {
        std::cout << "Peer2: " << static_cast<std::string>(peer) << std::endl;
        CPPUNIT_ASSERT_MESSAGE("Peer2 is invalid!", peer.isValid());
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
