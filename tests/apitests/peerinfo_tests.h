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

class PeerInfoTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(PeerInfoTests);
    CPPUNIT_TEST(testFalsity);
    CPPUNIT_TEST(testCreate1);
    CPPUNIT_TEST(testCreate2);
    CPPUNIT_TEST(testCreate3);
    CPPUNIT_TEST(testCreate4);
    CPPUNIT_TEST(testCreate5);
    CPPUNIT_TEST(testCreate6);
    CPPUNIT_TEST(testCreate7);
    CPPUNIT_TEST(testCreate8);
    CPPUNIT_TEST(testEqualOperator);
    CPPUNIT_TEST_SUITE_END();

 public:
    void setUp() {}
    void tearDown() {}

    void testFalsity();
    void testCreate1();
    void testCreate2();
    void testCreate3();
    void testCreate4();
    void testCreate5();
    void testCreate6();
    void testCreate7();
    void testCreate8();
    void testEqualOperator();
};

}  // namespace test
