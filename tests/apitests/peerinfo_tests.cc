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

#include <string>
#include <carrier.h>
#include "peerinfo_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(PeerInfoTests);

void PeerInfoTests::testFalsity() {
    auto peerId = Id::random();
    auto nodeId = Id::random();
    auto origin = Id::random();
    uint16_t port = 65535;
    std::vector<uint8_t> fakeSig(64, 1);
    std::string alternativeURL = "https://testing.pc2.net";
    auto peer = PeerInfo::of(peerId.blob(), {}, nodeId.blob(), origin.blob(), port, alternativeURL, fakeSig);

    CPPUNIT_ASSERT(peer.getId() == peerId);
    CPPUNIT_ASSERT(peer.getNodeId() == nodeId);
    CPPUNIT_ASSERT(peer.getOrigin() == origin);
    CPPUNIT_ASSERT(peer.getPort() == port);
    CPPUNIT_ASSERT(peer.hasAlternativeURL());
    CPPUNIT_ASSERT(peer.getAlternativeURL() == alternativeURL);
    CPPUNIT_ASSERT(peer.getSignature() == fakeSig);
    CPPUNIT_ASSERT(peer.isDelegated());
    CPPUNIT_ASSERT(!peer.hasPrivateKey());
    CPPUNIT_ASSERT(!peer.isValid());
}

void PeerInfoTests::testCreate1() {
    uint16_t port = 65534;
    auto nodeId = Id::random();
    auto peer = PeerInfo::create(nodeId, port);

    CPPUNIT_ASSERT(peer.getNodeId() == nodeId);
    CPPUNIT_ASSERT(peer.getOrigin() == nodeId);
    CPPUNIT_ASSERT(peer.getPort() == port);
    CPPUNIT_ASSERT(peer.hasPrivateKey());
    CPPUNIT_ASSERT(!peer.hasAlternativeURL());
    CPPUNIT_ASSERT(!peer.isDelegated());
    CPPUNIT_ASSERT(peer.getSignature().size() == 64);
    CPPUNIT_ASSERT(peer.isValid());
}

void PeerInfoTests::testCreate2() {
    uint16_t port = 65534;
    auto nodeId = Id::random();
    auto keypair = Signature::KeyPair::random();
    auto peer = PeerInfo::create(keypair, nodeId, port);

    CPPUNIT_ASSERT(peer.getId() == keypair.publicKey());
    CPPUNIT_ASSERT(peer.hasPrivateKey());
    CPPUNIT_ASSERT(peer.getPrivateKey() == keypair.privateKey());
    CPPUNIT_ASSERT(peer.getNodeId() == nodeId);
    CPPUNIT_ASSERT(peer.getOrigin() == nodeId);
    CPPUNIT_ASSERT(peer.getPort() == port);
    CPPUNIT_ASSERT(!peer.hasAlternativeURL());
    CPPUNIT_ASSERT(!peer.isDelegated());
    CPPUNIT_ASSERT(peer.getSignature().size() == 64);
    CPPUNIT_ASSERT(peer.isValid());
}

void PeerInfoTests::testCreate3() {
    uint16_t port = 65534;
    auto nodeId = Id::random();
    auto origin = Id::random();
    auto peer = PeerInfo::create(nodeId, origin, port);

    CPPUNIT_ASSERT(peer.getNodeId() == nodeId);
    CPPUNIT_ASSERT(peer.getOrigin() == origin);
    CPPUNIT_ASSERT(peer.getPort() == port);
    CPPUNIT_ASSERT(peer.hasPrivateKey());
    CPPUNIT_ASSERT(!peer.hasAlternativeURL());
    CPPUNIT_ASSERT(peer.isDelegated());
    CPPUNIT_ASSERT(peer.getSignature().size() == 64);
    CPPUNIT_ASSERT(peer.isValid());
}

void PeerInfoTests::testCreate4() {
    uint16_t port = 65534;
    auto nodeId = Id::random();
    auto origin = Id::random();
    auto keypair = Signature::KeyPair::random();
    auto peer = PeerInfo::create(keypair, nodeId, origin, port);

    CPPUNIT_ASSERT(peer.getId() == keypair.publicKey());
    CPPUNIT_ASSERT(peer.hasPrivateKey());
    CPPUNIT_ASSERT(peer.getPrivateKey() == keypair.privateKey());
    CPPUNIT_ASSERT(peer.getNodeId() == nodeId);
    CPPUNIT_ASSERT(peer.getOrigin() == origin);
    CPPUNIT_ASSERT(peer.getPort() == port);
    CPPUNIT_ASSERT(!peer.hasAlternativeURL());
    CPPUNIT_ASSERT(peer.isDelegated());
    CPPUNIT_ASSERT(peer.getSignature().size() == 64);
    CPPUNIT_ASSERT(peer.isValid());
}

void PeerInfoTests::testCreate5() {
    uint16_t port = 65534;
    auto nodeId = Id::random();
    auto alternativeURL = "https://testing.pc2.net";
    auto peer = PeerInfo::create(nodeId, port, alternativeURL);

    CPPUNIT_ASSERT(peer.getNodeId() == nodeId);
    CPPUNIT_ASSERT(peer.getOrigin() == nodeId);
    CPPUNIT_ASSERT(peer.getPort() == port);
    CPPUNIT_ASSERT(peer.hasPrivateKey());
    CPPUNIT_ASSERT(peer.hasAlternativeURL());
    CPPUNIT_ASSERT(peer.getAlternativeURL() == alternativeURL);
    CPPUNIT_ASSERT(!peer.isDelegated());
    CPPUNIT_ASSERT(peer.getSignature().size() == 64);
    CPPUNIT_ASSERT(peer.isValid());
}

void PeerInfoTests::testCreate6() {
    uint16_t port = 65534;
    auto nodeId = Id::random();
    auto alternativeURL = "https://testing.pc2.net";
    auto keypair = Signature::KeyPair::random();
    auto peer = PeerInfo::create(keypair, nodeId, port, alternativeURL);

    CPPUNIT_ASSERT(peer.getId() == keypair.publicKey());
    CPPUNIT_ASSERT(peer.hasPrivateKey());
    CPPUNIT_ASSERT(peer.getPrivateKey() == keypair.privateKey());
    CPPUNIT_ASSERT(peer.getNodeId() == nodeId);
    CPPUNIT_ASSERT(peer.getOrigin() == nodeId);
    CPPUNIT_ASSERT(peer.getPort() == port);
    CPPUNIT_ASSERT(peer.hasAlternativeURL());
    CPPUNIT_ASSERT(peer.getAlternativeURL() == alternativeURL);
    CPPUNIT_ASSERT(!peer.isDelegated());
    CPPUNIT_ASSERT(peer.getSignature().size() == 64);
    CPPUNIT_ASSERT(peer.isValid());
}

void PeerInfoTests::testCreate7() {
    uint16_t port = 65534;
    auto nodeId = Id::random();
    auto origin = Id::random();
    auto alternativeURL = "https://testing.pc2.net";
    auto peer = PeerInfo::create(nodeId, origin, port, alternativeURL);

    CPPUNIT_ASSERT(peer.getNodeId() == nodeId);
    CPPUNIT_ASSERT(peer.getOrigin() == origin);
    CPPUNIT_ASSERT(peer.getPort() == port);
    CPPUNIT_ASSERT(peer.hasPrivateKey());
    CPPUNIT_ASSERT(peer.hasAlternativeURL());
    CPPUNIT_ASSERT(peer.getAlternativeURL() == alternativeURL);
    CPPUNIT_ASSERT(peer.isDelegated());
    CPPUNIT_ASSERT(peer.getSignature().size() == 64);
    CPPUNIT_ASSERT(peer.isValid());
}

void PeerInfoTests::testCreate8() {
    uint16_t port = 65534;
    auto nodeId = Id::random();
    auto origin = Id::random();
    auto alternativeURL = "https://testing.pc2.net";
    auto keypair = Signature::KeyPair::random();
    auto peer = PeerInfo::create(keypair, nodeId, origin, port, alternativeURL);

    CPPUNIT_ASSERT(peer.getId() == keypair.publicKey());
    CPPUNIT_ASSERT(peer.hasPrivateKey());
    CPPUNIT_ASSERT(peer.getPrivateKey() == keypair.privateKey());
    CPPUNIT_ASSERT(peer.getNodeId() == nodeId);
    CPPUNIT_ASSERT(peer.getOrigin() == origin);
    CPPUNIT_ASSERT(peer.getPort() == port);
    CPPUNIT_ASSERT(peer.hasAlternativeURL());
    CPPUNIT_ASSERT(peer.getAlternativeURL() == alternativeURL);
    CPPUNIT_ASSERT(peer.isDelegated());
    CPPUNIT_ASSERT(peer.getSignature().size() == 64);
    CPPUNIT_ASSERT(peer.isValid());
}

void PeerInfoTests::testEqualOperator() {
    uint16_t port = 65534;
    auto nodeId = Id::random();
    auto origin = Id::random();
    auto alternativeURL = "https://testing.pc2.net";
    auto keypair = Signature::KeyPair::random();
    auto peer1 = PeerInfo::create(keypair, nodeId, origin, port, alternativeURL);
    auto peer2 = PeerInfo::create(keypair, nodeId, origin, port, alternativeURL);
    auto peer3 = PeerInfo::create(keypair, nodeId, port, alternativeURL);
    auto peer4 = PeerInfo::create(nodeId, port);

    CPPUNIT_ASSERT(peer1 == peer2);
    CPPUNIT_ASSERT(peer1 != peer3);
    CPPUNIT_ASSERT(peer1 != peer4);
}

}// namespace test
