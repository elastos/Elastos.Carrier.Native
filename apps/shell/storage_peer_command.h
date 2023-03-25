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

class PeerCommand : public Command {
public:
    PeerCommand() : Command("peer", "Display peer info from the local storage.") {};

protected:
    void setupOptions() override {
        auto app = getApp();

        app->add_option("-f, --family", family, "IP family: 4 for IPv4, 6 for IPv6, default both");
        app->add_option("ID", id, "The peer id.");
        app->require_option(1, 2);
    };

    void execute() override {
        auto peerid = Id(id);

        auto storage = node->getStorage();
        auto peers = storage->getPeer(peerid, family, 0);
        std::cout << "----------------------------------------------" << std::endl;
        if (!peers.empty()) {
            for (auto& peer : peers)
                std::cout << static_cast<std::string>(*peer) << std::endl;
            std::cout << "Total " << peers.size() << " peers." << std::endl;
        } else {
            std::cout << "Peer " << id << " not exists." << std::endl;
        }
        std::cout << "----------------------------------------------" << std::endl;
    };

private:
    std::string id {};
    int family = 10;
};
