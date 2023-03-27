#include <string>
#include <memory>

#include <carrier.h>

#include "command.h"
#include "application_lock.h"

using namespace elastos::carrier;

std::shared_ptr<Node> node {};

class Shell : public Command {
public:
    Shell() : Command("Elastos carrier interactive shell.") {};

protected:
    void setupOptions() override {
        auto app = getApp();

        app->add_option("-4,--address4", addr4, "IPv4 address to listen.");
        app->add_option("-6,--address6", addr6, "IPv6 address to listen.");
        app->add_option("-p,--port", port, "The port to listen.");
        app->add_option("-d,--dataDir", dataDir, "The directory to store the node data, default: ~/.cache/carrier.");
        app->add_option("-b,--bootstrap", bootstrap, "The bootstrap node.");
        app->add_option("-c,--config", configFile, "The configuration file.");
        app->add_flag("--debug", waitForDebugAttach, "Waiting for the debuger attach.");
    };

    void execute() override {
        if (node)
            return;

        if (waitForDebugAttach) {
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


        auto builder = DefaultConfiguration::Builder {};
        if (!configFile.empty()) {
            try {
                builder.load(configFile);
            } catch (const std::exception& e) {
                std::cout << "Can not load the config file: " << configFile << ", error: " << e.what();
            }
        }

        if (!addr4.empty())
            builder.setIPv4Address(addr4);

        if (!addr6.empty())
            builder.setIPv6Address(addr6);

        if (port != 0)
            builder.setListeningPort(port);

        if (!dataDir.empty())
            builder.setStoragePath(dataDir);

        auto config = builder.build();
        node = std::make_shared<Node>(config);

        std::string lockfile = dataDir + "/lock";

        if(lock.acquire(lockfile) < 0) {
            std::cout << "Another instance already running." << std::endl;
            exit(-1);
        }

        node->start();
    };

private:
    std::string addr4 {};
    std::string addr6 {};
    int port = 0;

    std::string dataDir = "~/.cache/carrier";
    std::string bootstrap {};
    std::string configFile {};

    bool waitForDebugAttach = false;
    ApplicationLock lock;
};
