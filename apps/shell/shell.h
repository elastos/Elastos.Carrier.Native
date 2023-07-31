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

#pragma once

#include <string>
#include <memory>

#include <carrier.h>
#include <application_lock.h>

#include "command.h"
#include "id_command.h"
#include "announce_peer_command.h"
#include "bootstrap_command.h"
#include "find_node_command.h"
#include "find_peer_command.h"
#include "store_value_command.h"
#include "find_value_command.h"
#include "routingtable_command.h"
#include "storage_command.h"
#include "help_command.h"
#include "exit_command.h"

using namespace elastos::carrier;

class Shell : public Command {
public:
    Shell() : Command("Elastos carrier interactive shell.") {};
    void handleCommands();

protected:
    void setupOptions() override;

private:
    std::string addr4 {};
    std::string addr6 {};
    int port = 0;

    std::string dataDir {};
    std::string bootstrap {};
    std::string configFile {};

    bool waitForDebugAttach = false;
    ApplicationLock lock;

    IdCommand idCommand;
    AnnouncePeerCommand announcePeerCommand {};
    BootstrapCommand bootstrapCommand {};
    FindNodeCommand findNodeCommand {};
    FindPeerCommand findPeerCommand {};
    StoreValueCommand storeValueCommand {};
    FindValueCommand findValueCommand {};
    RoutingTableCommand routingtableCommand {};
    StorageCommand storageCommand {};
    HelpCommand helpCommand {};
    ExitCommand exitCommand {};
};
