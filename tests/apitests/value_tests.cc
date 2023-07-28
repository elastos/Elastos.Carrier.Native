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
#include <iostream>
#include <carrier.h>

#include "utils.h"
#include "value_tests.h"
#include "utils/misc.h"

using namespace elastos::carrier;
using namespace std::chrono_literals;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(ValueTests);

void
ValueTests::setUp() {
    std::string path1 = Utils::getPwdStorage("carrier.db");
    std::string path2 = Utils::getPwdStorage("carriernode.db");

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
ValueTests::testImmutableValue() {
    auto data = Utils::getRandomData(32);
    auto value = Value::createValue(data);

    CPPUNIT_ASSERT(value.isValid());
    CPPUNIT_ASSERT(!value.isMutable());
    CPPUNIT_ASSERT(!value.isSigned());
    CPPUNIT_ASSERT(!value.isEncrypted());
    CPPUNIT_ASSERT(!value.hasPrivateKey());
    CPPUNIT_ASSERT(value.getSequenceNumber() == 0);
    CPPUNIT_ASSERT(value.getData() == data);
}

void
ValueTests::testSignedValue() {
    auto data1 = Utils::getRandomData(32);
    auto val1 = Value::createSignedValue(data1);

    CPPUNIT_ASSERT(val1.isValid());
    CPPUNIT_ASSERT(val1.isMutable());
    CPPUNIT_ASSERT(val1.isSigned());
    CPPUNIT_ASSERT(!val1.isEncrypted());
    CPPUNIT_ASSERT(val1.hasPrivateKey());
    CPPUNIT_ASSERT(!val1.getSignature().empty());
    CPPUNIT_ASSERT(val1.getSequenceNumber() == 0);
    CPPUNIT_ASSERT(val1.getData() == data1);

    auto valueId = val1.getId();
    auto future = node1->storeValue(val1);
    future.get();

    auto futureValue = node1->findValue(valueId, LookupOption::ARBITRARY);
    auto val2 = futureValue.get();

    CPPUNIT_ASSERT(val1 == *val2);

    //update signed value
    auto data2 = Utils::getRandomData(64);
    auto val3 = val1.update(data2);

    CPPUNIT_ASSERT(val3.isValid());
    CPPUNIT_ASSERT(val3.isMutable());
    CPPUNIT_ASSERT(val3.isSigned());
    CPPUNIT_ASSERT(!val3.isEncrypted());
    CPPUNIT_ASSERT(val3.hasPrivateKey());
    CPPUNIT_ASSERT(val3.getPublicKey() == val1.getPublicKey());
    CPPUNIT_ASSERT(val3.getNonce() == val1.getNonce());
    CPPUNIT_ASSERT(val3.getPrivateKey() == val1.getPrivateKey());
    CPPUNIT_ASSERT(val3.getSignature() != val1.getSignature());
    CPPUNIT_ASSERT(val3.getData() == data2);
    CPPUNIT_ASSERT(val3.getSequenceNumber() == 1);
    CPPUNIT_ASSERT(val3.getId() == val1.getId());

    auto p3 = node1->storeValue(val3);
    p3.get();

    auto p4 = node1->findValue(valueId, LookupOption::ARBITRARY);
    auto val4 = p4.get();
    CPPUNIT_ASSERT(val4);

    CPPUNIT_ASSERT(*val4 == val3);
}

void
ValueTests::testEncryptedValue() {
    auto data1 = Utils::getRandomData(32);
    auto val1 = Value::createEncryptedValue(node2->getId(), data1);

    CPPUNIT_ASSERT(val1.isValid());
    CPPUNIT_ASSERT(val1.isMutable());
    CPPUNIT_ASSERT(val1.isSigned());
    CPPUNIT_ASSERT(val1.isEncrypted());
    CPPUNIT_ASSERT(val1.hasPrivateKey());
    CPPUNIT_ASSERT(val1.getRecipient() == node2->getId());
    CPPUNIT_ASSERT(!val1.getSignature().empty());
    CPPUNIT_ASSERT(val1.getSequenceNumber() == 0);
    CPPUNIT_ASSERT(val1.getData() != data1); // encrypted data.

    auto valueId = val1.getId();
    auto future = node1->storeValue(val1);
    future.get();

    auto futureValue = node1->findValue(valueId, LookupOption::ARBITRARY);
    auto val2 = futureValue.get();

    CPPUNIT_ASSERT(val1 == *val2);

    //update encrypted value
    auto data2 = Utils::getRandomData(64);
    auto val3 = val1.update(data2);

    CPPUNIT_ASSERT(val3.isValid());
    CPPUNIT_ASSERT(val3.isMutable());
    CPPUNIT_ASSERT(val3.isSigned());
    CPPUNIT_ASSERT(val3.isEncrypted());
    CPPUNIT_ASSERT(val3.hasPrivateKey());
    CPPUNIT_ASSERT(val3.getPublicKey() == val1.getPublicKey());
    CPPUNIT_ASSERT(val3.getRecipient()== node2->getId());
    CPPUNIT_ASSERT(val3.getNonce() == val1.getNonce());
    CPPUNIT_ASSERT(val3.getSignature() != val1.getSignature());
    CPPUNIT_ASSERT(val3.getRecipient() == val1.getRecipient());
    CPPUNIT_ASSERT(val3.getData() != data2);
    CPPUNIT_ASSERT(val3.getSequenceNumber() == 1);
    CPPUNIT_ASSERT(val3.getId() == val1.getId());

    auto p3 = node1->storeValue(val3);
    p3.get();

    auto p4 = node1->findValue(valueId, LookupOption::ARBITRARY);
    auto val4 = p4.get();
    CPPUNIT_ASSERT(val4);

    CPPUNIT_ASSERT(*val4 == val3);
}

void
ValueTests::tearDown() {
    if (node1)
        node1->stop();
    if (node2)
        node2->stop();

    std::string path1 = Utils::getPwdStorage("carrier.db");
    std::string path2 = Utils::getPwdStorage("carriernode.db");

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);
}

}  // namespace test
