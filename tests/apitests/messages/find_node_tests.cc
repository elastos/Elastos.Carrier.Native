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

#include "carrier/id.h"
#include "messages/message.h"
#include "messages/find_node_request.h"
#include "messages/find_node_response.h"

#include "utils.h"
#include "find_node_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(FindNodeTests);

void FindNodeTests::setUp() {
}

void FindNodeTests::testFindNodeRequestSize() {
    auto msg = FindNodeRequest(Id::random());
    msg.setId(Id::random());
    msg.setTxid(0x87654321);
    msg.setWant4(true);
    msg.setWant6(true);
    msg.setWantToken(true);
    msg.setVersion(VERSION);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
}

void FindNodeTests::testFindNodeRequest4() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = FindNodeRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(true);
    msg.setWant6(false);
    msg.setVersion(VERSION);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindNodeRequest>(parsed);

    CPPUNIT_ASSERT(Message::Type::REQUEST == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(nodeId ==  _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(target == _msg->getTarget());
    CPPUNIT_ASSERT(_msg->doesWant4());
    CPPUNIT_ASSERT(!_msg->doesWant6());
    CPPUNIT_ASSERT(!_msg->doesWantToken());
    CPPUNIT_ASSERT(VERSION_STR == _msg->getReadableVersion());
}

void FindNodeTests::testFindNodeRequest4WithAt() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = FindNodeRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(true);
    msg.setWant6(false);
    msg.setWantToken(true);
    msg.setVersion(VERSION);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindNodeRequest>(parsed);

    CPPUNIT_ASSERT(Message::Type::REQUEST == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(nodeId ==  _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(target == _msg->getTarget());
    CPPUNIT_ASSERT(_msg->doesWant4());
    CPPUNIT_ASSERT(!_msg->doesWant6());
    CPPUNIT_ASSERT(_msg->doesWantToken());
    CPPUNIT_ASSERT(VERSION_STR == _msg->getReadableVersion());
}

void FindNodeTests::testFindNodeRequest6() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = FindNodeRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(false);
    msg.setWant6(true);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindNodeRequest>(parsed);

    CPPUNIT_ASSERT(Message::Type::REQUEST == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(nodeId == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(target == _msg->getTarget());
    CPPUNIT_ASSERT(!_msg->doesWant4());
    CPPUNIT_ASSERT(_msg->doesWant6());
    CPPUNIT_ASSERT(!_msg->doesWantToken());
}

void
FindNodeTests::testFindNodeRequest6WithAt() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = FindNodeRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(false);
    msg.setWant6(true);
    msg.setWantToken(true);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindNodeRequest>(parsed);

    CPPUNIT_ASSERT(Message::Type::REQUEST == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(nodeId == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(target == _msg->getTarget());
    CPPUNIT_ASSERT(!_msg->doesWant4());
    CPPUNIT_ASSERT(_msg->doesWant6());
    CPPUNIT_ASSERT(_msg->doesWantToken());
}

void FindNodeTests::testFindNodeRequest46() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = FindNodeRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(true);
    msg.setWant6(true);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindNodeRequest>(parsed);

    CPPUNIT_ASSERT(Message::Type::REQUEST == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(nodeId == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(target == _msg->getTarget());
    CPPUNIT_ASSERT(_msg->doesWant4());
    CPPUNIT_ASSERT(_msg->doesWant6());
    CPPUNIT_ASSERT(!_msg->doesWantToken());
}

void FindNodeTests::testFindNodeRequest46WithAt() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = FindNodeRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(true);
    msg.setWant6(true);
    msg.setWantToken(true);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindNodeRequest>(parsed);

    CPPUNIT_ASSERT(Message::Type::REQUEST == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(nodeId == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(target == _msg->getTarget());
    CPPUNIT_ASSERT(_msg->doesWant4());
    CPPUNIT_ASSERT(_msg->doesWant6());
    CPPUNIT_ASSERT(_msg->doesWantToken());
}

void FindNodeTests::testFindNodeResponseSize() {
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

    auto msg = FindNodeResponse(0xF7654321);
    msg.setId(Id::random());
    msg.setVersion(VERSION);
    msg.setNodes4(nodes4);
    msg.setNodes6(nodes6);
    msg.setToken(0x78901234);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
}

void
FindNodeTests::testFindNodeResponse4() {
    auto id = Id::random();
    int txid = Utils::getRandomInteger(62);

    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.2", 1232));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.3", 1233));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.4", 1234));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.5", 1235));

    auto msg = FindNodeResponse(txid);
    msg.setId(id);
    msg.setVersion(VERSION);
    msg.setNodes4(nodes4);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindNodeResponse>(parsed);

    CPPUNIT_ASSERT(Message::Type::RESPONSE == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(id == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(VERSION_STR == _msg->getReadableVersion());
    CPPUNIT_ASSERT(_msg->getNodes6().empty());
    CPPUNIT_ASSERT(!_msg->getNodes4().empty());
    CPPUNIT_ASSERT(0 == _msg->getToken());

    auto nodes = _msg->getNodes4();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes4, nodes));
}

void
FindNodeTests::testFindNodeResponse4WithToken() {
    auto id = Id::random();
    int txid = Utils::getRandomInteger(62);

    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.2", 1232));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.3", 1233));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.4", 1234));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.5", 1235));

    auto msg = FindNodeResponse(txid);
    msg.setId(id);
    msg.setVersion(VERSION);
    msg.setNodes4(nodes4);
    msg.setToken(0x12345678);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindNodeResponse>(parsed);

    CPPUNIT_ASSERT(Message::Type::RESPONSE == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(id == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(VERSION_STR == _msg->getReadableVersion());
    CPPUNIT_ASSERT(_msg->getNodes6().empty());
    CPPUNIT_ASSERT(!_msg->getNodes4().empty());
    CPPUNIT_ASSERT(0x12345678 == _msg->getToken());

    auto nodes = _msg->getNodes4();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes4, nodes));
}

void FindNodeTests::testFindNodeResponse6() {
    auto id = Id::random();
    int txid = Utils::getRandomInteger(62);

    std::list<std::shared_ptr<NodeInfo>> nodes6 {};
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:8070:6543:8a2e:0370:7334", 65535));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7332", 1232));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7333", 1233));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7334", 1234));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7335", 1235));

    auto msg = FindNodeResponse(txid);
    msg.setId(id);
    msg.setVersion(VERSION);
    msg.setNodes6(nodes6);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindNodeResponse>(parsed);

    CPPUNIT_ASSERT(Message::Type::RESPONSE == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(id == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(VERSION_STR == _msg->getReadableVersion());
    CPPUNIT_ASSERT(_msg->getNodes4().empty());
    CPPUNIT_ASSERT(!_msg->getNodes6().empty());
    CPPUNIT_ASSERT(0 == _msg->getToken());

    auto nodes = _msg->getNodes6();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes6, nodes));
}

void FindNodeTests::testFindNodeResponse6WithToken() {
    auto id = Id::random();
    int txid = Utils::getRandomInteger(62);

    std::list<std::shared_ptr<NodeInfo>> nodes6 {};
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:8070:6543:8a2e:0370:7334", 65535));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7332", 1232));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7333", 1233));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7334", 1234));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7335", 1235));

    auto msg = FindNodeResponse(txid);
    msg.setId(id);
    msg.setVersion(VERSION);
    msg.setNodes6(nodes6);
    msg.setToken(0x43218765);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindNodeResponse>(parsed);

    CPPUNIT_ASSERT(Message::Type::RESPONSE == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(id == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(VERSION_STR == _msg->getReadableVersion());
    CPPUNIT_ASSERT(_msg->getNodes4().empty());
    CPPUNIT_ASSERT(!_msg->getNodes6().empty());
    CPPUNIT_ASSERT(0x43218765 == _msg->getToken());

    auto nodes = _msg->getNodes6();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes6, nodes));
}

void FindNodeTests::testFindNodeResponse46() {
    auto id = Id::random();
    int txid = Utils::getRandomInteger(62);

    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.2", 1232));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.3", 1233));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.4", 1234));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.5", 1235));

    std::list<std::shared_ptr<NodeInfo>> nodes6 {};
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:8070:6543:8a2e:0370:7334", 65535));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7332", 1232));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7333", 1233));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7334", 1234));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7335", 1235));

    auto msg = FindNodeResponse(txid);
    msg.setId(id);
    msg.setNodes4(nodes4);
    msg.setNodes6(nodes6);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindNodeResponse>(parsed);

    CPPUNIT_ASSERT(Message::Type::RESPONSE == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(id == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(0 == _msg->getVersion());
    CPPUNIT_ASSERT(!_msg->getNodes4().empty());
    CPPUNIT_ASSERT(!_msg->getNodes6().empty());
    CPPUNIT_ASSERT(0 == _msg->getToken());

    auto nodes = _msg->getNodes4();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes4, nodes));

    nodes = _msg->getNodes6();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes6, nodes));
}

void FindNodeTests::testFindNodeResponse46WithToken() {
    auto id = Id::random();
    int txid = Utils::getRandomInteger(62);

    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.2", 1232));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.3", 1233));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.4", 1234));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.5", 1235));

    std::list<std::shared_ptr<NodeInfo>> nodes6 {};
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:8070:6543:8a2e:0370:7334", 65535));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7332", 1232));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7333", 1233));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7334", 1234));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7335", 1235));

    auto msg = FindNodeResponse(txid);
    msg.setId(id);
    msg.setNodes4(nodes4);
    msg.setNodes6(nodes6);
    msg.setToken(0x87654321);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindNodeResponse>(parsed);

    CPPUNIT_ASSERT(Message::Type::RESPONSE == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::FIND_NODE == _msg->getMethod());
    CPPUNIT_ASSERT(id == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(0 == _msg->getVersion());
    CPPUNIT_ASSERT(false == _msg->getNodes4().empty());
    CPPUNIT_ASSERT(false == _msg->getNodes6().empty());
    CPPUNIT_ASSERT(0x87654321 == _msg->getToken());

    auto nodes = _msg->getNodes4();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes4, nodes));

    nodes = _msg->getNodes6();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes6, nodes));
}

void FindNodeTests::tearDown() {
}

}
