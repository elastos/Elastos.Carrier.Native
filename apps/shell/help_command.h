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

#include <iostream>
#include <string>

#include "command.h"

class HelpCommand : public Command {
public:
    HelpCommand() : Command("help", "Show all subcommands") {};

protected:
    void execute() override {
        std::cout << "Subcommands:" << std::endl
                  << "   id                          Display the ID of current Carrier node." << std::endl
                  << "   announcepeer                Announce a service peer." << std::endl
                  << "   bootstrap                   Bootstrap from the node." << std::endl
                  << "   findnode                    Find node and show the node info if exists" << std::endl
                  << "   findpeer                    Find peer and show the candidate peers if exists" << std::endl
                  << "   storevalue                  Store a value to the DHT." << std::endl
                  << "   findvalue                   Find value and show the value if exists" << std::endl
                  << "   routingtable                Display the routing tables." << std::endl
                  << "   storage                     Show the local data storage." << std::endl
                  << "   exit                        Close and quit the shell." << std::endl;
    }
};
