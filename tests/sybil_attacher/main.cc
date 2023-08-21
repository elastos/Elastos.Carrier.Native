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
#include <chrono>
#include <thread>

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

#include <CLI/CLI.hpp>
#include <carrier.h>

#include "../common/utils.h"

using namespace std;
using namespace elastos::carrier;
using namespace test;

struct Options {
    int mode {0};
    int duration {20};  //minutes
    int interval {5};  //seconds
    std::string id {};
    std::string ip {};
    int port = 0;
};

static Options options;
static int defaultPort = 38244;
static std::string dataDir;

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
    exit(-1);
}

static void parseArgs(int argc, char **argv)
{
    CLI::App app("Elastos Carrier sybil attacher", "attacher");
    app.add_option("-m, --mode", options.mode, "0: same address; 1: same node id.");
    app.add_option("-d, --duration", options.duration, "the duration (minute) of each node running");
    app.add_option("-i, --interval", options.interval, "the inerval time (second) of send 'find node' message.");
    app.add_option("-n, --remoteid", options.id, "remote node id.");
    app.add_option("-a, --remoteip", options.ip, "the ip of remote node.");
    app.add_option("-p, --remoteport", options.port, "the port of remote node.");

    try {
        app.parse(argc, argv);
    } catch (const CLI::Error &e) {
        int rc = app.exit(e);
        std::exit(rc);
    }
}

static void setupSignals()
{
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
#ifdef HAVE_SIGKILL
    signal(SIGKILL, signal_handler);
#endif
#ifdef HAVE_SIGHUP
    signal(SIGHUP, signal_handler);
#endif
}

static void run()
{
    int i = 0;
    std::string storagePath;

    auto remoteNode = NodeInfo{options.id, options.ip, options.port};

    auto builder = DefaultConfiguration::Builder {};
    auto ipAddresses = Utils::getLocalIpAddresses();
    builder.setIPv4Address(ipAddresses);

    if (options.mode == 0)
        storagePath = dataDir + Utils::PATH_SEP + "node1";
    else
        storagePath = dataDir + Utils::PATH_SEP + "node2";

    Utils::removeStorage(storagePath);

    auto expire = Utils::currentTimeMillis() + options.duration * 60 * 1000;

    do {
        if (options.mode == 0) {
            builder.setListeningPort(defaultPort);
            storagePath = storagePath + Utils::PATH_SEP + std::to_string(i);
        } else {
            builder.setListeningPort(defaultPort + i + 1);
        }

        builder.setStoragePath(storagePath);
        Utils::removeStorage(storagePath + Utils::PATH_SEP + "dht4.cache");
        Utils::removeStorage(storagePath + Utils::PATH_SEP + "dht6.cache");

        auto node = std::make_shared<Node>(builder.build());
        node->start();
        node->bootstrap(remoteNode);

        std::cout << "-------- create new node " << i++ << " /" << node->getId().toBase58String() << std::endl;

        for (int j = 0; j < Utils::getRandom(2, 5); j++) {
            auto future = node->findNode(remoteNode.getId());
            auto nis = future.get();
            if (nis.size() == 0) {
                std::cout << "       find node " << j << ": get no node" << std::endl;
            } else {
                std::cout << "       find node " << j << ": get node";
                for (auto ni: nis)
                    std::cout << " [" << ni->toString() << "] ";
                std::cout << "" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(options.interval * 1000));
        }

        node->stop();
    } while((int64_t)expire - (int64_t)Utils::currentTimeMillis() > 0);
}

int main(int argc, char* argv[])
{
#ifdef HAVE_SYS_RESOURCE_H
    sys_coredump_set(true);
#endif

    parseArgs(argc, argv);
    if (options.id.empty() || options.ip.empty() || options.port == 0) {
        std::cout << "Invalid remote id, remote ip or port. Please provide valid ones." << std::endl;
        return 0;
    }

    setupSignals();

    dataDir = Utils::getPwdStorage(Utils::PATH_SEP + "malicious_node");
    run();

    return 0;
}
