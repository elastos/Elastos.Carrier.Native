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

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <iostream>
#include <stddef.h>

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

extern "C" {
    #include <signal.h>
    #include <unistd.h>
    #include <getopt.h>
}

using namespace std;

int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}

void signal_handler(int signum)
{
    exit(-1);
}

void usage(void)
{
    cout << "apitests" << endl;
    cout << "Usage: didtest [OPTION]..." << endl;
    cout << "" << endl;
    cout << "      --debug                      Wait for debugger to attach" << endl;
    cout << "" << endl;
}

int main(int argc, char* argv[])
{
    int wait_for_attach = 0;
    int opt;
    int idx;

    struct option options[] = {
        { "help",           no_argument,        NULL, 'h' },
        { "debug",          no_argument,        NULL,  1  },
        { NULL,             0,                  NULL,  0  }
    };

    sys_coredump_set(true);

    while ((opt = getopt_long(argc, argv, "h?", options, &idx)) != -1) {
        switch (opt) {
        case 1:
            wait_for_attach = 1;
            break;

        case 'h':
        case '?':
        default:
            usage();
            return -1;
        }
    }

    if (wait_for_attach) {
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
