/*
 * Copyright (c) 2023 trinity-tech.io
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
#include <cstdlib>
#include <csignal>
#include <chrono>
#include <thread>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <CLI/CLI.hpp>

#include <IP2Location.h>

#include "crawler.h"

static int interrupted = 0;

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>

int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}
#endif

struct Options {
    bool help {false}; // print help and exit
    bool version {false};
    std::string configFile {};
};

static void printVersion()
{
    std::cout << "Elastos Carrier Crawler version " << "1.0" << std::endl;
}

static Options parseArgs(int argc, char **argv)
{
    Options options;
    bool version {false};

    CLI::App app("Elastos Carrier Crawler", "crawler");
    app.add_option("-c, --config", options.configFile, "The configuration file.");
    app.add_flag("-v, --version", version, "Show the Carrier version.");

    try {
        app.parse(argc, argv);
    } catch (const CLI::Error &e) {
        int rc = app.exit(e);
        std::exit(rc);
    }

    if (version) {
        printVersion();
        std::exit(0);
    }

    return options;
}

static void signal_handler(int sig)
{
    // logger->info("Controller - INT signal catched, interrupte all crawlers.");
    interrupted = 1;
}

static void setupSignals()
{
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
#ifdef HAVE_SIGKILL
    signal(SIGKILL, signal_handler);
#endif
#ifdef HAVE_SIGCHLD
    signal(SIGCHLD, SIG_IGN); /* ignore child */
#endif
#ifdef HAVE_SIGTSTP
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
#endif
#ifdef HAVE_SIGTTOU
    signal(SIGTTOU, SIG_IGN);
#endif
#ifdef HAVE_SIGTTIN
    signal(SIGTTIN, SIG_IGN);
#endif
#ifdef HAVE_SIGHUP
    signal(SIGHUP, SIG_IGN);
#endif
}

int main(int argc, char **argv)
{

#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    auto options = parseArgs(argc, argv);

    setupSignals();

    Crawler crawler {};

    try {
        crawler.init(options.configFile);
    }
    catch (const std::overflow_error& e) {
        std::cout << e.what() << '\n';
        return -1;
    }

    crawler.start();

    while (interrupted == 0 && !crawler.finished()) {
        crawler.sendNodeRequests();
        sleep(1);
    }

    crawler.stop();

    return (interrupted == 2) ? 0 : 1;
}
