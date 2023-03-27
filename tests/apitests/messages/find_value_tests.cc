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

#include "carrier/node_info.h"
#include "messages/message.h"
#include "messages/find_value_request.h"
#include "messages/find_value_response.h"

#include "utils.h"
#include "find_value_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(FindValueTests);

void FindValueTests::setUp() {
}

void FindValueTests::testFindValueRequestSize() {
    auto msg = FindValueRequest(Id::random());
    msg.setId(Id::random());
    msg.setTxid(0x87654321);
    msg.setVersion(VERSION);
    msg.setWant4(true);
    msg.setWant6(true);
    msg.setSequenceNumber(Utils::getRandomInteger(62));

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
}

void FindValueTests::testFindValueRequest4() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomValue();
    int seq = Utils::getRandomInteger(62);

    auto msg = FindValueRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setVersion(VERSION);
    msg.setWant4(true);
    msg.setWant6(false);
    msg.setSequenceNumber(seq);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindValueRequest>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::REQUEST, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_VALUE, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(nodeId, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(VERSION_STR, _msg->getReadableVersion());
    CPPUNIT_ASSERT_EQUAL(target, _msg->getTarget());
    CPPUNIT_ASSERT_EQUAL(seq, _msg->getSequenceNumber());
    CPPUNIT_ASSERT(_msg->doesWant4());
    CPPUNIT_ASSERT(!_msg->doesWant6());
}

void FindValueTests::testFindValueRequest6() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomValue();
    int seq = Utils::getRandomInteger(62);

    auto msg = FindValueRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(false);
    msg.setWant6(true);
    msg.setSequenceNumber(seq);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindValueRequest>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::REQUEST, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_VALUE, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(nodeId, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(target, _msg->getTarget());
    CPPUNIT_ASSERT_EQUAL(seq, _msg->getSequenceNumber());
    CPPUNIT_ASSERT(!_msg->doesWant4());
    CPPUNIT_ASSERT(_msg->doesWant6());
}

void FindValueTests::testFindValueRequest46() {
    auto nodeId = Id::random();
    auto target = Id::random();
    int txid = Utils::getRandomValue();
    int seq = Utils::getRandomInteger(62);

    auto msg = FindValueRequest(target);
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setWant4(true);
    msg.setWant6(true);
    msg.setSequenceNumber(seq);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<FindValueRequest>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::REQUEST, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_VALUE, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(nodeId, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(target, _msg->getTarget());
    CPPUNIT_ASSERT_EQUAL(seq, _msg->getSequenceNumber());
    CPPUNIT_ASSERT(_msg->doesWant4());
    CPPUNIT_ASSERT(_msg->doesWant6());
}

void
FindValueTests::testFindValueResponseSize() {
#if 0
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

    std::vector<uint8_t> sig {};
    uint8_t s = 'S';
    for (int i = 0; i < 64; i++)
        sig.push_back(s);

    std::vector<uint8_t> value {};
    uint8_t v = 'D';
    for (int i = 0; i < 1056; i++)
        value.push_back(v);

    auto msg = FindValueResponse(0xF7654321);
    msg.setId(Id::random());
    msg.setVersion(VERSION);
    msg.setNodes4(nodes4);
    msg.setNodes6(nodes6);
    msg.setPublicKey(Id::random());
    msg.setRecipient(Id::random());
    auto n = CryptoBox::Nonce().random();
    msg.setNonce(n);
    msg.setSequenceNumber(0x77654321);
    msg.setSignature(sig);
    msg.setToken(0xF8765432);
    msg.setData(value);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
#endif
}

void
FindValueTests::testFindValueResponse4() {
#if 0
    auto id = Id::random();
    auto pk = Id::random();
    auto recipient = Id::random();
    int txid = Utils::getRandomValue();

    int cas = Utils::getRandomInteger(62);
    int seq = cas + 1;

    std::vector<uint8_t> sig {};
    sig.resize(64);
    Utils::setRandomBytes(sig.data(), sig.size());

    int token = Utils::getRandomValue();

    std::vector<uint8_t> value {};
    value.resize(1056);
    Utils::setRandomBytes(value.data(), value.size());

    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.2", 1232));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.3", 1233));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.4", 1234));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.5", 1235));

    auto msg = FindValueResponse(txid);
    msg.setId(id);
    msg.setVersion(VERSION);
    msg.setNodes4(nodes4);
    msg.setPublicKey(pk);
    msg.setRecipient(recipient);
    auto n = CryptoBox::Nonce().random();
    msg.setNonce(n);
    msg.setSequenceNumber(seq);
    msg.setSignature(sig);
    msg.setToken(token);
    msg.setData(value);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindValueResponse>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::RESPONSE, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_VALUE, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(id, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(VERSION_STR, _msg->getReadableVersion());
    CPPUNIT_ASSERT(_msg->getNodes6().empty());
    CPPUNIT_ASSERT(!_msg->getNodes4().empty());
    CPPUNIT_ASSERT_EQUAL(pk, _msg->getPublicKey());
    CPPUNIT_ASSERT_EQUAL(recipient, _msg->getRecipient());
    //assertArrayEquals(nonce, m->getNonce());
    CPPUNIT_ASSERT_EQUAL(seq, _msg->getSequenceNumber());
    CPPUNIT_ASSERT(sig == _msg->getSignature());
    CPPUNIT_ASSERT_EQUAL(token, _msg->getToken());
    CPPUNIT_ASSERT(value == _msg->getData());

    auto nodes = _msg->getNodes4();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes4, nodes));
#endif
}

void FindValueTests::testFindValueResponse4Immutable() {
#if 0
    auto id = Id::random();
    int txid = Utils::getRandomValue();
    int token = Utils::getRandomValue();

    std::vector<uint8_t> value {};
    value.resize(1056);
    Utils::setRandomBytes(value.data(), value.size());

    std::list<std::shared_ptr<NodeInfo>> nodes4 {};
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "251.251.251.251", 65535));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.2", 1232));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.3", 1233));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.4", 1234));
    nodes4.push_back(std::make_shared<NodeInfo>(Id::random(), "192.168.1.5", 1235));

    auto msg = FindValueResponse(txid);
    msg.setId(id);
    msg.setVersion(VERSION);
    msg.setNodes4(nodes4);
    auto n = CryptoBox::Nonce().random();
    msg.setNonce(n);
    msg.setToken(token);
    msg.setData(value);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindValueResponse>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::RESPONSE, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_VALUE, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(id, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(VERSION_STR, _msg->getReadableVersion());
    CPPUNIT_ASSERT(_msg->getNodes6().empty());
    CPPUNIT_ASSERT(!_msg->getNodes4().empty());
    //assertArrayEquals(nonce, m->getNonce());
    CPPUNIT_ASSERT_EQUAL(token, _msg->getToken());
    CPPUNIT_ASSERT(value == _msg->getData());

    auto nodes = _msg->getNodes4();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes4, nodes));
#endif
}

void FindValueTests::testFindValueResponse6() {
#if 0
    auto id = Id::random();
    auto pk = Id::random();
    auto recipient = Id::random();
    int txid = Utils::getRandomValue();

    int cas = Utils::getRandomInteger(62);
    int seq = cas + 1;

    std::vector<uint8_t> sig {};
    sig.resize(64);
    Utils::setRandomBytes(sig.data(), sig.size());

    int token = Utils::getRandomValue();

    std::vector<uint8_t> value {};
    value.resize(1056);
    Utils::setRandomBytes(value.data(), value.size());

    std::list<std::shared_ptr<NodeInfo>> nodes6 {};
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:8070:6543:8a2e:0370:7334", 65535));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7332", 1232));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7333", 1233));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7334", 1234));
    nodes6.push_back(std::make_shared<NodeInfo>(Id::random(), "2001:0db8:85a3:0000:0000:8a2e:0370:7335", 1235));

    auto msg = FindValueResponse(txid);
    msg.setId(id);
    msg.setVersion(VERSION);
    msg.setNodes6(nodes6);
    msg.setPublicKey(pk);
    msg.setRecipient(recipient);
    auto n = CryptoBox::Nonce().random();
    msg.setNonce(n);
    msg.setSequenceNumber(seq);
    msg.setSignature(sig);
    msg.setToken(token);
    msg.setData(value);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindValueResponse>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::RESPONSE, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_VALUE, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(id, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(VERSION_STR, _msg->getReadableVersion());
    CPPUNIT_ASSERT(_msg->getNodes4().empty());
    CPPUNIT_ASSERT(false == _msg->getNodes6().empty());
    CPPUNIT_ASSERT_EQUAL(pk, _msg->getPublicKey());
    CPPUNIT_ASSERT_EQUAL(recipient, _msg->getRecipient());
    //assertArrayEquals(nonce, m->getNonce());
    CPPUNIT_ASSERT_EQUAL(seq, _msg->getSequenceNumber());
    CPPUNIT_ASSERT(sig == _msg->getSignature());
    CPPUNIT_ASSERT_EQUAL(token, _msg->getToken());
    CPPUNIT_ASSERT(value == _msg->getData());

    auto nodes = _msg->getNodes6();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes6, nodes));
#endif
}

void FindValueTests::testFindValueResponse46() {
#if 0
    auto id = Id::random();
    auto pk = Id::random();
    auto recipient = Id::random();
    int txid = Utils::getRandomValue();

    int cas = Utils::getRandomInteger(62);
    int seq = cas + 1;

    std::vector<uint8_t> sig {};
    sig.resize(64);
    Utils::setRandomBytes(sig.data(), sig.size());

    int token = Utils::getRandomValue();

    std::vector<uint8_t> value {};
    value.resize(1056);
    Utils::setRandomBytes(value.data(), value.size());

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

    auto msg = FindValueResponse(txid);
    msg.setId(id);
    msg.setNodes4(nodes4);
    msg.setNodes6(nodes6);
    msg.setPublicKey(pk);
    msg.setRecipient(recipient);
    auto n = CryptoBox::Nonce().random();
    msg.setNonce(n);
    msg.setSequenceNumber(seq);
    msg.setSignature(sig);
    msg.setToken(token);
    msg.setData(value);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<FindValueResponse>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::RESPONSE, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::FIND_VALUE, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(id, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(0, _msg->getVersion());
    CPPUNIT_ASSERT(!_msg->getNodes4().empty());
    CPPUNIT_ASSERT(!_msg->getNodes6().empty());
    CPPUNIT_ASSERT_EQUAL(pk, _msg->getPublicKey());
    CPPUNIT_ASSERT_EQUAL(recipient, _msg->getRecipient());
    //assertArrayEquals(nonce, m->getNonce());
    CPPUNIT_ASSERT_EQUAL(seq, _msg->getSequenceNumber());
    CPPUNIT_ASSERT(sig == _msg->getSignature());
    CPPUNIT_ASSERT_EQUAL(token, _msg->getToken());
    CPPUNIT_ASSERT(value == _msg->getData());

    auto nodes = _msg->getNodes4();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes4, nodes));

    nodes = _msg->getNodes6();
    CPPUNIT_ASSERT(Utils::arrayEquals(nodes6, nodes));
#endif
}

void FindValueTests::tearDown() {
}
}
