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

#include "messages/message.h"
#include "messages/find_peer_request.h"
#include "messages/find_peer_response.h"

#include "utils.h"
#include "find_peer_tests.h"

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(FindPeerTests);

void FindPeerTests::setUp() {
}

void FindPeerTests::testFindPeerRequestSize() {
    auto msg = FindPeerRequest(Id::random());
    msg.setId(Id::random());
    msg.setTxid(0x87654321);
    msg.setVersion(VERSION);
    msg.setWant4(true);
    msg.setWant6(true);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
}

void FindPeerTests::testFindPeerRequest4() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomValue();

    auto msg = FindPeerRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setVersion(VERSION);
    msg.setWant4(true);
    msg.setWant6(false);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindPeerRequest>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::REQUEST, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_PEER, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(nodeId, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(target, _msg->getTarget());
    CPPUNIT_ASSERT(_msg->doesWant4());
    CPPUNIT_ASSERT(!_msg->doesWant6());
    CPPUNIT_ASSERT_EQUAL(VERSION_STR, _msg->getReadableVersion());
}

void FindPeerTests::testFindPeerRequest6() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomValue();

    auto msg = FindPeerRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(false);
    msg.setWant6(true);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindPeerRequest>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::REQUEST, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_PEER, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(nodeId, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(target, _msg->getTarget());
    CPPUNIT_ASSERT(!_msg->doesWant4());
    CPPUNIT_ASSERT(_msg->doesWant6());
}

void FindPeerTests::testFindPeerRequest46() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomValue();

    auto msg = FindPeerRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(true);
    msg.setWant6(true);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindPeerRequest>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::REQUEST, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_PEER, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(nodeId, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(target, _msg->getTarget());
    CPPUNIT_ASSERT(_msg->doesWant4());
    CPPUNIT_ASSERT(_msg->doesWant6());
}

void FindPeerTests::testFindPeerResponseSize() {
    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65534));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65533));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65532));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65531));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65530));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65529));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65528));

    std::list<std::shared_ptr<NodeInfo>> nodes6 {};
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65535));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65534));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65533));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65532));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65531));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65530));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65529));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65528));


    std::list<PeerInfo> peers {};
    std::vector<uint8_t> sig(64);
    Id pid = Id::random();
    for (int i = 0; i < 8; i++) {
        Random::buffer(sig.data(), sig.size());
        peers.push_back(PeerInfo::of(pid, Id::random(), 65535 - i, sig));
    }

    auto msg = FindPeerResponse(0xF7654321);
    msg.setId(Id::random());
    msg.setVersion(VERSION);
    msg.setNodes4(nodes4);
    msg.setNodes6(nodes6);
    msg.setToken(0x87654321);
    msg.setPeers(peers);

    auto serialized = msg.serialize();
    printMessage(msg, serialized);
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
}

void FindPeerTests::testFindPeerResponseSize2() {
    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65534));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65533));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65532));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65531));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65530));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65529));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65528));

    std::list<std::shared_ptr<NodeInfo>> nodes6 {};
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65535));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65534));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65533));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65532));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65531));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65530));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65529));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", 65528));


    std::list<PeerInfo> peers {};
    std::vector<uint8_t> sig(64);
    Id pid = Id::random();
    for (int i = 0; i < 8; i++) {
        Random::buffer(sig.data(), sig.size());
        peers.push_back(PeerInfo::of(pid, Id::random(), Id::random(), 65535 - i, "http://abc.pc2.net", sig));
    }

    auto msg = FindPeerResponse(0xF7654321);
    msg.setId(Id::random());
    msg.setVersion(VERSION);
    msg.setNodes4(nodes4);
    msg.setNodes6(nodes6);
    msg.setToken(0x87654321);
    msg.setPeers(peers);

    auto serialized = msg.serialize();
    printMessage(msg, serialized);
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
}

void FindPeerTests::testFindPeerResponse4() {
    auto id = Id::random();
    int txid = Utils::getRandomValue();
    int token = Utils::getRandomValue();

    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.2", 1232));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.3", 1233));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.4", 1234));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.5", 1235));

    std::list<PeerInfo> peers {};
    std::vector<uint8_t> sig(64);
    Id pid = Id::random();
    for (int i = 0; i < 8; i++) {
        Random::buffer(sig.data(), sig.size());
        peers.push_back(PeerInfo::of(pid, Id::random(), 65535 - i, sig));
    }

    auto msg = FindPeerResponse(txid);
    msg.setId(id);
    msg.setVersion(VERSION);
    msg.setNodes4(nodes4);
    msg.setToken(token);
    msg.setPeers(peers);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindPeerResponse>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::RESPONSE, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_PEER, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(id, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(VERSION_STR, _msg->getReadableVersion());
    CPPUNIT_ASSERT_EQUAL(token, _msg->getToken());
    CPPUNIT_ASSERT(_msg->getNodes6().empty());
    CPPUNIT_ASSERT(!_msg->getNodes4().empty());
    CPPUNIT_ASSERT(!_msg->getPeers().empty());

    auto nodes = _msg->getNodes4();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes4, nodes));

    CPPUNIT_ASSERT(peers == _msg->getPeers());
}

void FindPeerTests::testFindPeerResponse6() {
    auto id = Id::random();
    int txid = Utils::getRandomValue();
    int token = Utils::getRandomValue();

    std::list<std::shared_ptr<NodeInfo>> nodes6 {};
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:8070:6543:8a2e:0370:7334", 65535));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7332", 1232));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7333", 1233));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7334", 1234));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7335", 1235));

    std::list<PeerInfo> peers {};
    std::vector<uint8_t> sig(64);
    Id pid = Id::random();
    for (int i = 0; i < 8; i++) {
        Random::buffer(sig.data(), sig.size());
        peers.push_back(PeerInfo::of(pid, Id::random(), Id::random(), 65535 - i, "http://abc.pc2.net", sig));
    }

    auto msg = FindPeerResponse(txid);
    msg.setId(id);
    msg.setVersion(VERSION);
    msg.setNodes6(nodes6);
    msg.setToken(token);
    msg.setPeers(peers);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindPeerResponse>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::RESPONSE, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_PEER, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(id, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(VERSION_STR, _msg->getReadableVersion());
    CPPUNIT_ASSERT_EQUAL(token, _msg->getToken());
    CPPUNIT_ASSERT(_msg->getNodes4().empty());
    CPPUNIT_ASSERT(!_msg->getNodes6().empty());
    CPPUNIT_ASSERT(!_msg->getPeers().empty());

    auto nodes = _msg->getNodes6();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes6, nodes));

    CPPUNIT_ASSERT(peers == _msg->getPeers());
}

void FindPeerTests::testFindPeerResponse46() {
    auto id = Id::random();
    int txid = Utils::getRandomValue();
    int token = Utils::getRandomValue();

    std::list<std::shared_ptr<NodeInfo>> nodes4 = {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.2", 1232));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.3", 1233));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.4", 1234));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.5", 1235));

    std::list<std::shared_ptr<NodeInfo>> nodes6 = {};
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:8070:6543:8a2e:0370:7334", 65535));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7332", 1232));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7333", 1233));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7334", 1234));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7335", 1235));

    std::list<PeerInfo> peers {};
    std::vector<uint8_t> sig(64);
    Id pid = Id::random();

    for (int i = 0; i < 4; i++) {
        Random::buffer(sig.data(), sig.size());
        peers.push_back(PeerInfo::of(pid, Id::random(), Id::random(), 65535 - i, sig));
    }

    for (int i = 0; i < 4; i++) {
        Random::buffer(sig.data(), sig.size());
        peers.push_back(PeerInfo::of(pid, Id::random(), Id::random(), 65535 - i, "http://abc.pc2.net", sig));
    }

    auto msg = FindPeerResponse(txid);
    msg.setId(id);
    msg.setNodes4(nodes4);
    msg.setNodes6(nodes6);
    msg.setToken(token);
    msg.setPeers(peers);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindPeerResponse>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::RESPONSE, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_PEER, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(id, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(0, _msg->getVersion());
    CPPUNIT_ASSERT_EQUAL(token, _msg->getToken());
    CPPUNIT_ASSERT(!_msg->getNodes4().empty());
    CPPUNIT_ASSERT(!_msg->getNodes6().empty());
    CPPUNIT_ASSERT(!_msg->getPeers().empty());

    auto nodes = _msg->getNodes4();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes4, nodes));

    nodes = _msg->getNodes6();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes6, nodes));

    CPPUNIT_ASSERT(peers == _msg->getPeers());
}

void FindPeerTests::tearDown() {
}
}

