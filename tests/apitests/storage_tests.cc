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
#include <string>
#include <filesystem>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <carrier.h>
#include "utils/list.h"

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
    path = Utils::getPwdStorage("storagetests_out");
    Utils::removeStorage(path);

    path1 = path + Utils::PATH_SEP + "carrier1";
    path2 = path + Utils::PATH_SEP + "carrier2";
    path3 = path + Utils::PATH_SEP + "carrier3";

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

void DataStorageTests::tearDown() {
    if (node2)
        node2->stop();
    if (node1)
        node1->stop();

    Utils::removeStorage(path);
}

void DataStorageTests::testPutAndGetValue() {
    auto ds = SqliteStorage::open(path3, scheduler);

    std::list<Id> ids {};
    std::vector<uint8_t> data(1024);

    std::cout << "Writing values...";
    for (int i = 1; i <= 256; i++) {
        std::vector<uint8_t> data;
        data.resize(1024);
        std::fill(data.begin(), data.end(), (uint8_t)(i % (126 - 32) + 33));
        auto v = Value::createValue(data);

        ids.push_back(v.getId());
        ds->putValue(v);

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

        auto removed = ds->removeValue(id);
        CPPUNIT_ASSERT(removed);

        v = ds->getValue(id);
        CPPUNIT_ASSERT(v == nullptr);

        removed = ds->removeValue(id);
        // CPPUNIT_ASSERT(!removed);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;

        ids.pop_front();
    }

    ds->close();
}

void DataStorageTests::testPutAndGetPersistentValue() {
    auto ds = SqliteStorage::open(path3, scheduler);

    std::list<Id> ids {};
    std::vector<uint8_t> data(1024);

    std::cout << "Writing values...";
    for (int i = 1; i <= 256; i++) {
        Random::buffer(data);
        data[0] = (uint8_t)(i % (126 - 32) + 33);
        auto v = Value::createValue(data);

        ids.push_back(v.getId());
        ds->putValue(v, i % 2 == 0);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;
    }

    auto ts = currentTimeMillis();
    auto values = ds->getPersistentValues(ts);
    CPPUNIT_ASSERT_EQUAL((size_t)128, values.size());

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "\nUpdate the last announced for values..." << std::endl;
    for (int i = 1; i <= 128; i++) {
        Id id = list_get(values, i - 1).getId();

        if (i % 2 == 0)
            ds->updateValueLastAnnounce(id);
    }

    values = ds->getPersistentValues(ts);
    CPPUNIT_ASSERT_EQUAL((size_t)64, values.size());

    std::cout << "\nReading values...";
    for (int i = 1; i <= 256; i++) {
        auto id = ids.front();
        auto v = ds->getValue(id);
        CPPUNIT_ASSERT(v != nullptr);

        CPPUNIT_ASSERT(1024 == v->getData().size());
        CPPUNIT_ASSERT((uint8_t)(i % (126 - 32) + 33) == v->getData()[0]);

        auto removed = ds->removeValue(id);
        CPPUNIT_ASSERT(removed);

        v = ds->getValue(id);
        CPPUNIT_ASSERT(v);

        removed = ds->removeValue(id);
        CPPUNIT_ASSERT(!removed);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;

        ids.pop_front();
    }

    ds->close();
}

void DataStorageTests::testUpdateSignedValue() {
    auto ds = SqliteStorage::open(path3, scheduler);

    // new value: success
    std::string str = "Hello, world";
    std::vector<uint8_t> data1(str.cbegin(), str.cend());

    auto signedValue = Value::createSignedValue(data1);
    auto p1 = node1->storeValue(signedValue);
    p1.get();

    auto valueId = signedValue.getId();
    std::cout << valueId << std::endl;

    ds->putValue(signedValue, 0);
    auto v = ds->getValue(valueId);

    CPPUNIT_ASSERT(signedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    // update: invalid sequence number
    CPPUNIT_ASSERT_THROW(ds->putValue(*v, 10), std::invalid_argument);

    // update: CAS fail
    CPPUNIT_ASSERT_THROW(ds->putValue(*v, 9), std::invalid_argument);

    // should be the original value
    v = ds->getValue(valueId);
    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(signedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    // update: success
    str = "Hello, world";
    std::vector<uint8_t> data2(str.cbegin(), str.cend());
    signedValue = signedValue.update(data2);

    CPPUNIT_ASSERT(signedValue.getId() == valueId);

    ds->putValue(signedValue, 0);

    v = ds->getValue(valueId);
    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(signedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    ds->close();
}

void DataStorageTests::testUpdateEncryptedValue() {
    auto ds = SqliteStorage::open(path3, scheduler);

    // new value: success
    auto to = node2->getId();
    std::string str = "Hello, world";
    std::vector<uint8_t> data1(str.cbegin(), str.cend());
    auto encryptedValue = Value::createEncryptedValue(to, data1);
    node1->storeValue(encryptedValue);

    auto valueId = encryptedValue.getId();
    std::cout << valueId << std::endl;

    ds->putValue(encryptedValue, 0);
    auto v = ds->getValue(valueId);

    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(encryptedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    // update: invalid sequence number
    CPPUNIT_ASSERT_THROW(ds->putValue(*v, 10), std::invalid_argument);

    // update: CAS fail
    CPPUNIT_ASSERT_THROW(ds->putValue(*v, 9), std::invalid_argument);

    // should be the original value
    v = ds->getValue(valueId);

    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(encryptedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    // update: success
    str = "Foo bar";
    std::vector<uint8_t> data2(str.cbegin(), str.cend());
    encryptedValue = encryptedValue.update(data2);
    CPPUNIT_ASSERT(encryptedValue.getId() == valueId);

    ds->putValue(encryptedValue, 0);

    v = ds->getValue(valueId);
    CPPUNIT_ASSERT(nullptr != v);
    CPPUNIT_ASSERT(encryptedValue == *v);
    CPPUNIT_ASSERT(v->isValid());

    ds->close();
}

void DataStorageTests::testPutAndGetPeer() {
    auto ds = SqliteStorage::open(path3, scheduler);

    std::map<Id, std::list<PeerInfo>> allPeers {};

    std::list<Id> ids {};
    int basePort = 8000;

    std::cout << "Writing peers...";
    std::vector<uint8_t> sig(64, 5);
    for (int i = 1; i <= 64; i++) {
        auto id = Id::random();
        ids.push_back(id);

        std::list<PeerInfo> peers = {};
        for (int j = 0; j < i; j++) {
            auto pi = PeerInfo::of(Id::random().blob(), {}, Id::random().blob(), {}, basePort + i, "alt:" + std::to_string(i), sig);
            peers.push_back(pi);
        }

        allPeers.emplace(id, peers);

        ds->putPeer(peers);
        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;
    }

    std::cout << std::endl << "Reading peers...";
    int total = 0;
    for (auto entry : allPeers) {
        total++;
        auto id = entry.first;
        auto peers = entry.second;

        //all
        auto ps = ds->getPeer(id, peers.size() + 8);
        CPPUNIT_ASSERT_EQUAL(peers.size(), ps.size());

        auto c = [&](const PeerInfo& a, const PeerInfo& b) {
            int r = a.getNodeId().compareTo(b.getNodeId());
            if (r != 0)
                return r;

            return a.getOrigin().compareTo(b.getOrigin());
        };

        peers.sort(c);
        ps.sort(c);
        for (int i = 0; i < peers.size(); i++) {
            CPPUNIT_ASSERT_EQUAL(list_get(peers, i), list_get(ps, i));
        }

        // limited
        ps = ds->getPeer(id, 16);
        CPPUNIT_ASSERT_EQUAL(std::min<size_t>(16, peers.size()), ps.size());
        for (PeerInfo pi : ps)
            CPPUNIT_ASSERT_EQUAL(list_get(peers, 0).getPort(), pi.getPort());

        for (auto p : peers) {
            auto pi = ds->getPeer(p.getId(), p.getOrigin());
            CPPUNIT_ASSERT(pi != nullptr);
            CPPUNIT_ASSERT_EQUAL(p, *pi);

            auto removed = ds->removePeer(p.getId(), p.getOrigin());
            CPPUNIT_ASSERT(removed);

            pi = ds->getPeer(p.getId(), p.getOrigin());
            CPPUNIT_ASSERT(pi);

            removed = ds->removePeer(p.getId(), p.getOrigin());
            CPPUNIT_ASSERT(!removed);
        }

        std::cout << ".";
        if (total % 16 == 0)
            std::cout << std::endl;

        ids.pop_front();
    }

    ds->close();
}

void DataStorageTests::testPutAndGetPersistentPeer() {
    auto ds = SqliteStorage::open(path3, scheduler);

    std::list<Id> ids {};
    auto nodeId = Id::random();
    uint16_t basePort = 8000;
    std::vector<uint8_t> sig(64);

    std::cout << "Writing peers...";
    for (int i = 1; i <= 256; i++) {
        Random::buffer(sig);
        auto pi = PeerInfo::of(Id::random().blob(), {}, nodeId.blob(), {}, basePort + i, {}, sig);

        ids.push_back(pi.getId());
        ds->putPeer(pi, i % 2 == 0);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;
    }

    auto ts = currentTimeMillis();
    std::list<PeerInfo> peers = ds->getPersistentPeers(ts);
    CPPUNIT_ASSERT_EQUAL((size_t)128, peers.size());

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "\nUpdate the last announced for peers...";
    for (int i = 1; i <= 128; i++) {
        Id id = list_get(peers, i - 1).getId();

        if (i % 2 == 0)
            ds->updatePeerLastAnnounce(id, nodeId);
    }

    peers = ds->getPersistentPeers(ts);
    CPPUNIT_ASSERT_EQUAL((size_t)64, peers.size());

    std::cout << "\nReading values...";
    for (int i = 1; i <= 256; i++) {
        auto id = ids.front();
        auto p = ds->getPeer(id, nodeId);
        CPPUNIT_ASSERT(p != nullptr);

        CPPUNIT_ASSERT_EQUAL((uint16_t)(basePort + i),  p->getPort());

        auto removed = ds->removePeer(id, nodeId);
        CPPUNIT_ASSERT(removed);

        p = ds->getPeer(id, nodeId);
        CPPUNIT_ASSERT(p == nullptr);

        removed = ds->removePeer(id, nodeId);
        // CPPUNIT_ASSERT(!removed);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;

        ids.pop_front();
    }

    ds->close();
}

}  // namespace test
