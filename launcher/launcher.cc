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
#include <csignal>

#ifndef _MSC_VER
#include <getopt.h>
#else
#define SIGHUP 0
#include "wingetopt.h"
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

#include "../src/constants.h"
#include "carrier.h"

using namespace elastos::carrier;

Sp<Node> g_node = nullptr;
bool stopped = false;

static void print_usage()
{
    std::cout << "Usage: launcher [OPTIONS] " << std::endl;
    std::cout << "   -c, --config <configFile>    The configuration file." << std::endl;
    std::cout << "   -4, --address4 <addr4>       IPv4 address to listen." << std::endl;
    std::cout << "   -6, --address6 <addr6>       IPv6 address to listen." << std::endl;
    std::cout << "   -p, --port <port>            The port to listen." << std::endl;
    std::cout << "   -d, --data-dir <dir>         The dir for storage data." << std::endl;
    std::cout << "   -D, --daemonize              Set daemonize." << std::endl;
    std::cout << "   -h, --help                   Show this help message and exit." << std::endl;
}

static void print_version() {
    std::cout << "Elastos Carrier version " << Constants::VERSION << std::endl;
}

static const constexpr struct option long_options[] = {
    {"help",        no_argument,        nullptr,    'h'},
    {"config",      required_argument,  nullptr,    'c'},
    {"address4",    required_argument,  nullptr,    '4'},
    {"address6",    required_argument,  nullptr,    '6'},
    {"port",        required_argument,  nullptr,    'p'},
    {"data-dir",    required_argument,  nullptr,    'd'},
    {"daemonize",   no_argument      ,  nullptr,    'D'},
    {"version",     no_argument      ,  nullptr,    'v'},
    {nullptr,       0,                  nullptr,    0}
};

struct dht_params {
    bool help {false}; // print help and exit
    bool version {false};
    bool daemonize {false};
    std::string ipv4 {};
    std::string ipv6 {};
    uint16_t port {0};
    std::string config_file {};
    std::string data_dir {};
};

static dht_params parseArgs(int argc, char **argv) {
    dht_params params;
    int opt;

    if (argc < 2) {
        params.help = true;
        return params;
    }

    while ((opt = getopt_long(argc, argv, "ic:4:6:p:d:Dv", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'v':
                params.version = true;
                break;
            case 'c':
                params.config_file = optarg;
                break;
            case '4':
                params.ipv4 = optarg;
                break;
            case '6':
                params.ipv6 = optarg;
                break;
            case 'p': {
                    int port_arg = atoi(optarg);
                    if (port_arg >= 0 && port_arg < 0x10000)
                        params.port = port_arg;
                    else
                        std::cout << "Invalid port: " << port_arg << std::endl;
                }
                break;
            case 'd':
                params.data_dir = optarg;
                break;
            case 'D':
                params.daemonize = true;
                break;
            case 'h':
            default:
                params.help = true;
                return params;
        }
    }

    return params;
}

static Sp<Configuration> initConfigure(dht_params& params) {
    auto builder = DefaultConfiguration::Builder{};

    if (!params.config_file.empty()) {
        builder.load(params.config_file);
    }

    if (!params.ipv4.empty()) {
        builder.setIPv4Address(params.ipv4);
    }

    if (!params.ipv6.empty()) {
        builder.setIPv6Address(params.ipv6);
    }

    if (params.port > 0) {
        builder.setListeningPort(params.port);
    }

    if (!params.data_dir.empty()) {
        builder.setStoragePath(params.data_dir);
    }

    auto config = builder.build();
    return config;
}

static Sp<Node> initCarrierNode(Sp<Configuration> config) {
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
#ifndef _MSC_VER
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

static void stop() {
    stopped = true;
    unloadAddons();

    if (g_node != nullptr) {
        g_node->stop();
        g_node = nullptr;
    }
}

static void signal_handler(int sig)
{
    switch(sig) {
    case SIGHUP:
        break;
    case SIGINT:
       stop();
    case SIGTERM:
        break;
    }
}

static void setupSignals()
{
#ifndef _MSC_VER
    signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP,signal_handler); /* catch hangup signal */
    signal(SIGINT,signal_handler); /* catch interrupt signal */
    signal(SIGTERM,signal_handler); /* catch kill signal */
#endif
}

int main(int argc, char *argv[])
{
    auto params = parseArgs(argc, argv);
    if (params.help) {
        print_usage();
        return 0;
    }
    if (params.version) {
        print_version();
        return 0;
    }

    if (params.daemonize) {
        daemonize();
    }

    setupSignals();

    auto config = initConfigure(params);
	g_node = initCarrierNode(config);
    if (!g_node)
        return 0;

	if (!loadAddons(g_node, config->getServices())) {
        stop();
        return 0;
    }

    while (!stopped) {
        sleep(1);
    }

    return 0;
}
