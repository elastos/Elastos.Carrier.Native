#include <iostream>
#include <csignal>

#include "activeproxy.h"

using namespace elastos::carrier;
using namespace elastos::carrier::activeproxy;

// TODO: need update
static Id serverId {"FrAnGZadEdf6av8xbCy89oUTEkUNKujA1yMADAkXPcZj"};
static const char *serverHost = "192.168.8.80";
static uint16_t serverPort = 10099;
static const char *upstreamHost = "192.168.8.80";
static uint16_t upstreamPort = 8080;

ActiveProxy* __proxy__;
bool stopped = false;

int main(int argc, char *argv[])
{
    std::string dataPath = "./data";
    auto b = DefaultConfiguration::Builder {};

    b.setIPv4Address(upstreamHost);
    b.setListeningPort(39009);
    b.setStoragePath(dataPath);

    Node node = Node(b.build());
    node.start();

    ActiveProxy proxy{node, serverId, serverHost, serverPort, upstreamHost, upstreamPort};
    __proxy__ = &proxy;

    signal(SIGINT, [](int sig) {
        stopped = true;
        __proxy__->stop();
    });

    proxy.start();

    while (!stopped) {
        sleep(1);
    }

    return 0;
}
