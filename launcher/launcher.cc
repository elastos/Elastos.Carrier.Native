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
#include "activeproxy.h"

using namespace elastos::carrier;
using namespace elastos::carrier::activeproxy;

Sp<Node> __node__ = nullptr;
std::unique_ptr<ActiveProxy> __proxy__ = nullptr;
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
	node->start();
    return node;
}

static bool loadServices(Sp<Node> node, Sp<Configuration> config) {
    std::map<std::string, std::any>& services = config->getServices();
    if (services.empty())
        return true;

    for (auto& [name, value] : services) {
        if (name == "ActiveProxy") {
            if (value.type() != typeid(std::map<std::string, std::any>)) {
                std::cout << "Service '" << name << "': invalid configure! " << std::endl;
                return false;
            }

            auto configure = std::any_cast<std::map<std::string, std::any>>(value);
            if (!configure.count("serverId")) {
                std::cout << "Service '" << name << "': invalid serverId! " << std::endl;
                return false;
            }
            if (!configure.count("serverHost")) {
                std::cout << "Service '" << name << "': invalid serverHost! " << std::endl;
                return false;
            }
            if (!configure.count("serverPort")) {
                std::cout << "Service '" << name << "': invalid serverPort! " << std::endl;
                return false;
            }
            if (!configure.count("upstreamHost")) {
                std::cout << "Service '" << name << "': invalid upstreamHost! " << std::endl;
                return false;
            }
            if (!configure.count("upstreamPort")) {
                std::cout << "Service '" << name << "': invalid upstreamPort! " << std::endl;
                return false;
            }

            std::string serverId = std::any_cast<std::string>(configure["serverId"]);
            Id id(serverId);
            std::string serverHost = std::any_cast<std::string>(configure["serverHost"]);
            uint16_t serverPort = (uint16_t)std::any_cast<int64_t>(configure["serverPort"]);
            std::string upstreamHost = std::any_cast<std::string>(configure["upstreamHost"]);
            uint16_t upstreamPort = (uint16_t)std::any_cast<int64_t>(configure["upstreamPort"]);

            __proxy__ = std::make_unique<ActiveProxy>(*node, id, serverHost, serverPort, upstreamHost, upstreamPort);
            __proxy__->start();
        }
    }

    return true;
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
    if (__proxy__ != nullptr) {
        __proxy__->stop();
        __proxy__ = nullptr;
    }

    if (__node__ != nullptr) {
        __node__->stop();
        __node__ = nullptr;
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
	__node__ = initCarrierNode(config);
	if (!loadServices(__node__, config)) {
        stop();
        return 0;
    }

    while (!stopped) {
        sleep(1);
    }

    return 0;
}
