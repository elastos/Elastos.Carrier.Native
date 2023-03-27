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

void PingTests::setUp() {
}

void PingTests::testPingRequestSize() {
    auto msg = PingRequest();
    msg.setId(Id::random());
    msg.setTxid(0xF8901234);
    msg.setVersion(VERSION);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
}

void PingTests::testPingRequest() {
    auto id = Id::random();
    int txid = Utils::getRandomInteger(62);

    auto msg = PingRequest();
    msg.setId(id);
    msg.setTxid(txid);
    msg.setVersion(VERSION);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<PingRequest>(parsed);

    CPPUNIT_ASSERT(Message::Type::REQUEST == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::PING == _msg->getMethod());
    CPPUNIT_ASSERT(id == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(VERSION_STR == _msg->getReadableVersion());
}

void
PingTests::testPingResponseSize() {
    auto msg = PingResponse();
    msg.setId(Id::random());
    msg.setTxid(0x78901234);
    msg.setVersion(VERSION);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());
}

void
PingTests::testPingResponse() {
    auto id = Id::random();
    int txid = Utils::getRandomInteger(62);;

    auto msg = PingResponse(txid);
    msg.setId(id);

    auto serialized = msg.serialize();
    CPPUNIT_ASSERT(serialized.size() <= msg.estimateSize());

    auto parsed = Message::parse(serialized.data(), serialized.size());
    parsed->setId(id);
    auto _msg = std::static_pointer_cast<PingResponse>(parsed);

    CPPUNIT_ASSERT(Message::Type::RESPONSE == _msg->getType());
    CPPUNIT_ASSERT(Message::Method::PING == _msg->getMethod());
    CPPUNIT_ASSERT(id == _msg->getId());
    CPPUNIT_ASSERT(txid == _msg->getTxid());
    CPPUNIT_ASSERT(0 == _msg->getVersion());
}

void PingTests::tearDown() {
}
}
