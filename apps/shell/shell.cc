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

#include <string>
#include <cstdlib>
#include <csignal>
#include <cctype>

#include <editline/readline.h>

#include <coredump.h>

#include "shell.h"

std::shared_ptr<Node> Command::node {};
static const std::string prompt = "Carrier $";

static char *trim(char *str)
{
    char *s, *t;

    for (s = str; isspace(*s); s++);
    if (*s == 0)
        return (s);

    t = s + strlen(s) - 1;
    while (t > s && isspace(*t))
        t--;
    *++t = '\0';

    return s;
}

void Shell::setupOptions()
{
    add_option("-4,--address4", addr4, "IPv4 address to listen.");
    add_option("-6,--address6", addr6, "IPv6 address to listen.");
    add_option("-p,--port", port, "The port to listen.");
    add_option("-d,--dataDir", dataDir, "The directory to store the node data, default: ~/.cache/carrier.");
    add_option("-b,--bootstrap", bootstrap, "The bootstrap node.");
    add_option("-c,--config", configFile, "The configuration file.");
    add_flag("--debug", waitForDebugAttach, "Waiting for the debuger attach.");

    addSubCommand(idCommand);
    addSubCommand(announcePeerCommand);
    addSubCommand(bootstrapCommand);
    addSubCommand(findNodeCommand);
    addSubCommand(findPeerCommand);
    addSubCommand(storeValueCommand);
    addSubCommand(findValueCommand);
    addSubCommand(routingtableCommand);
    addSubCommand(storageCommand);
    addSubCommand(helpCommand);
    addSubCommand(exitCommand);
}

void Shell::handleCommands()
{
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

    if (builder.getStoragePath().empty())
        builder.setStoragePath(dataDir);

    auto config = builder.build();
    node = std::make_shared<Node>(config);

    std::string lockfile = config->getStoragePath() + "/lock";

    if(lock.acquire(lockfile) < 0) {
        std::cout << "Another instance already running." << std::endl;
        std::exit(-1);
    }

    try {
        node->start();
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        std::exit(-1);
    }

    char* line;
    char* cmdline;
    while ((line = readline("Carrier $ ")) != nullptr) {
        cmdline = trim(line);

        if (*cmdline) {
            process(cmdline);
            add_history(cmdline);
        }

        free(line);
    }
}

int main(int argc, char **argv)
{
    sys_coredump_set(true);

    rl_initialize();
    stifle_history(256);
    rl_bind_key('\t', rl_complete);
    rl_readline_name = "carrier-shell";

    // We can add auto completion later
    // rl_attempted_completion_function = custom_completion;

    Shell shell;
    shell.process(argc, argv);
    shell.handleCommands();

    return 0;
}
