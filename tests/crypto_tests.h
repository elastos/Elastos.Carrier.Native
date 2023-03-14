#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace test {

class CryptoTester : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CryptoTester);
    CPPUNIT_TEST(testEncryption);
    CPPUNIT_TEST(testSignatrue);
    CPPUNIT_TEST(testPublicKey);
    CPPUNIT_TEST(testCrytoContext);
    CPPUNIT_TEST_SUITE_END();

 public:
    void setUp();
    void tearDown();

    void testEncryption();
    void testSignatrue();
    void testPublicKey();
    void testCrytoContext();
};

}  // namespace test
