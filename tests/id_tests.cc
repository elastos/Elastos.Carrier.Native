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

#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <random>

#include <carrier/id.h>

#include "utils/hex.h"
#include "id_tests.h"

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(IdTests);

typedef elastos::carrier::Id    Id;
typedef elastos::carrier::Hex   Hex;

void
IdTests::setUp() {
}

void IdTests::testIdFromHexString() {
    std::string hexWithPrefix = "0x71e1b2ecdf528b623192f899d984c53f2b13508e21ccd53de5d7158672820636";
    Id id = Id(hexWithPrefix);
    CPPUNIT_ASSERT(hexWithPrefix == id.toHexString());

    std::string hexWithoutPrefix = "F897B6CB7969005520E6F6101EB5466D9859926A51653365E36C4A3C42E5DE6F";
    id = Id::ofHex(hexWithoutPrefix);
    std::transform(hexWithoutPrefix.begin(), hexWithoutPrefix.end(), hexWithoutPrefix.begin(), tolower);
    CPPUNIT_ASSERT(hexWithoutPrefix == id.toHexString().substr(2));

    std::string hexWithPrefix2 = "0x71E1B2ECDF528B623192F899D984C53F2B13508E21CCD53DE5D71586728206";
    CPPUNIT_ASSERT_THROW_MESSAGE("Hex ID string should be 64 characters long.", new Id(hexWithPrefix2), std::out_of_range);

    std::string hexWithoutPrefix2 = "f897b6cb7969005520e6f6101eb5466d9859926a51653365e36c4a3c42e5de";
    CPPUNIT_ASSERT_THROW_MESSAGE("Hex ID string should be 64 characters long.", Id::ofHex(hexWithoutPrefix2), std::out_of_range);

    std::string hexWithPrefix3 = "0x71E1B2ECDR528B623192F899D984C53F2B13508E21CCD53DE5D7158672820636";
    CPPUNIT_ASSERT_THROW_MESSAGE("not an hex character", new Id(hexWithPrefix3), std::domain_error);

    std::string hexWithoutPrefix3 = "f897b6cb7969005520e6f6101ebx466d9859926a51653365e36c4a3c42e5de6f";
    CPPUNIT_ASSERT_THROW_MESSAGE("not an hex character", Id::ofHex(hexWithoutPrefix3), std::domain_error);
}

void IdTests::testIdFromBytes() {
    std::random_device rd;
    std::uniform_int_distribution<uint8_t> dist(0, 9);

    std::array<uint8_t, ID_BYTES> binId;
    std::generate_n(binId.begin(), ID_BYTES, [&]{ return dist(rd); });
    auto id = Id(binId);

    auto hexStr = id.toHexString().substr(2);
    CPPUNIT_ASSERT(std::memcmp(binId.data(), Hex::decode(hexStr.c_str(), hexStr.size()).data(), ID_BYTES) == 0);

    std::array<uint8_t, 20> binId2;
    std::generate_n(binId2.begin(), 20, [&]{ return dist(rd); });
    CPPUNIT_ASSERT_THROW_MESSAGE("Binary id should be 32 bytes long.", new Id(binId2), std::invalid_argument);
}

void IdTests::testIdFromId() {
    Id id1 = Id::random();
    Id id2 = Id(id1);

    CPPUNIT_ASSERT(id1.toHexString() == id2.toHexString());
    CPPUNIT_ASSERT(id1 == id2);

    id2 = Id::random();
    CPPUNIT_ASSERT(id1 != id2);
}

void IdTests::testDistance() {
    Id id1 = Id("0x00000000f528d6132c15787ed16f09b08a4e7de7e2c5d3838974711032cb7076");
    Id id2 = Id("0x00000000f0a8d6132c15787ed16f09b08a4e7de7e2c5d3838974711032cb7076");

    CPPUNIT_ASSERT("0x0000000005800000000000000000000000000000000000000000000000000000" == Id::distance(id1, id2).toHexString());

    //todo: test
    /*for (int i = 0; i < 1000; i++) {
        id1 = Id.random();
        id2 = Id.random();

        Id id3 = id1.distance(id2);
        BigInteger n = id1.toInteger().xor(id2.toInteger());
        assertTrue(id3.toInteger().equals(n));
    }*/
    //end test
}

void IdTests::testThreeWayCompare() {
    Id id = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a886a8ca214a3d09b6676cb8");
    Id id1 = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");
    Id id2 = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a885a8ca214a3d09b6676cb8");

    CPPUNIT_ASSERT(id.threeWayCompare(id1, id2) < 0);

    id1 = Id("0xf833af415161cbd0a3ef83aa59a55fbadc9bd520b886a8ca214a3d09b6676cb8");
    id2 = Id("0xf833af415161cbd0a3ef83aa59a55fbadc9bd520b886a8ca214a3d09b6676cb8");

    CPPUNIT_ASSERT(id.threeWayCompare(id1, id2) == 0);

    id1 = Id("0x4833af415161cbd0a3ef83aa59a55f1adc9bd520a886a8ca214a3d09b6676cb8");
    id2 = Id("0x4833af415161cbd0a3ef83aa59a55fcadc9bd520a886a8ca214a3d09b6676cb8");

    CPPUNIT_ASSERT(id.threeWayCompare(id1, id2) > 0);

    //todo: test
    /*for (int i = 0; i < 1000; i++) {
        id1 = Id.random();
        id2 = Id.random();
        int d = id.threeWayCompare(id1, id2);

        Id d1 = id.distance(id1);
        Id d2 = id.distance(id2);
        int n = d1.toInteger().compareTo(d2.toInteger());

        assertEquals(n, d);
    }*/
    //end test
}

void IdTests::testBitsEqual() {
    Id id1 = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");
    Id id2 = Id("0x4833af415166cbd0a3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");

    for (int i = 0; i < 45; i++)
        CPPUNIT_ASSERT(Id::bitsEqual(id1, id2, i));

    for (int i = 45; i < ID_BITS; i++)
        CPPUNIT_ASSERT(Id::bitsEqual(id1, id2, i) == false);

    id2 = Id(id1);
    for (int i = 0; i < ID_BITS; i++)
        CPPUNIT_ASSERT(Id::bitsEqual(id1, id2, i));

    id2 = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb9");

    for (int i = 0; i < ID_BITS - 1; i++)
        CPPUNIT_ASSERT(Id::bitsEqual(id1, id2, i));

    CPPUNIT_ASSERT(Id::bitsEqual(id1, id2, ID_BITS - 1) == false);
}

void IdTests::testBitsCopy() {
    Id id1 = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");

    for (int i = 0; i > ID_BITS; i++) {
        Id id2 = Id::random();

        Id::bitsCopy(id1,id2,i);
        CPPUNIT_ASSERT(Id::bitsEqual(id1, id2, i));
    }
}

void
IdTests::tearDown() {
}
}  // namespace test
