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
#include <cctype>

#include <editline/readline.h>

#include <coredump.h>

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

char *trim(char *str)
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

int main(int argc, char **argv)
{
    sys_coredump_set(true);

    rl_initialize();
    stifle_history(256);
    rl_bind_key('\t', rl_complete);
    rl_readline_name = "carriershell";

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

    // We can add auto completion later
    // rl_attempted_completion_function = custom_completion;

    char* line;
    char* cmdline;
    while ((line = readline("Carrier $ ")) != nullptr) {
        cmdline = trim(line);

        if (*cmdline) {
            shell.run(cmdline);
            add_history(cmdline);
        }

        free(line);
    }

    return 0;
}
