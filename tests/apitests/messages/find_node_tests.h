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

#include "message_tests.h"

namespace test {
class FindNodeTests : public MessageTests, public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(FindNodeTests);
    CPPUNIT_TEST(testFindNodeRequestSize);
    CPPUNIT_TEST(testFindNodeRequest4);
    CPPUNIT_TEST(testFindNodeRequest4WithAt);
    CPPUNIT_TEST(testFindNodeRequest6);
    CPPUNIT_TEST(testFindNodeRequest6WithAt);
    CPPUNIT_TEST(testFindNodeRequest46);
    CPPUNIT_TEST(testFindNodeRequest46WithAt);
    CPPUNIT_TEST(testFindNodeResponseSize);
    CPPUNIT_TEST(testFindNodeResponse4);
    CPPUNIT_TEST(testFindNodeResponse4WithToken);
    CPPUNIT_TEST(testFindNodeResponse6);
    CPPUNIT_TEST(testFindNodeResponse6WithToken);
    CPPUNIT_TEST(testFindNodeResponse46);
    CPPUNIT_TEST(testFindNodeResponse46WithToken);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testFindNodeRequestSize();
    void testFindNodeRequest4();
    void testFindNodeRequest4WithAt();
    void testFindNodeRequest6();
    void testFindNodeRequest6WithAt();
    void testFindNodeRequest46();
    void testFindNodeRequest46WithAt();
    void testFindNodeResponseSize();
    void testFindNodeResponse4();
    void testFindNodeResponse4WithToken();
    void testFindNodeResponse6();
    void testFindNodeResponse6WithToken();
    void testFindNodeResponse46();
    void testFindNodeResponse46WithToken();
};
}
