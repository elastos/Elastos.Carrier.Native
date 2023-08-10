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
#include "value_store_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(ValueStoreTests);

void ValueStoreTests::setUp() {
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

void ValueStoreTests::tearDown() {
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

void ValueStoreTests::testValue() {
    auto data = Utils::getRandomData(32);
    auto value = Value::createValue(data);
    CPPUNIT_ASSERT(!value.isSigned());
    CPPUNIT_ASSERT(!value.isEncrypted());
    CPPUNIT_ASSERT(value.isValid());

    auto valueId = value.getId();

    auto future1 = node1->findValue(valueId);
    auto val1 = future1.get();
    CPPUNIT_ASSERT(!val1);

    auto future2 = node2->findValue(valueId);
    auto val2 = future2.get();
    CPPUNIT_ASSERT(!val2);

    auto future3 = node1->storeValue(value);
    future3.get();

    // check findValue on local Node
    auto future4 = node1->findValue(valueId);
    auto val4 = future4.get();
    CPPUNIT_ASSERT(val4);
    CPPUNIT_ASSERT(!val4->isSigned());
    CPPUNIT_ASSERT(!val4->isEncrypted());
    CPPUNIT_ASSERT(val4->isValid());
    CPPUNIT_ASSERT(val4->getData() == data);
    CPPUNIT_ASSERT(val4->getId() == valueId);
    CPPUNIT_ASSERT(*val4 == value);

    // check findValue from remote Node
    auto future5 = node2->findValue(valueId);
    auto val5 = future5.get();
    CPPUNIT_ASSERT(val5);
    CPPUNIT_ASSERT(!val5->isSigned());
    CPPUNIT_ASSERT(!val5->isEncrypted());
    CPPUNIT_ASSERT(val5->isValid());
    CPPUNIT_ASSERT(val4->getData() == data);
    CPPUNIT_ASSERT(val4->getId() == valueId);
    CPPUNIT_ASSERT(*val5 == value);

    // check findValue from remote Node
    auto future6 = node3->findValue(valueId);
    auto val6 = future6.get();
    CPPUNIT_ASSERT(val6);
    CPPUNIT_ASSERT(!val6->isSigned());
    CPPUNIT_ASSERT(!val6->isEncrypted());
    CPPUNIT_ASSERT(val6->isValid());
    CPPUNIT_ASSERT(val4->getData() == data);
    CPPUNIT_ASSERT(val4->getId() == valueId);
    CPPUNIT_ASSERT(*val6 == value);
}

void ValueStoreTests::testSignedValue() {
    auto data = Utils::getRandomData(32);
    auto value = Value::createSignedValue(data);
    CPPUNIT_ASSERT(value.isSigned());
    CPPUNIT_ASSERT(!value.isEncrypted());
    CPPUNIT_ASSERT(value.isValid());

    auto valueId = value.getId();

    auto future1 = node1->findValue(valueId);
    auto val1 = future1.get();
    CPPUNIT_ASSERT(!val1);

    auto future2 = node2->findValue(valueId);
    auto val2 = future2.get();
    CPPUNIT_ASSERT(!val2);

    auto future3 = node1->storeValue(value);
    future3.get();

    // check findValue for signed value on remote node.
    auto future4 = node2->findValue(valueId);
    auto val4 = future4.get();
    CPPUNIT_ASSERT(val4);
    CPPUNIT_ASSERT(val4->isSigned());
    CPPUNIT_ASSERT(!val4->isEncrypted());
    CPPUNIT_ASSERT(val4->isValid());
    CPPUNIT_ASSERT(val4->getData() == data);
    CPPUNIT_ASSERT(val4->getId() == valueId);
    CPPUNIT_ASSERT(*val4 == value);

    // check findValue for signed value on remote node.
    auto future5 = node3->findValue(valueId);
    auto val5 = future5.get();
    CPPUNIT_ASSERT(val5);
    CPPUNIT_ASSERT(val5->isSigned());
    CPPUNIT_ASSERT(!val5->isEncrypted());
    CPPUNIT_ASSERT(val5->isValid());
    CPPUNIT_ASSERT(val5->getData() == data);
    CPPUNIT_ASSERT(val4->getId() == valueId);
    CPPUNIT_ASSERT(*val5 == value);

    // check updateValue
    auto newData = Utils::getRandomData(64);
    auto newValue = value.update(newData);
    CPPUNIT_ASSERT(newValue.isSigned());
    CPPUNIT_ASSERT(!newValue.isEncrypted());
    CPPUNIT_ASSERT(newValue.isSigned());
    CPPUNIT_ASSERT(newValue.getId() == valueId);

    auto future6 = node1->storeValue(newValue);
    future6.get();

    // check findValue for updated value on remote node.
    auto future7 = node2->findValue(valueId);
    auto val7 = future7.get();
    CPPUNIT_ASSERT(val7);
    CPPUNIT_ASSERT(val7->isSigned());
    CPPUNIT_ASSERT(!val7->isEncrypted());
    CPPUNIT_ASSERT(val7->isValid());
    CPPUNIT_ASSERT(val7->getData() == newData);
    CPPUNIT_ASSERT(val4->getId() == valueId);
    CPPUNIT_ASSERT(*val7 == newValue);

    // check findValue for updated value on remote node.
    auto future8 = node3->findValue(valueId);
    auto val8 = future8.get();
    CPPUNIT_ASSERT(val8);
    CPPUNIT_ASSERT(val8->isSigned());
    CPPUNIT_ASSERT(!val8->isEncrypted());
    CPPUNIT_ASSERT(val8->isValid());
    CPPUNIT_ASSERT(val8->getData() == newData);
    CPPUNIT_ASSERT(val4->getId() == valueId);
    CPPUNIT_ASSERT(*val8 == newValue);
}

void ValueStoreTests::testEncryptedValue() {
    auto data = Utils::getRandomData(32);
    auto value = Value::createEncryptedValue(node2->getId(), data);
    CPPUNIT_ASSERT(value.isSigned());
    CPPUNIT_ASSERT(value.isEncrypted());
    CPPUNIT_ASSERT(value.isValid());

    auto valueId = value.getId();

    auto future1 = node1->storeValue(value);
    future1.get();

    auto future2 = node1->findValue(valueId);
    auto val2 = future2.get();
    CPPUNIT_ASSERT(val2->isSigned());
    CPPUNIT_ASSERT(val2->isEncrypted());
    CPPUNIT_ASSERT(val2->isValid());
    CPPUNIT_ASSERT(val2);
    CPPUNIT_ASSERT(*val2 == value);

    //check findValue for encrypted value from remote node.
    auto future3 = node2->findValue(valueId);
    auto val3 = future3.get();
    CPPUNIT_ASSERT(val3);
    CPPUNIT_ASSERT(val3->isSigned());
    CPPUNIT_ASSERT(val3->isEncrypted());
    CPPUNIT_ASSERT(val3->isValid());
    CPPUNIT_ASSERT(val3->getId() == valueId);
    CPPUNIT_ASSERT(*val3 == value);

    //check findValue for encrypted value from remote node.
    auto future4 = node3->findValue(valueId);
    auto val4 = future4.get();
    CPPUNIT_ASSERT(val4);
    CPPUNIT_ASSERT(val4->isSigned());
    CPPUNIT_ASSERT(val4->isEncrypted());
    CPPUNIT_ASSERT(val4->isValid());
    CPPUNIT_ASSERT(val3->getId() == valueId);
    CPPUNIT_ASSERT(*val4 == value);

    //update encrypted value
    auto newData = Utils::getRandomData(64);
    auto newValue = value.update(newData);
    CPPUNIT_ASSERT(newValue.isSigned());
    CPPUNIT_ASSERT(newValue.isEncrypted());
    CPPUNIT_ASSERT(newValue.isValid());
    CPPUNIT_ASSERT(newValue.getId() == valueId);

    auto future5 = node1->storeValue(newValue);
    future5.get();

    //check to findValue for updated value on remote node
    auto future6 = node2->findValue(valueId);
    auto val6 = future6.get();
    CPPUNIT_ASSERT(val6);
    CPPUNIT_ASSERT(val6->isSigned());
    CPPUNIT_ASSERT(val6->isEncrypted());
    CPPUNIT_ASSERT(val6->isValid());
    CPPUNIT_ASSERT(val6->getId() == valueId);
    CPPUNIT_ASSERT(*val6 == newValue);

    //check to findValue for updated value on remote node
    auto future7 = node3->findValue(valueId);
    auto val7 = future7.get();
    CPPUNIT_ASSERT(val7);
    CPPUNIT_ASSERT(val7->isSigned());
    CPPUNIT_ASSERT(val7->isEncrypted());
    CPPUNIT_ASSERT(val7->isValid());
    CPPUNIT_ASSERT(val7->getId() == valueId);
    CPPUNIT_ASSERT(*val7 == newValue);
}

}  // namespace test
