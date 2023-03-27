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
#include "utils/misc.h"
#include "store_find_value_tests.h"

using namespace elastos::carrier;
using namespace std::chrono_literals;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(StoreFindValueTests);

void
StoreFindValueTests::setUp() {
    std::string path = getenv("PWD");
    std::string path1 = path + "/carrier1";
    std::string path2 = path + "/carrier2";
    std::string path3 = path + "/carrier3";

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);
    Utils::removeStorage(path3);

    //create node1, node2 and node3
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

    auto b3 = DefaultConfiguration::Builder {};
    b3.setIPv4Address(ipAddresses);
    b3.setListeningPort(42224);
    b3.setStoragePath(path3);

    node3 = std::make_shared<Node>(b3.build());
    node3->start();

    //boostrap
    auto ni1 = NodeInfo {node1->getId(), ipAddresses, node1->getPort()};
    node2->bootstrap(ni1);
    node3->bootstrap(ni1);
}

void
StoreFindValueTests::testValue() {
    auto data = Utils::getRandomData(32);
    auto val = node1->createValue(data);
    CPPUNIT_ASSERT(val);

    //node1 to find value: no value
    auto future1 = node1->findValue(val->getId());
    auto val1 = future1.get();
    CPPUNIT_ASSERT(val1 == nullptr);

    //node2 to find value: no value
    auto future2 = node2->findValue(val->getId());
    auto val2 = future2.get();
    CPPUNIT_ASSERT(val2 == nullptr);

    //node1 to store value: success
    auto future3 = node1->storeValue(val);
    auto result = future3.get();
    CPPUNIT_ASSERT(result);

    //node1 to find value: has value
    auto future4 = node1->findValue(val->getId());
    auto val4 = future4.get();
    CPPUNIT_ASSERT(val4);
    CPPUNIT_ASSERT(val4->isValid());
    CPPUNIT_ASSERT(*val4 == *val);

    //node2 to find value: has value
    auto future6 = node2->findValue(val->getId());
    auto val6 = future6.get();
    CPPUNIT_ASSERT(val6);
    CPPUNIT_ASSERT(val6->isValid());
    CPPUNIT_ASSERT(*val6 == *val);
}

void
StoreFindValueTests::testSignedValue() {
    auto data1 = Utils::getRandomData(32);
    auto val = node1->createSignedValue(data1);
    CPPUNIT_ASSERT(val);

    auto valueId = val->getId();

    //node1 to find value: no value
    auto future1 = node1->findValue(val->getId());
    auto val1 = future1.get();
    CPPUNIT_ASSERT(val1 == nullptr);

    //node2 to find value: no value
    auto future2 = node2->findValue(val->getId());
    auto val2 = future2.get();
    CPPUNIT_ASSERT(val2 == nullptr);

    //node1 to store value: success
    auto future3 = node1->storeValue(val);
    auto result = future3.get();
    CPPUNIT_ASSERT(result);

    //node2 to find value with default mode: has value
    auto future4 = node2->findValue(val->getId());
    auto val4 = future4.get();
    CPPUNIT_ASSERT(val4);
    CPPUNIT_ASSERT(val4->isValid());
    CPPUNIT_ASSERT(*val4 == *val);

    //node1 update signed value
    auto data2 = Utils::getRandomData(64);
    auto val5 = node1->updateValue(valueId, data2);
    CPPUNIT_ASSERT(val5);

    auto future6 = node1->storeValue(val5);
    auto result2 = future6.get();
    CPPUNIT_ASSERT_EQUAL(true, result2);

    //node2 to find new value
    auto future7 = node2->findValue(valueId);
    auto val7 = future7.get();
    CPPUNIT_ASSERT(val7);

    CPPUNIT_ASSERT(*val7 == *val5);
    CPPUNIT_ASSERT(!(*val7 == *val));
}

void
StoreFindValueTests::testEncryptedValue() {
    auto data1 = Utils::getRandomData(32);
    auto val = node1->createEncryptedValue(node2->getId(), data1);
    CPPUNIT_ASSERT(val);
    CPPUNIT_ASSERT(val->isValid());

    auto valueId = val->getId();

    //node1 store value: success
    auto future1 = node1->storeValue(val);
    auto result1 = future1.get();
    CPPUNIT_ASSERT_EQUAL(true, result1);

    //node1 to find value: has value
    auto future2 = node1->findValue(valueId);
    auto val2 = future2.get();
    CPPUNIT_ASSERT(val2);
    CPPUNIT_ASSERT(*val == *val2);

    //node2 to find value: has value
    auto future3 = node2->findValue(valueId);
    auto val3 = future3.get();
    CPPUNIT_ASSERT(val3);
    CPPUNIT_ASSERT(*val == *val3);

    //node3 to find value: has value
    auto future4 = node3->findValue(valueId);
    auto val4 = future4.get();
    CPPUNIT_ASSERT(val4);
    CPPUNIT_ASSERT(*val == *val4);

    //update encrypted value
    auto data2 = Utils::getRandomData(64);
    auto val5 = node1->updateValue(valueId, data2);
    CPPUNIT_ASSERT(val5);
    CPPUNIT_ASSERT(val5->isValid());

    auto future6 = node1->storeValue(val5);
    auto result2 = future6.get();
    CPPUNIT_ASSERT_EQUAL(true, result2);

    //node1 to find new value
    auto future7 = node1->findValue(valueId);
    auto val7 = future7.get();
    CPPUNIT_ASSERT(val7);
    CPPUNIT_ASSERT(*val7 == *val5);
    CPPUNIT_ASSERT(!(*val7 == *val));

    auto future8 = node2->findValue(valueId);
    auto val8 = future8.get();
    CPPUNIT_ASSERT(val8);
    CPPUNIT_ASSERT(*val8 == *val5);
    CPPUNIT_ASSERT(!(*val8 == *val));

    auto future9 = node3->findValue(valueId);
    auto val9 = future9.get();
    CPPUNIT_ASSERT(val9);
    CPPUNIT_ASSERT(*val9 == *val5);
    CPPUNIT_ASSERT(!(*val9 == *val));
}

void
StoreFindValueTests::tearDown() {
    if (node1)
        node1->stop();
    if (node2)
        node2->stop();
    if (node3)
        node3->stop();

    std::string pwd = getenv("PWD");
    Utils::removeStorage(pwd + "/carrier1");
    Utils::removeStorage(pwd + "/carrier2");
    Utils::removeStorage(pwd + "/carrier3");
}

}  // namespace test
