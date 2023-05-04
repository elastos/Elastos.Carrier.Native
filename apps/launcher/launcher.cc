/*
 *  Copyright (c) 2022 - 2023 trinity-tech.io
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

#include <carrier.h>
#include <application_lock.h>
#include <coredump.h>

using namespace std::chrono_literals;
using namespace elastos::carrier;

static Sp<Node> g_node = nullptr;
static bool broke = false;
static ApplicationLock lock;

struct Options {
    bool help {false}; // print help and exit
    bool version {false};
    bool daemonize {false};
    std::string addr4 {};
    std::string addr6 {};
    uint16_t port {0};
    std::string configFile {};
    std::string dataDir {};
};

static void printVersion()
{
    std::cout << "Elastos Carrier version " << version() << std::endl;
}

static Options parseArgs(int argc, char **argv)
{
    Options options;
    bool version {false};

    CLI::App app("Elastos Carrier Launcher", "launcher");
    app.add_option("-c, --config", options.configFile, "The configuration file.");
    app.add_option("-4, --address4", options.addr4, "IPv4 address to listen.");
    app.add_option("-6, --address6", options.addr6, "IPv6 address to listen.");
    app.add_option("-p, --port", options.port, "The port to listen.");
    app.add_option("-d, --data-dir", options.dataDir, "The directory to store the node data.");
    app.add_flag("-D, --daemonize", options.daemonize, "Run in daemonize mode.");
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

static Sp<Configuration> initConfigure(Options& options)
{
    auto builder = DefaultConfiguration::Builder{};

    if (!options.configFile.empty())
        builder.load(options.configFile);

    if (!options.addr4.empty())
        builder.setIPv4Address(options.addr4);

    if (!options.addr6.empty())
        builder.setIPv6Address(options.addr6);

    if (options.port > 0)
        builder.setListeningPort(options.port);

    if (!options.dataDir.empty())
        builder.setStoragePath(options.dataDir);

    auto config = builder.build();
    return config;
}

static Sp<Node> initCarrierNode(Sp<Configuration> config)
{
	auto node = std::make_shared<Node>(config);
    try {
	    node->start();
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        return nullptr;
    }

    return node;
}

static void daemonize() {
#ifndef _WIN32
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);

    pid_t sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
#endif
}

static void stop()
{
    unloadAddons();

    if (g_node != nullptr) {
        g_node->stop();
        g_node = nullptr;
    }
}

static void signal_handler(int sig)
{
    broke = true;
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

int main(int argc, char *argv[])
{
    sys_coredump_set(true);

    auto options = parseArgs(argc, argv);
    if (options.daemonize)
        daemonize();

    setupSignals();

    auto config = initConfigure(options);

    std::string lockfile = config->getStoragePath() + "/lock";
    if(lock.acquire(lockfile) < 0) {
        std::cout << "Another instance already running." << std::endl;
        std::exit(-1);
    }

	g_node = initCarrierNode(config);
    if (!g_node)
        return 0;

	if (!loadAddons(g_node, config->getServices())) {
        stop();
        return 0;
    }

    while (!broke)
        std::this_thread::sleep_for(1000ms);

    stop();
    return 0;
}
