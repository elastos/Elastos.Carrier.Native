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

#include <carrier.h>
#include "utils.h"
#include "peerinfo_announce_tests.h"

using namespace elastos::carrier;

namespace test {

CPPUNIT_TEST_SUITE_REGISTRATION(PeerInfoAnnounceTests);

void PeerInfoAnnounceTests::setUp() {
    auto path1 = Utils::getPwdStorage("node1");
    auto path2 = Utils::getPwdStorage("node2");
    auto path3 = Utils::getPwdStorage("node3");

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);
    Utils::removeStorage(path3);

    //create and runnode1
    auto b1 = DefaultConfiguration::Builder {};
    auto ipAddresses = Utils::getLocalIpAddresses();

    b1.setIPv4Address(ipAddresses);
    b1.setListeningPort(32222);
    b1.setStoragePath(path1);

    node1 = std::make_shared<Node>(b1.build());
    node1->start();

    auto nodeInfo = NodeInfo {node1->getId(), Utils::getLocalIpAddresses(), node1->getPort()};

    // create and run node2.
    auto b2 = DefaultConfiguration::Builder {};
    b2.setIPv4Address(ipAddresses);
    b2.setListeningPort(32224);
    b2.setStoragePath(path2);

    node2 = std::make_shared<Node>(b2.build());
    node2->start();
    node2->bootstrap(nodeInfo);

    // create and run node2.
    auto b3 = DefaultConfiguration::Builder {};
    b3.setIPv4Address(ipAddresses);
    b3.setListeningPort(32226);
    b3.setStoragePath(path3);

    node3 = std::make_shared<Node>(b3.build());
    node3->start();
    node3->bootstrap(nodeInfo);
}

void PeerInfoAnnounceTests::tearDown() {
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

void PeerInfoAnnounceTests::testAnnounceAndFind() {
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

}// namespace test
