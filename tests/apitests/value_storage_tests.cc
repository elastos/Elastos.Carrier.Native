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
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <carrier.h>

#include "sqlite_storage.h"
#include "utils/list.h"
#include "utils.h"
#include "value_storage_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(ValueStorageTests);

void ValueStorageTests::setUp() {
    path = Utils::getPwdStorage("apitests.db");
}

void ValueStorageTests::tearDown() {
    Utils::removeStorage(path);
}

void ValueStorageTests::testPutAndGetValue() {
    auto storage = SqliteStorage::open(path, scheduler);
    std::list<Id> valueIds {};

    std::cout << "Writing values...";
    for (int i = 1; i <= 256; i++) {
        auto data = Utils::getRandomData(1024);
        data[0] = (uint8_t)(i % (126 - 32) + 33);
        auto value = Value::createValue(data);

        valueIds.push_back(value.getId());
        storage->putValue(value);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;
    }

    std::cout << "Reading values...";
    for (int i = 1; i <= 256; i++) {
        auto valueId = valueIds.front();
        auto value = storage->getValue(valueId);
        CPPUNIT_ASSERT(value);
        CPPUNIT_ASSERT(value->getId() == valueId);
        CPPUNIT_ASSERT(value->getData().size() == 1024);
        CPPUNIT_ASSERT(value->getData()[0] == (uint8_t)(i % (126 - 32) + 33));

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;

        valueIds.pop_front();
    }

    storage->close();
}

void ValueStorageTests::testPutAndGetPersistentValue() {
    auto storage = SqliteStorage::open(path, scheduler);
    std::list<Id> valueIds {};

    std::cout << "Writing values...";
    for (int i = 1; i <= 256; i++) {
        auto data = Utils::getRandomData(1024);
        data[0] = (uint8_t)(i % (126 - 32) + 33);
        auto value = Value::createValue(data);

        valueIds.push_back(value.getId());
        storage->putValue(value, i % 2 == 0);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;
    }

    auto ts = currentTimeMillis();
    auto values = storage->getPersistentValues(ts);
    CPPUNIT_ASSERT(values.size() == 128);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "\nUpdate the last announced for values..." << std::endl;
    for (int i = 1; i <= 128; i++) {
        Id valueId = list_get(values, i - 1).getId();

        if (i % 2 == 0)
            storage->updateValueLastAnnounce(valueId);
    }

    values = storage->getPersistentValues(ts);
    CPPUNIT_ASSERT(values.size() == 64);

    std::cout << "\nReading values...";
    for (int i = 1; i <= 256; i++) {
        auto valueId = valueIds.front();
        auto value = storage->getValue(valueId);
        CPPUNIT_ASSERT(value);
        CPPUNIT_ASSERT(value->getId() == valueId);
        CPPUNIT_ASSERT(value->getData().size() == 1024);
        CPPUNIT_ASSERT(value->getData()[0] == (uint8_t)(i % (126 - 32) + 33));

        auto removed = storage->removeValue(valueId);
        CPPUNIT_ASSERT(removed);

        value = storage->getValue(valueId);
        CPPUNIT_ASSERT(!value);

        removed = storage->removeValue(valueId);
        CPPUNIT_ASSERT(!removed);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;

        valueIds.pop_front();
    }

    storage->close();
}

void ValueStorageTests::testUpdateSignedValue() {
    auto storage = SqliteStorage::open(path, scheduler);

    // new value: success
    std::string str = "Hello, world";
    std::vector<uint8_t> data1(str.cbegin(), str.cend());

    auto signedValue = Value::createSignedValue(data1);

    storage->putValue(signedValue, 0);
    auto valueId = signedValue.getId();
    auto value = storage->getValue(valueId);

    CPPUNIT_ASSERT(*value == signedValue);
    CPPUNIT_ASSERT(value->isValid());
    CPPUNIT_ASSERT(value->getId() == valueId);

    // update: invalid sequence number
    CPPUNIT_ASSERT_THROW(storage->putValue(*value, 10), std::invalid_argument);

    // update: CAS fail
    CPPUNIT_ASSERT_THROW(storage->putValue(*value, 9), std::invalid_argument);

    // should be the original value
    value = storage->getValue(valueId);
    CPPUNIT_ASSERT(value);
    CPPUNIT_ASSERT(*value == signedValue);
    CPPUNIT_ASSERT(value->isValid());
    CPPUNIT_ASSERT(value->getId() == valueId);

    // update: success
    str = "Hello, world2";
    std::vector<uint8_t> data2(str.cbegin(), str.cend());
    signedValue = signedValue.update(data2);
    CPPUNIT_ASSERT(signedValue.getId() == valueId);

    storage->putValue(signedValue, 0);

    value = storage->getValue(valueId);
    CPPUNIT_ASSERT(value);
    CPPUNIT_ASSERT(*value == signedValue);
    CPPUNIT_ASSERT(value->isValid());
    CPPUNIT_ASSERT(value->getId() == valueId);

    storage->close();
}

void ValueStorageTests::testUpdateEncryptedValue() {
    auto storage = SqliteStorage::open(path, scheduler);

    // new value: success
    std::string str = "Hello, world";
    std::vector<uint8_t> data1(str.cbegin(), str.cend());
    auto recipient = Signature::KeyPair::random().publicKey();
    //auto recipient = node2->getId();
    auto encryptedValue = Value::createEncryptedValue(recipient, data1);

    storage->putValue(encryptedValue, 0);
    auto valueId = encryptedValue.getId();
    auto value = storage->getValue(valueId);

    CPPUNIT_ASSERT(value);
    CPPUNIT_ASSERT(*value == encryptedValue);
    CPPUNIT_ASSERT(value->isValid());
    CPPUNIT_ASSERT(value->getId() == valueId);

    // update: invalid sequence number
    CPPUNIT_ASSERT_THROW(storage->putValue(*value, 10), std::invalid_argument);

    // update: CAS fail
    CPPUNIT_ASSERT_THROW(storage->putValue(*value, 9), std::invalid_argument);

    // should be the original value
    value = storage->getValue(valueId);
    CPPUNIT_ASSERT(value);
    CPPUNIT_ASSERT(*value == encryptedValue);
    CPPUNIT_ASSERT(value->isValid());

    // update: success
    str = "Foo bar";
    std::vector<uint8_t> data2(str.cbegin(), str.cend());
    encryptedValue = encryptedValue.update(data2);
    CPPUNIT_ASSERT(encryptedValue.getId() == valueId);

    storage->putValue(encryptedValue, 0);

    value = storage->getValue(valueId);
    CPPUNIT_ASSERT(value);
    CPPUNIT_ASSERT(*value == encryptedValue);
    CPPUNIT_ASSERT(value->isValid());

    storage->close();
}

}  // namespace test
