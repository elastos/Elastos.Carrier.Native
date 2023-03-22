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
#include <csignal>

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#include "shell.h"
#include "id_command.h"
#include "announce_peer_command.h"
#include "bootstrap_command.h"
#include "find_node_command.h"
#include "find_peer_command.h"
#include "store_value_command.h"
#include "find_value_command.h"
#include "routingtable_command.h"
#include "storage_command.h"
#include "storage_value_command.h"
#include "storage_peer_command.h"
#include "storage_listpeer_command.h"
#include "storage_listvalue_command.h"
#include "exit_command.h"

static const std::string prompt = "Carrier $";

int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}

void signal_handler(int signum)
{
    exit(-1);
}

int main(int argc, char **argv)
{
    Shell shell;
    IdCommand idCommand;
    AnnouncePeerCommand announcePeerCommand;
    BootstrapCommand bootstrapCommand;
    FindNodeCommand findNodeCommand;
    FindPeerCommand findPeerCommand;
    StoreValueCommand storeValueCommand;
    FindValueCommand findValueCommand;
    RoutingTableCommand routingtableCommand;
    ValueCommand valueCommand;
    PeerCommand peerCommand;
    ListValueCommand listValueCommand;
    ListPeerCommand listPeerCommand;
    StorageCommand storageCommand;
    ExitCommand exitCommand;

    std::string cmdline;

    shell.addSubCommand(idCommand);
    shell.addSubCommand(announcePeerCommand);
    shell.addSubCommand(bootstrapCommand);
    shell.addSubCommand(findNodeCommand);
    shell.addSubCommand(findPeerCommand);
    shell.addSubCommand(storeValueCommand);
    shell.addSubCommand(findValueCommand);
    shell.addSubCommand(routingtableCommand);
    storageCommand.addSubCommand(valueCommand);
    storageCommand.addSubCommand(peerCommand);
    storageCommand.addSubCommand(listValueCommand);
    storageCommand.addSubCommand(listPeerCommand);
    shell.addSubCommand(storageCommand);
    shell.addSubCommand(exitCommand);
    shell.prepare();

    shell.run(argc, argv);

    while (true) {
        std::cout << "Carrier $ ";
        std::getline(std::cin, cmdline);

        shell.run(cmdline);
     }
}
