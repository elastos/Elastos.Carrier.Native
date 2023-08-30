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
#include "messages/ping_request.h"
#include "messages/ping_response.h"

#include "utils.h"
#include "ping_tests.h"

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(PingTests);

void PingTests::testPingRequest1() {
    auto nodeId = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = PingRequest();
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setVersion(VERSION);

    CPPUNIT_ASSERT(msg.getType() == Message::Type::REQUEST);
    CPPUNIT_ASSERT(msg.getMethod() == Message::Method::PING);
    CPPUNIT_ASSERT(msg.getId() == nodeId);
    CPPUNIT_ASSERT(msg.getTxid() == txid);
    CPPUNIT_ASSERT(msg.getVersion() == VERSION);
    CPPUNIT_ASSERT(msg.estimateSize() >= msg.serialize().size());
}

void PingTests::testPingRequest2() {
    auto nodeId = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = PingRequest();
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setVersion(VERSION);

    auto serialized = msg.serialize();
    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<PingRequest>(parsed);

    CPPUNIT_ASSERT(_msg->getType() == Message::Type::REQUEST);
    CPPUNIT_ASSERT(_msg->getMethod() == Message::Method::PING);
    CPPUNIT_ASSERT(_msg->getId() == nodeId);
    CPPUNIT_ASSERT(_msg->getTxid() == txid);
    CPPUNIT_ASSERT(_msg->getVersion() == VERSION);
    CPPUNIT_ASSERT(_msg->getReadableVersion() == VERSION_STR);
}

void PingTests::testPingResponse1() {
    auto nodeId = Id::random();
    int txid = Utils::getRandomInteger(62);
    int version = Utils::getRandomInteger(100);

    auto msg = PingResponse();
    msg.setId(nodeId);
    msg.setTxid(txid);
    msg.setVersion(version);

    CPPUNIT_ASSERT(msg.getType() == Message::Type::RESPONSE);
    CPPUNIT_ASSERT(msg.getMethod() == Message::Method::PING);
    CPPUNIT_ASSERT(msg.getId() == nodeId);
    CPPUNIT_ASSERT(msg.getTxid() == txid);
    CPPUNIT_ASSERT(msg.getVersion() == version);
    CPPUNIT_ASSERT(msg.estimateSize() >= msg.serialize().size());
}

void PingTests::testPingResponse2() {
    auto nodeId = Id::random();
    int txid = Utils::getRandomInteger(62);
    int version = Utils::getRandomInteger(100);

    auto msg = PingResponse(txid);
    msg.setId(nodeId);
    msg.setVersion(version);

    auto serialized = msg.serialize();
    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(nodeId);
    auto _msg = std::static_pointer_cast<PingResponse>(parsed);

    CPPUNIT_ASSERT(_msg->getType() == Message::Type::RESPONSE);
    CPPUNIT_ASSERT(_msg->getMethod() == Message::Method::PING);
    CPPUNIT_ASSERT(_msg->getId() == nodeId);
    CPPUNIT_ASSERT(_msg->getTxid() == txid);
    CPPUNIT_ASSERT(_msg->getVersion() == version);
}

}
