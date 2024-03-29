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

#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace test {

class IdTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(IdTests);
    CPPUNIT_TEST(test1);
    CPPUNIT_TEST(test2);
    CPPUNIT_TEST(test3);
    CPPUNIT_TEST(test4);
    CPPUNIT_TEST(test5);
    CPPUNIT_TEST(test6);
    CPPUNIT_TEST(testOutOfRangeError);
    CPPUNIT_TEST(testDomainError);
    CPPUNIT_TEST(testDistance);
    CPPUNIT_TEST(testThreeWayCompare);
    CPPUNIT_TEST(testBitsEqual);
    CPPUNIT_TEST(testBitsCopy);
    CPPUNIT_TEST_SUITE_END();

 public:
    void setUp() {}
    void tearDown() {}

    void test1();
    void test2();
    void test3();
    void test4();
    void test5();
    void test6();
    void testOutOfRangeError();
    void testDomainError();
    void testDistance();
    void testThreeWayCompare();
    void testBitsEqual();
    void testBitsCopy();
};

}  // namespace test
