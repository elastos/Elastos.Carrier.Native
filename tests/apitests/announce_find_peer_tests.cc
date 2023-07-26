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

#include <list>
#include <iostream>
#include <array>
#include <carrier.h>
#include <chrono>
#include <thread>
#include <limits.h>

#include "utils.h"
#include "utils/misc.h"
#include "announce_find_peer_tests.h"

using namespace elastos::carrier;
using namespace std::chrono_literals;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(AnnounceFindPeerTests);

void
AnnounceFindPeerTests::setUp() {
    std::string path1 = Utils::getPwdStorage("carrier1");
    std::string path2 = Utils::getPwdStorage("carrier2");
    std::string path3 = Utils::getPwdStorage("carrier3");
    std::string path4 = Utils::getPwdStorage("proxy");

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);
    Utils::removeStorage(path3);
    Utils::removeStorage(path4);

    //create node1, node2 and node3
    auto b1 = DefaultConfiguration::Builder {};

    auto ipAddresses = Utils::getLocalIpAddresses();
    b1.setIPv4Address(ipAddresses);
    b1.setListeningPort(42222);
    b1.setStoragePath(path1);

    node1 = std::make_shared<Node>(b1.build());
    node1->start();

    auto b2 = DefaultConfiguration::Builder {};
    b2.setIPv4Address(ipAddresses);
    b2.setListeningPort(42223);
    b2.setStoragePath(path2);

    node2 = std::make_shared<Node>(b2.build());
    node2->start();

    auto b3 = DefaultConfiguration::Builder {};
    b3.setIPv4Address(ipAddresses);
    b3.setListeningPort(42224);
    b3.setStoragePath(path3);

    node3 = std::make_shared<Node>(b3.build());
    node3->start();

    //boostrap
    auto ni1 = NodeInfo {node1->getId(), ipAddresses, node1->getPort()};
    node2->bootstrap(ni1);
    node3->bootstrap(ni1);

    auto b4 = DefaultConfiguration::Builder {};
    b4.setIPv4Address(ipAddresses);
    b4.setListeningPort(42532);
    b4.setStoragePath(path4);

    proxy = std::make_shared<Node>(b4.build());
    proxy->start();
}

void
AnnounceFindPeerTests::testPeer() {
    Signature::KeyPair keyPair1 = Signature::KeyPair::random();
    Signature::KeyPair keyPair2 = Signature::KeyPair::random();
    Id peerId1 = Id(keyPair1.publicKey());
    Id peerId2 = Id(keyPair2.publicKey());

    auto proxyId = proxy->getId();

    std::vector<int> ports1;
    for (int i = 0; i < 12; i++ )
        ports1.push_back(Utils::getRandom(40000, 45000));

    std::vector<int> ports2;
    for (int i = 0; i < 12; i++)
        ports2.push_back(Utils::getRandom(40000, 45000));

    std::vector<Id> nodes;
    nodes.push_back(node1->getId());
    nodes.push_back(node2->getId());
    nodes.push_back(node3->getId());

    for (int i = 0; i < 4; i++) {
        int val = 3 * i;
        auto peer = PeerInfo::create(keyPair1, node1->getId().blob(), ports1[val], std::to_string(val));
        auto future1 = node1->announcePeer(peer);
        future1.get();

        val++;
        peer = PeerInfo::create(keyPair1, node2->getId().blob(), ports1[val], std::to_string(val));
        auto future2 = node2->announcePeer(peer);
        future2.get();

        val++;
        peer = PeerInfo::create(keyPair1, node3->getId().blob(), ports1[val], std::to_string(val));
        auto future3 = node3->announcePeer(peer);
        future3.get();

        val = 3 * i;
        auto data1 = Utils::getSignData(proxyId, node1->getId(), ports2[val], std::to_string(val));
        auto signature1 = keyPair2.privateKey().sign(data1);

        Signature::PublicKey pk = keyPair2.publicKey();
        CPPUNIT_ASSERT(Signature::verify(data1, signature1, pk));

        peer = PeerInfo::of(peerId2.blob(), keyPair2.privateKey().blob(), proxyId.blob(), node1->getId().blob(), ports2[val], std::to_string(val), signature1);
        auto future4 = node1->announcePeer(peer);
        future4.get();

        val++;
        auto data2 = Utils::getSignData(proxyId, node2->getId(), ports2[val], std::to_string(val));
        auto signature2 = keyPair2.privateKey().sign(data2);
        peer = PeerInfo::of(peerId2.blob(), keyPair2.privateKey().blob(), proxyId.blob(), node2->getId().blob(), ports2[val], std::to_string(val), signature2);
        auto future5 = node2->announcePeer(peer);
        future5.get();

        val++;
        auto data3 = Utils::getSignData(proxyId, node3->getId(), ports2[val], std::to_string(val));
        auto signature3 = keyPair2.privateKey().sign(data3);
        peer = PeerInfo::of(peerId2.blob(), keyPair2.privateKey().blob(), proxyId.blob(), node3->getId().blob(), ports2[val], std::to_string(val), signature3);
        auto future6 = node3->announcePeer(peer);
        future6.get();
    }

    std::this_thread::sleep_for(2000ms);

    auto future7 = node2->findPeer(peerId1, 3);
    auto peers7 = future7.get();
    CPPUNIT_ASSERT(3 == peers7.size());
    for (auto& peer: peers7)
        CPPUNIT_ASSERT(peer.isValid());

    auto future8 = node3->findPeer(peerId1, 24);
    auto peers8 = future8.get();
    CPPUNIT_ASSERT(3 == peers8.size());

    for (auto &peer : peers8) {
        CPPUNIT_ASSERT(peer.isValid());

        auto port = peer.getPort();
        auto node = peer.getNodeId();
        auto result7 = std::find(ports1.begin(), ports1.end(), port);
        CPPUNIT_ASSERT(result7 != ports1.end());

        auto result8 = std::find(nodes.begin(), nodes.end(), node);
        CPPUNIT_ASSERT(result8 != nodes.end());
    }

    //find peer2
    auto future9 = node1->findPeer(peerId2, 24);
    auto peers9 = future9.get();
    CPPUNIT_ASSERT(3 == peers9.size());
    for (auto& peer: peers9)
        CPPUNIT_ASSERT(peer.isValid());

    auto future10 = node2->findPeer(peerId2, 4);
    auto peers10 = future10.get();
    CPPUNIT_ASSERT(3 == peers10.size());

    for (auto &peer : peers10) {
        CPPUNIT_ASSERT(peer.isValid());

        auto port = peer.getPort();
        auto node = peer.getNodeId();
        auto result9 = std::find(ports2.begin(), ports2.end(), port);
        CPPUNIT_ASSERT(result9 != ports2.end());

        auto result10 = std::find(nodes.begin(), nodes.end(), node);
        CPPUNIT_ASSERT(result10 != nodes.end());
    }
}

void
AnnounceFindPeerTests::tearDown() {
    if (node1)
        node1->stop();
    if (node2)
        node2->stop();
    if (node3)
        node3->stop();

    std::string path1 = Utils::getPwdStorage("carrier1");
    std::string path2 = Utils::getPwdStorage("carrier2");
    std::string path3 = Utils::getPwdStorage("carrier3");

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);
    Utils::removeStorage(path3);
}

}  // namespace test
