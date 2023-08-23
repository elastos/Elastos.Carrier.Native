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

#include <vector>
#include <chrono>
#include <thread>
#include <carrier.h>

#include "utils.h"
#include "peer_tests.h"

using namespace elastos::carrier;
using namespace std::chrono_literals;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(PeerTests);

void PeerTests::setUp() {
    auto path1 = Utils::getPwdStorage("node1");
    auto path2 = Utils::getPwdStorage("node2");
    auto path3 = Utils::getPwdStorage("node3");
    auto path4 = Utils::getPwdStorage("proxy");

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);
    Utils::removeStorage(path3);
    Utils::removeStorage(path4);

    auto ipAddress = Utils::getLocalIpAddresses();
    //create node1, node2, node3 and proxy node.
    auto b1 = DefaultConfiguration::Builder {};
    b1.setIPv4Address(ipAddress);
    b1.setListeningPort(42222);
    b1.setStoragePath(path1);

    node1 = std::make_shared<Node>(b1.build());
    node1->start();

    auto b2 = DefaultConfiguration::Builder {};
    b2.setIPv4Address(ipAddress);
    b2.setListeningPort(42223);
    b2.setStoragePath(path2);

    node2 = std::make_shared<Node>(b2.build());
    node2->start();

    auto b3 = DefaultConfiguration::Builder {};
    b3.setIPv4Address(ipAddress);
    b3.setListeningPort(42224);
    b3.setStoragePath(path3);

    node3 = std::make_shared<Node>(b3.build());
    node3->start();

    auto b4 = DefaultConfiguration::Builder {};
    b4.setIPv4Address(ipAddress);
    b4.setListeningPort(42532);
    b4.setStoragePath(path4);

    proxy = std::make_shared<Node>(b4.build());
    proxy->start();

    //boostrap
    auto ni1 = NodeInfo {node1->getId(), ipAddress, node1->getPort()};
    node2->bootstrap(ni1);
    node3->bootstrap(ni1);
}

void PeerTests::tearDown() {
    if (node1)
        node1->stop();
    if (node2)
        node2->stop();
    if (node3)
        node3->stop();
    if (proxy)
        proxy->stop();

    auto path1 = Utils::getPwdStorage("node1");
    auto path2 = Utils::getPwdStorage("node2");
    auto path3 = Utils::getPwdStorage("node3");
    auto path4 = Utils::getPwdStorage("proxy");

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);
    Utils::removeStorage(path3);
    Utils::removeStorage(path4);
}

void PeerTests::testPeer1() {
    auto alternativeURL = "https://testing.pc2.net";
    auto keypair = Signature::KeyPair::random();
    auto peer = PeerInfo::create(keypair, node1->getId(), node1->getId(), 65534, alternativeURL);

    CPPUNIT_ASSERT(peer.isValid());
    CPPUNIT_ASSERT(!peer.isDelegated());

    auto future = node1->announcePeer(peer, false);
    future.get();

    auto findFuture1 = node2->findPeer(peer.getId(), 1);
    auto peerList1 = findFuture1.get();
    CPPUNIT_ASSERT(peerList1.size() == 1);

    auto& foundPeer1 = peerList1.front();
    CPPUNIT_ASSERT(foundPeer1 == peer);
    CPPUNIT_ASSERT(foundPeer1.getId() == peer.getId());
    CPPUNIT_ASSERT(foundPeer1.getNodeId() == peer.getNodeId());
    CPPUNIT_ASSERT(foundPeer1.getOrigin() == peer.getOrigin());
    CPPUNIT_ASSERT(foundPeer1.getPort() == peer.getPort());
    CPPUNIT_ASSERT(foundPeer1.getAlternativeURL() == peer.getAlternativeURL());
    CPPUNIT_ASSERT(foundPeer1.getSignature() == peer.getSignature());
    CPPUNIT_ASSERT(foundPeer1.isValid());
    CPPUNIT_ASSERT(!foundPeer1.isDelegated());

    auto findFuture2 = node3->findPeer(peer.getId(), 1);
    auto peerList2 = findFuture2.get();
    CPPUNIT_ASSERT(peerList2.size() == 1);

    auto& foundPeer2 = peerList2.front();
    CPPUNIT_ASSERT(foundPeer2 == peer);
    CPPUNIT_ASSERT(foundPeer2.getId() == peer.getId());
    CPPUNIT_ASSERT(foundPeer2.getNodeId() == peer.getNodeId());
    CPPUNIT_ASSERT(foundPeer2.getOrigin() == peer.getOrigin());
    CPPUNIT_ASSERT(foundPeer2.getPort() == peer.getPort());
    CPPUNIT_ASSERT(foundPeer2.getAlternativeURL() == peer.getAlternativeURL());
    CPPUNIT_ASSERT(foundPeer2.getSignature() == peer.getSignature());
    CPPUNIT_ASSERT(foundPeer2.isValid());
    CPPUNIT_ASSERT(!foundPeer2.isDelegated());
}

void PeerTests::testPeer2() {
    auto keyPair1 = Signature::KeyPair::random();
    auto keyPair2 = Signature::KeyPair::random();
    auto peerId1 = Id(keyPair1.publicKey());
    auto peerId2 = Id(keyPair2.publicKey());
    auto proxyId = proxy->getId();

    std::vector<int> ports1 {12};
    for (int i = 0; i < 12; i++ )
        ports1.push_back(Utils::getRandom(40000, 45000));

    std::vector<int> ports2 {12};
    for (int i = 0; i < 12; i++)
        ports2.push_back(Utils::getRandom(40000, 45000));

    std::vector<Id> nodeIds {3};
    nodeIds.push_back(node1->getId());
    nodeIds.push_back(node2->getId());
    nodeIds.push_back(node3->getId());

    for (int i = 0; i < 4; i++) {
        int val = 3 * i;
        auto peer = PeerInfo::create(keyPair1, node1->getId(), ports1[val], std::to_string(val));
        auto future = node1->announcePeer(peer);
        future.get();

        val++;
        peer = PeerInfo::create(keyPair1, node2->getId(), ports1[val], std::to_string(val));
        future = node2->announcePeer(peer);
        future.get();

        val++;
        peer = PeerInfo::create(keyPair1, node3->getId(), ports1[val], std::to_string(val));
        future = node3->announcePeer(peer);
        future.get();

        val = 3 * i;
        auto data = Utils::getSignData(proxyId, node1->getId(), ports2[val], std::to_string(val));
        auto signature = keyPair2.privateKey().sign(data);
        auto pk = keyPair2.publicKey();
        CPPUNIT_ASSERT(Signature::verify(data, signature, pk));

        peer = PeerInfo::of(peerId2.blob(), keyPair2.privateKey().blob(), proxyId.blob(), node1->getId().blob(), ports2[val], std::to_string(val), signature);
        future = node1->announcePeer(peer);
        future.get();

        val++;
        data = Utils::getSignData(proxyId, node2->getId(), ports2[val], std::to_string(val));
        signature = keyPair2.privateKey().sign(data);
        CPPUNIT_ASSERT(Signature::verify(data, signature, pk));

        peer = PeerInfo::of(peerId2.blob(), keyPair2.privateKey().blob(), proxyId.blob(), node2->getId().blob(), ports2[val], std::to_string(val), signature);
        future = node2->announcePeer(peer);
        future.get();

        val++;
        data = Utils::getSignData(proxyId, node3->getId(), ports2[val], std::to_string(val));
        signature = keyPair2.privateKey().sign(data);
        CPPUNIT_ASSERT(Signature::verify(data, signature, pk));

        peer = PeerInfo::of(peerId2.blob(), keyPair2.privateKey().blob(), proxyId.blob(), node3->getId().blob(), ports2[val], std::to_string(val), signature);
        future = node3->announcePeer(peer);
        future.get();
    }

    std::this_thread::sleep_for(2000ms);

    auto future = node2->findPeer(peerId1, 3);
    auto peers = future.get();
    CPPUNIT_ASSERT(peers.size() == 3);
    for (const auto& peer: peers)
        CPPUNIT_ASSERT(peer.isValid());

    future = node3->findPeer(peerId1, 24);
    peers = future.get();
    CPPUNIT_ASSERT(peers.size() == 3);

    for (const auto& peer : peers) {
        CPPUNIT_ASSERT(peer.isValid());

        auto port = peer.getPort();
        auto node = peer.getNodeId();
        auto result1 = std::find(ports1.begin(), ports1.end(), port);
        CPPUNIT_ASSERT(result1 != ports1.end());

        auto result2 = std::find(nodeIds.begin(), nodeIds.end(), node);
        CPPUNIT_ASSERT(result2 != nodeIds.end());
    }

    //find peer2
    future = node1->findPeer(peerId2, 24);
    peers = future.get();
    CPPUNIT_ASSERT(peers.size() == 3);
    for (const auto& peer: peers)
        CPPUNIT_ASSERT(peer.isValid());

    future = node2->findPeer(peerId2, 4);
    peers = future.get();
    CPPUNIT_ASSERT(peers.size() == 3);

    for (const auto& peer : peers) {
        CPPUNIT_ASSERT(peer.isValid());

        auto port = peer.getPort();
        auto nodeId = peer.getNodeId();
        auto origin = peer.getOrigin();

        auto result1 = std::find(ports2.begin(), ports2.end(), port);
        CPPUNIT_ASSERT(result1 != ports2.end());
        CPPUNIT_ASSERT(nodeId == proxyId);

        auto result2 = std::find(nodeIds.begin(), nodeIds.end(), origin);
        CPPUNIT_ASSERT(result2 != nodeIds.end());
    }
}

}  // namespace test
