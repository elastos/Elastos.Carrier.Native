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
#include <stddef.h>
#include <signal.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <io.h>
#include <process.h>
#include <debugapi.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <CLI/CLI.hpp>

using namespace std;

bool stopped = false;

struct Options {
    bool wait_for_attach {false};
};

#ifdef HAVE_SYS_RESOURCE_H
int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

void signal_handler(int signum)
{
    stopped = true;
    // exit(-1);
}

static Options parseArgs(int argc, char **argv)
{
    Options options;

    CLI::App app("Elastos Carrier functions test case", "functions tests");
    app.add_option("-d, --debug", options.wait_for_attach, "Wait for debugger to attach");

    try {
        app.parse(argc, argv);
    } catch (const CLI::Error &e) {
        int rc = app.exit(e);
        std::exit(rc);
    }

    return options;
}

int main(int argc, char* argv[])
{
#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    auto options = parseArgs(argc, argv);
    if (options.wait_for_attach){
        printf("Wait for debugger attaching, process id is: %d.\n", getpid());
#ifndef _MSC_VER
        printf("After debugger attached, press any key to continue......");
        getchar();
        printf("Attached, press any key to continue......");
        getchar();
#else
        DebugBreak();
#endif
    }

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
#ifdef HAVE_SIGKILL
    signal(SIGKILL, signal_handler);
#endif
#ifdef HAVE_SIGHUP
    signal(SIGHUP, signal_handler);
#endif

    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    CppUnit::Test *suite = registry.makeTest();
    if (suite->countTestCases() == 0) {
        std::cout << "No test cases specified for suite" << std::endl;
        return 1;
    }
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);
    auto result = runner.run() ? 0 : 1;

    return result;
}
