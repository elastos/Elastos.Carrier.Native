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
#include <vector>
#include <algorithm>
#include <cstring>

#include "command.h"

class FindPeerCommand : public Command {
public:
    FindPeerCommand() : Command("findpeer", "Find peer and show the candidate peers if exists.") {};

protected:
    void setupOptions() override {
        add_option("-m, --mode", mode, "lookup mode: 0(arbitrary), 1(optimistic), 2(conservative).");
        add_option("-x, --expected-count", excepted, "expected number of peers.");
        add_option("ID", id, "The peer id to be find.");
        require_option(1, 3);
    };

    void execute() override {
        if (mode > 2) {
            std::cout << "----------------------------------------------" << std::endl
                      << "Invalid mode: " << mode << std::endl
                      << "----------------------------------------------" << std::endl;
            return;
        }

        LookupOption option {mode};
        auto future = node->findPeer(Id(id), excepted, option);
        auto peers = future.get();
        std::cout << "----------------------------------------------" << std::endl;
        if (peers.empty()) {
            std::cout<< " Not found peer [" << id << "]" << std::endl;
        } else {
            for (const auto& peer : peers)
                std::cout << peer.toString() << std::endl;
        }
        std::cout << "----------------------------------------------" << std::endl;
    };

private:
    std::string id {};
    int excepted = 0;
    int mode = 2;

};
