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

#include <string>
#include <algorithm>
#include <carrier.h>
#include "id_tests.h"

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(IdTests);

using namespace elastos::carrier;

void IdTests::test1() {
    auto pk = Signature::KeyPair::random().publicKey();
    auto id = Id(pk.blob());
    CPPUNIT_ASSERT(id.toSignatureKey() == pk);
    CPPUNIT_ASSERT(id == Id(pk));
}

void IdTests::test2() {
    auto pk = Signature::KeyPair::random().publicKey();
    auto id = Id(pk);
    CPPUNIT_ASSERT(id.toSignatureKey() == pk);
}

void IdTests::test3() {
    // Test constructor for ID string with "0x" prefix.
    auto idstring = "0x71e1b2ecdf528b623192f899d984c53f2b13508e21ccd53de5d7158672820636";
    auto id1 = Id(idstring);
    CPPUNIT_ASSERT(id1.toHexString() == idstring);

    auto id2 = Id::ofHex(idstring);
    CPPUNIT_ASSERT(id2.toHexString() == idstring);
    CPPUNIT_ASSERT(id1 == id2);
}

void IdTests::test4() {
    // Test constructor for ID string without "0x" prefix;
    auto idstring = "71e1b2ecdf528b623192f899d984c53f2b13508e21ccd53de5d7158672820636";
    auto id = Id::ofHex(idstring);
    CPPUNIT_ASSERT(id.toHexString().substr(2) == idstring);
}

void IdTests::test5() {
    std::string idstring = "F897B6CB7969005520E6F6101EB5466D9859926A51653365E36C4A3C42E5DE6F";
    auto id = Id::ofHex(idstring);
    std::transform(idstring.begin(), idstring.end(), idstring.begin(), tolower);
    CPPUNIT_ASSERT(id.toHexString().substr(2) == idstring);
}

void IdTests::test6() {
    Id id1 = Id::random();
    Id id2 = Id::random();
    Id id3 = Id(id1);

    CPPUNIT_ASSERT(id1 == id3);
    CPPUNIT_ASSERT(id2 != id3);
    CPPUNIT_ASSERT(id1.toHexString() == id3.toHexString());
    CPPUNIT_ASSERT(id2.toHexString() != id3.toHexString());
}

void IdTests::testOutOfRangeError() {
    auto idstring1 = "0x71E1B2ECDF528B623192F899D984C53F2B13508E21CCD53DE5D71586728206";
    CPPUNIT_ASSERT_THROW_MESSAGE("Hex ID string should be 64 characters long.",
        new Id(idstring1), std::out_of_range);

    auto idstring2 = "f897b6cb7969005520e6f6101eb5466d9859926a51653365e36c4a3c42e5de";
    CPPUNIT_ASSERT_THROW_MESSAGE("Hex ID string should be 64 characters long.",
        new Id(idstring2), std::out_of_range);
}

void IdTests::testDomainError() {
    auto idstring1 = "0x71E1B2ECDR528B623192F899D984C53F2B13508E21CCD53DE5D7158672820636";
    CPPUNIT_ASSERT_THROW_MESSAGE("not an hex character", new Id(idstring1), std::domain_error);

    auto idstring2 = "f897b6cb7969005520e6f6101ebx466d9859926a51653365e36c4a3c42e5de6f";
    CPPUNIT_ASSERT_THROW_MESSAGE("not an hex character", Id::ofHex(idstring2), std::domain_error);

    std::array<uint8_t, 20> binId;
    std::generate_n(binId.begin(), 20, []{ return 1; });
    CPPUNIT_ASSERT_THROW_MESSAGE("Binary id should be 32 bytes long.", new Id(binId), std::invalid_argument);
}

void IdTests::testDistance() {
    Id id1 = Id("0x00000000f528d6132c15787ed16f09b08a4e7de7e2c5d3838974711032cb7076");
    Id id2 = Id("0x00000000f0a8d6132c15787ed16f09b08a4e7de7e2c5d3838974711032cb7076");
    auto distance = "0x0000000005800000000000000000000000000000000000000000000000000000";

    CPPUNIT_ASSERT(Id::distance(id1,id2) == Id(distance));
    CPPUNIT_ASSERT(Id::distance(id1, id2).toHexString() == distance);
}

void IdTests::testThreeWayCompare() {
    Id id0 = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a886a8ca214a3d09b6676cb8");
    Id id1 = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a886a8fa214a3d09b6676cb8");
    Id id2 = Id("0x4833af415161cbd0a3ef83aa59a55fbadc9bd520a885a8ca214a3d09b6676cb8");

    CPPUNIT_ASSERT(id0.threeWayCompare(id1, id2) < 0);
    CPPUNIT_ASSERT(id0.threeWayCompare(id1, id1) == 0);
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

}  // namespace test
