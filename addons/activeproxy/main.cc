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

int main(int argc, char *argv[])
{
    std::string dataPath = "./data";
    auto b = DefaultConfiguration::Builder {};

    b.setIPv4Address(upstreamHost);
    b.setListeningPort(39009);
    b.setStoragePath(dataPath);

    Node node = Node(b.build());
    node.start();

    std::cout << "Node Id: " << node.getId() << std::endl;

    ActiveProxy proxy{node, serverId, serverHost, serverPort, upstreamHost, upstreamPort};
    __proxy__ = &proxy;

    signal(SIGINT, [](int sig) {
        __proxy__->stop();
    });

    proxy.start();

    return 0;
}
