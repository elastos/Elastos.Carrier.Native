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
#include <filesystem>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <carrier/id.h>
#include <carrier/value.h>
#include <carrier/node.h>
#include <carrier/peer_info.h>
#include <carrier/default_configuration.h>

#include "sqlite_storage.h"
#include "data_storage.h"
#include "utils.h"

#include "storage_tests.h"

using namespace std::chrono_literals;
using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(DataStorageTests);

void increment(std::vector<uint8_t> value) {
    short c = 1;
    for (int i = value.size() - 1; i >= 0; i--) {
        c += (value[i] & 0xff);
        value[i] = (uint8_t)(c & 0xff);
        c >>= 8;

        if (c == 0)
            break;
    }
}

void DataStorageTests::setUp() {
    path = getenv("PWD");
    path1 = path + "/carrier.db";
    path2 = path + "/carriernode.db";

    Utils::removeStorage(path1);
    Utils::removeStorage(path2);

    //create node1 and node2
    auto b1 = DefaultConfiguration::Builder {};

    auto ipAddresses = Utils::getLocalIpAddresses();
    b1.setIPv4Address(ipAddresses);
    b1.setListeningPort(42222);
    b1.setStoragePath(path);

    node1 = std::make_shared<Node>(b1.build());
    node1->start();

    auto b2 = DefaultConfiguration::Builder {};
    b2.setIPv4Address(ipAddresses);
    b2.setListeningPort(42223);
    b2.setStoragePath(path);

    node2 = std::make_shared<Node>(b2.build());
    node2->start();
}

void DataStorageTests::tearDown() {
    if (node2)
        node2->stop();
    if (node1)
        node1->stop();
}

void DataStorageTests::testPutAndGetValue() {
    auto ds = SqliteStorage::open(path1, scheduler);

    std::list<Id> ids {};

    std::cout << "Writing values...";
    for (int i = 1; i <= 256; i++) {
        std::vector<uint8_t> data;
        data.resize(1024);
        std::fill(data.begin(), data.end(), (uint8_t)(i % (126 - 32) + 33));
        auto v = node1->createValue(data);

        ids.push_back(v->getId());
        ds->putValue(v, -1);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Reading values...";
    for (int i = 1; i <= 256; i++) {
        auto id = ids.front();
        auto v = ds->getValue(id);
        CPPUNIT_ASSERT(v != nullptr);

        CPPUNIT_ASSERT(1024 == v->getData().size());
        CPPUNIT_ASSERT((uint8_t)(i % (126 - 32) + 33) == v->getData()[0]);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;

        ids.pop_front();
    }

    ds->close();
}

void DataStorageTests::testUpdateSignedValue() {
    auto ds = SqliteStorage::open(path1, scheduler);

    // new value: success
    std::string str = "Hello, world";
    std::vector<uint8_t> data1(str.cbegin(), str.cend());

    auto signedValue = node1->createSignedValue(data1);
    auto p1 = node1->storeValue(signedValue);
    auto result1 = p1.get();
    CPPUNIT_ASSERT_EQUAL(true, result1);

    auto valueId = signedValue->getId();
    std::cout << valueId << std::endl;

    ds->putValue(signedValue, 0);
    auto v = ds->getValue(valueId);

    CPPUNIT_ASSERT(*signedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    // update: invalid sequence number
    CPPUNIT_ASSERT_THROW(ds->putValue(v, 10), std::invalid_argument);

    // update: CAS fail
    CPPUNIT_ASSERT_THROW(ds->putValue(v, 9), std::invalid_argument);

    // should be the original value
    v = ds->getValue(valueId);
    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(*signedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    // update: success
    str = "Hello, world";
    std::vector<uint8_t> data2(str.cbegin(), str.cend());
    signedValue = node1->updateValue(valueId, data2);

    CPPUNIT_ASSERT(signedValue->getId() == valueId);

    ds->putValue(signedValue, 0);

    v = ds->getValue(valueId);
    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(*signedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    ds->close();
}

void DataStorageTests::testUpdateEncryptedValue() {
    auto ds = SqliteStorage::open(path1, scheduler);

    // new value: success
    auto to = node2->getId();
    std::string str = "Hello, world";
    std::vector<uint8_t> data1(str.cbegin(), str.cend());
    auto encryptedValue = node1->createEncryptedValue(to, data1);
    node1->storeValue(encryptedValue);

    auto valueId = encryptedValue->getId();
    std::cout << valueId << std::endl;

    ds->putValue(encryptedValue, 0);
    auto v = ds->getValue(valueId);

    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(*encryptedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    // update: invalid sequence number
    CPPUNIT_ASSERT_THROW(ds->putValue(v, 10), std::invalid_argument);

    // update: CAS fail
    CPPUNIT_ASSERT_THROW(ds->putValue(v, 9), std::invalid_argument);

    // should be the original value
    v = ds->getValue(valueId);

    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(*encryptedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    // update: success
    str = "Foo bar";
    std::vector<uint8_t> data2(str.cbegin(), str.cend());
    encryptedValue = node1->updateValue(valueId, data2);
    CPPUNIT_ASSERT(encryptedValue != nullptr);
    CPPUNIT_ASSERT(encryptedValue->getId() == valueId);

    ds->putValue(encryptedValue, 0);

    v = ds->getValue(valueId);
    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(*encryptedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    ds->close();
}

void DataStorageTests::testPutAndGetPeer() {
    auto ds = SqliteStorage::open(path1, scheduler);

    std::list<Id> ids {};

    std::vector<uint8_t> ipv4Addr = { 0x0a, 0x00, 0x00, 0x00 };
    std::vector<uint8_t> ipv6Addr = { (uint8_t)0xfc, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int basePort = 8000;

    std::cout << "Writing peers...";
    for (int i = 1; i <= 64; i++) {
        auto id = Id::random();
        ids.push_back(id);

        std::list<Sp<PeerInfo>> peers = {};
        for (int j = 0; j < i; j++) {
            increment(ipv4Addr);
            auto pi = std::make_shared<PeerInfo>(Id::random(), ipv4Addr, basePort + i);
            peers.push_back(pi);

            increment(ipv6Addr);
            pi = std::make_shared<PeerInfo>(Id::random(), ipv6Addr, basePort + i);
            peers.push_back(pi);
        }
        ds->putPeer(id, peers);
        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;
    }

    std::cout << std::endl << "Reading peers...";
    for (int i = 1; i <= 64; i++) {
        auto id = ids.front();

        // all
        auto ps = ds->getPeer(id, 10, 2 * i + 16);
        CPPUNIT_ASSERT(2 * i == ps.size());
        for (auto pi: ps)
            CPPUNIT_ASSERT(basePort + i == pi->getPort());

        ps = ds->getPeer(id, 4, i + 16);
        CPPUNIT_ASSERT(i == ps.size());
        for (auto pi : ps)
            CPPUNIT_ASSERT(basePort + i == pi->getPort());

        ps = ds->getPeer(id, 6, i + 16);
        CPPUNIT_ASSERT(i == ps.size());
        for (auto pi : ps)
            CPPUNIT_ASSERT(basePort + i == pi->getPort());

        // limited
        ps = ds->getPeer(id, 10, 32);
        CPPUNIT_ASSERT(std::min(2 * i, 64) == ps.size());
        for (auto pi : ps)
            CPPUNIT_ASSERT(basePort + i == pi->getPort());

        ps = ds->getPeer(id, 4, 32);
        CPPUNIT_ASSERT(std::min(i, 32) == ps.size());
        for (auto pi : ps)
            CPPUNIT_ASSERT(basePort + i == pi->getPort());

        ps = ds->getPeer(id, 6, 32);
        CPPUNIT_ASSERT(std::min(i, 32) == ps.size());
        for (auto pi : ps)
            CPPUNIT_ASSERT(basePort + i == pi->getPort());

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;

        ids.pop_front();
    }

    ds->close();
}
}  // namespace test
