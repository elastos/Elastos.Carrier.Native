/*
 * Copyright (c) 2022 - 2023 Elastos Foundation
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

#include <carrier/value.h>
#include <carrier/default_configuration.h>

#include "utils.h"
#include "value_tests.h"
#include "utils/misc.h"

using namespace elastos::carrier;
using namespace std::chrono_literals;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(ValueTests);

void
ValueTests::setUp() {
    std::string path = getenv("PWD");
    std::string path1 = path + "/carrier.db";
    std::string path2 = path + "/carriernode.db";

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);

    //create node1 and node2
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
}

void
ValueTests::testValue() {
    auto data = Utils::getRandomData(32);
    auto val = node1->createValue(data);
    CPPUNIT_ASSERT(val);

    CPPUNIT_ASSERT(val->isValid());
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val->getPublicKey()), false);
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val->getRecipient()), false);
    CPPUNIT_ASSERT_EQUAL(val->hasPrivateKey(), false);
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val->getNonce()), false);
    CPPUNIT_ASSERT(val->getSignature().empty());
    CPPUNIT_ASSERT_EQUAL(val->isMutable(), false);
    CPPUNIT_ASSERT_EQUAL(val->isEncrypted(), false);
    CPPUNIT_ASSERT_EQUAL(val->isSigned(), false);
    CPPUNIT_ASSERT_EQUAL(val->getSequenceNumber(), 0);
    CPPUNIT_ASSERT(val->getData() == data);
}

void
ValueTests::testSignedValue() {
    auto data1 = Utils::getRandomData(32);
    auto val1 = node1->createSignedValue(data1);
    CPPUNIT_ASSERT(val1);
    CPPUNIT_ASSERT(val1->isValid());
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val1->getPublicKey()), true);
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val1->getRecipient()), false);
    CPPUNIT_ASSERT_EQUAL(val1->hasPrivateKey(), true);
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val1->getNonce()), true);
    CPPUNIT_ASSERT(!val1->getSignature().empty());
    CPPUNIT_ASSERT_EQUAL(val1->isMutable(), true);
    CPPUNIT_ASSERT_EQUAL(val1->isEncrypted(), false);
    CPPUNIT_ASSERT_EQUAL(val1->isSigned(), true);
    CPPUNIT_ASSERT_EQUAL(val1->getSequenceNumber(), 0);
    CPPUNIT_ASSERT(val1->getData() == data1);

    auto valueId = val1->getId();

    auto p1 = node1->storeValue(val1);
    auto result1 = p1.get();
    CPPUNIT_ASSERT_EQUAL(true, result1);

    auto p2 = node1->findValue(valueId, LookupOption::ARBITRARY);
    auto val2 = p2.get();

    CPPUNIT_ASSERT(*val1 == *val2);
    //printf("\n\nval1: %s\n", (static_cast<std::string>(*val1)).c_str());
    //printf("val2: %s\n\n", (static_cast<std::string>(*val2)).c_str());

    //update signed value
    auto data2 = Utils::getRandomData(64);
    auto val3 = node1->updateValue(valueId, data2);
    CPPUNIT_ASSERT(val3);
    CPPUNIT_ASSERT(val3->isValid());
    CPPUNIT_ASSERT(val3->getPublicKey() == val1->getPublicKey());
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val3->getRecipient()), false);
    CPPUNIT_ASSERT(val1->getNonce() == val3->getNonce());
    CPPUNIT_ASSERT(val3->getSignature() != val1->getSignature());
    CPPUNIT_ASSERT(val3->getData() == data2);
    CPPUNIT_ASSERT_EQUAL(true, val3->hasPrivateKey());
    CPPUNIT_ASSERT_EQUAL(1, val3->getSequenceNumber());
    CPPUNIT_ASSERT_EQUAL(false, val3->isEncrypted());
    CPPUNIT_ASSERT_EQUAL(true, val3->isSigned());
    CPPUNIT_ASSERT_EQUAL(true, val3->isMutable());

    auto p3 = node1->storeValue(val3);
    result1 = p3.get();
    CPPUNIT_ASSERT_EQUAL(true, result1);

    auto p4 = node1->findValue(valueId, LookupOption::ARBITRARY);
    auto val4 = p4.get();
    CPPUNIT_ASSERT(val4);

    CPPUNIT_ASSERT(*val4 == *val3);
}

void
ValueTests::testEncryptedValue() {
    auto data1 = Utils::getRandomData(32);
    auto val1 = node1->createEncryptedValue(node2->getId(), data1);
    CPPUNIT_ASSERT(val1);

    CPPUNIT_ASSERT(val1->isValid());
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val1->getPublicKey()), true);
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val1->getRecipient()), true);
    CPPUNIT_ASSERT(val1->getRecipient() == node2->getId());
    CPPUNIT_ASSERT_EQUAL(static_cast<bool>(val1->getNonce()), true);
    CPPUNIT_ASSERT(!val1->getSignature().empty());
    CPPUNIT_ASSERT_EQUAL(val1->isMutable(), true);
    CPPUNIT_ASSERT_EQUAL(val1->isEncrypted(), true);
    CPPUNIT_ASSERT_EQUAL(val1->isSigned(), true);
    CPPUNIT_ASSERT_EQUAL(val1->getSequenceNumber(), 0);
    CPPUNIT_ASSERT(val1->getData() != data1); //the data of encrypted value is encrpted data, not origin data

    auto valueId = val1->getId();

    auto p1 = node1->storeValue(val1);
    auto result1 = p1.get();
    CPPUNIT_ASSERT_EQUAL(true, result1);

    auto p2 = node1->findValue(valueId, LookupOption::ARBITRARY);
    auto val2 = p2.get();

    CPPUNIT_ASSERT(*val1 == *val2);

    //update encrypted value
    auto data2 = Utils::getRandomData(64);
    auto val3 = node1->updateValue(valueId, data2);
    CPPUNIT_ASSERT(val3);
    CPPUNIT_ASSERT(val3->isValid());
    CPPUNIT_ASSERT(val3->getPublicKey() == val1->getPublicKey());
    CPPUNIT_ASSERT(val3->getRecipient()== node2->getId());
    CPPUNIT_ASSERT(val1->getNonce() == val3->getNonce());
    CPPUNIT_ASSERT(val3->getSignature() != val1->getSignature());
    CPPUNIT_ASSERT(val3->getData() != data2);
    CPPUNIT_ASSERT_EQUAL(true, val3->hasPrivateKey());
    CPPUNIT_ASSERT_EQUAL(1, val3->getSequenceNumber());
    CPPUNIT_ASSERT_EQUAL(true, val3->isEncrypted());
    CPPUNIT_ASSERT_EQUAL(true, val3->isSigned());
    CPPUNIT_ASSERT_EQUAL(true, val3->isMutable());

    auto p3 = node1->storeValue(val3);
    result1 = p3.get();
    CPPUNIT_ASSERT_EQUAL(true, result1);

    auto p4 = node1->findValue(valueId, LookupOption::ARBITRARY);
    auto val4 = p4.get();
    CPPUNIT_ASSERT(val4);

    CPPUNIT_ASSERT(*val4 == *val3);
}

void
ValueTests::tearDown() {
    if (node1)
        node1->stop();
    if (node2)
        node2->stop();
}

}  // namespace test
