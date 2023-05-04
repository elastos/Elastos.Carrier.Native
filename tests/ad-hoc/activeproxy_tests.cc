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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// std
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

// carrier
#include <carrier.h>
#include <utils.h>
#include <httplib.h>

#include "activeproxy_tests.h"

using namespace elastos::carrier;
using namespace httplib;
extern bool stopped;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(ActiveProxyTester);

#define TEST_COUNT 8000

void ActiveProxyTester::setUp() {

}

void ActiveProxyTester::testActiveProxy() {
    //Use AdminLTE-3.2.0
    auto host = "192.168.8.214";
    auto port = 20000;
    Client cli(host, port);

    std::thread threads[TEST_COUNT];
    cli.set_connection_timeout(2);

    for (int i = 0; i < TEST_COUNT; i++) {
        threads[i] = std::thread([=, &cli]() {
            std::cout << "-- Thread " << i << ": start --" << std::endl;

            auto res = cli.Get("/dist/css/adminlte.min.css"); //1.4M
            if (res->status == 200) {
                std::cout << res->body.size() << std::endl;
            } else {
                auto err = res.error();
                std::cout << "HTTP error: " << httplib::to_string(err) << std::endl;
            }

            std::cout << "-- Thread " << i << ": end --" << std::endl;
        });
    }


    for (int i = 0; i < TEST_COUNT; i++) {
        threads[i].join();
    }
}

void ActiveProxyTester::tearDown() {

}
}  // namespace test
