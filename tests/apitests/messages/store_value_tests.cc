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

#include "messages/message.h"
#include "messages/store_value_request.h"
#include "messages/store_value_response.h"

#include "utils.h"
#include "store_value_tests.h"

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(StoreValueTests);

void StoreValueTests::setUp() {
}

void StoreValueTests::testStoreValueRequestSize() {
#if 0
    std::vector<uint8_t> sig {};
    uint8_t s = 'S';
    for (int i = 0; i < 64; i++)
        sig.push_back(s);

    std::vector<uint8_t> value {};
    uint8_t v = 'D';
    for (int i = 0; i < 1056; i++)
        value.push_back(v);

    auto msg = StoreValueRequest();
    msg.setId(Id::random());
    msg.setTxid(0x87654321);
    msg.setVersion(VERSION);
    msg.setPublicKey(Id::random());
    msg.setRecipient(Id::random());
    auto n = CryptoBox::Nonce().random();
    msg.setNonce(n);
    msg.setExpectedSequenceNumber(0x77654320);
    msg.setSequenceNumber(0x77654321);
    msg.setSignature(sig);
    msg.setToken(0x88888888);
    msg.setData(value);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
#endif
}

void StoreValueTests::testStoreValueRequest() {
#if 0
    auto nodeId = Id::random();
    auto pk = Id::random();
    auto recipient = Id::random();
    int txid = Utils::getRandomInteger(62);

    int cas = Utils::getRandomInteger(62);
    int seq = cas + 1;

    std::vector<uint8_t> sig {};
    sig.resize(64);
    Utils::setRandomBytes(sig.data(), sig.size());

    int token = Utils::getRandomValue();

    std::vector<uint8_t> value {};
    value.resize(1056);
    Utils::setRandomBytes(value.data(), value.size());

    auto msg = StoreValueRequest();
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setVersion(VERSION);
    msg.setPublicKey(pk);
    msg.setRecipient(recipient);
    auto n = CryptoBox::Nonce().random();
    msg.setNonce(n);
    msg.setExpectedSequenceNumber(cas);
    msg.setSequenceNumber(seq);
    msg.setSignature(sig);
    msg.setToken(token);
    msg.setData(value);

    auto serialized = msg.serialize();
    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<StoreValueRequest>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::REQUEST, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::STORE_VALUE, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(nodeId, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(VERSION_STR, _msg->getReadableVersion());
    CPPUNIT_ASSERT_EQUAL(pk, _msg->getPublicKey());
    CPPUNIT_ASSERT_EQUAL(recipient, _msg->getRecipient());
    //todo:
    //CPPUNIT_ASSERT(nonce == m->getNonce());
    CPPUNIT_ASSERT_EQUAL(cas, _msg->getExpectedSequenceNumber());
    CPPUNIT_ASSERT_EQUAL(seq, _msg->getSequenceNumber());
    CPPUNIT_ASSERT(sig == _msg->getSignature());
    CPPUNIT_ASSERT_EQUAL(token, _msg->getToken());
    CPPUNIT_ASSERT(value == _msg->getData());
#endif
}

void StoreValueTests::testStoreValueResponseSize() {
    auto msg = StoreValueResponse(0xf7654321);
    msg.setId(Id::random());
    msg.setTxid(0x87654321);
    msg.setVersion(VERSION);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
}

void StoreValueTests::testStoreValueResponse() {
    auto id = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = StoreValueResponse(txid);
    msg.setId(id);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<StoreValueResponse>(parsed);

    CPPUNIT_ASSERT_EQUAL(Message::Type::RESPONSE, _msg->getType());
    CPPUNIT_ASSERT_EQUAL(Message::Method::STORE_VALUE, _msg->getMethod());
    CPPUNIT_ASSERT_EQUAL(id, _msg->getId());
    CPPUNIT_ASSERT_EQUAL(txid, _msg->getTxid());
    CPPUNIT_ASSERT_EQUAL(0, _msg->getVersion());
}

void StoreValueTests::tearDown() {
}
}
