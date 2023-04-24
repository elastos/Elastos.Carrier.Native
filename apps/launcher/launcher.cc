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

#ifndef _WIN32
#include <unistd.h>
#endif

#include <CLI/CLI.hpp>

#include <carrier.h>
#include <application_lock.h>
#include <coredump.h>

using namespace elastos::carrier;

static Sp<Node> g_node = nullptr;
static bool stopped = false;
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
    if (stopped)
        return;

    stopped = true;
    unloadAddons();

    if (g_node != nullptr) {
        g_node->stop();
        g_node = nullptr;
    }
}

static void signalHandler(int sig)
{
    stop();
}

static void setupSignals()
{

    signal(SIGCHLD, SIG_IGN);   /* ignore child */
    signal(SIGTSTP, SIG_IGN);   /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
#ifndef _MSC_VER
    signal(SIGHUP, SIG_IGN);    /* catch hangup signal */
#endif
    signal(SIGINT, signalHandler); /* catch interrupt signal */
    signal(SIGTERM, signalHandler); /* catch kill signal */
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

    while (!stopped)
        sleep(1);

    return 0;
}
