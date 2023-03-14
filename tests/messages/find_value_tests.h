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

#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "message_tests.h"

namespace test {
class FindValueTests : public MessageTests, public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(FindValueTests);
    CPPUNIT_TEST(testFindValueRequestSize);
    CPPUNIT_TEST(testFindValueRequest4);
    CPPUNIT_TEST(testFindValueRequest6);
    CPPUNIT_TEST(testFindValueRequest46);
    CPPUNIT_TEST(testFindValueResponse4);
    CPPUNIT_TEST(testFindValueResponse4Immutable);
    CPPUNIT_TEST(testFindValueResponse6);
    CPPUNIT_TEST(testFindValueResponse46);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testFindValueRequestSize();
    void testFindValueRequest4();
    void testFindValueRequest6();
    void testFindValueRequest46();
    void testFindValueResponseSize();
    void testFindValueResponse4();
    void testFindValueResponse4Immutable();
    void testFindValueResponse6();
    void testFindValueResponse46();
};
}
