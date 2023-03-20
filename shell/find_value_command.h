/*
 * Copyright (c) 2022 - 2023 Elastos Foundation
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

class FindValueCommand : public Command {
public:
    FindValueCommand() : Command("findvalue", "Find value and show the value if exists.") {};

protected:
    void setupOptions() override {
        auto app = getApp();

        app->add_option("-m, --mode", mode, "lookup mode: 0(arbitrary), 1(optimistic), 2(conservative).");
        app->add_option("ID", id, "The target node id to be find.");
        app->require_option(1, 1);
    };

    void execute() override {
        if (mode > 2) {
            std::cout << "Invalid mode: " << mode << std::endl;
            return;
        }

        auto nodeid = Id(id);
        LookupOption option {mode};
        auto future = node->findValue(nodeid, option);
        auto value = future.get();
        if (value)
            std::cout << static_cast<std::string>(*value) << std::endl;
        else
            std::cout << "Not found." << std::endl;
    };

private:
    std::string id {};
    int mode = 2;

};
