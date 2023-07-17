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
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>

#include <utf8proc.h>

#include "command.h"

class AnnouncePeerCommand : public Command {
public:
    AnnouncePeerCommand() : Command("announcepeer", "Announce a service peer.") {};

protected:
    void setupOptions() override {
        add_option("PORT", port, "The peer port to be announce.");
        add_option("ALT", alt, "The peer alt to be announce.");
        require_option(3, 3);
    };

    void execute() override {
        if (port <= 0) {
            std::cout << "----------------------------------------------" << std::endl
                      << "Invalid port: " << port << std::endl
                      << "----------------------------------------------" << std::endl;
            return;
        }


        const char* alternative = (const char *)utf8proc_NFC((unsigned char *)(alt.c_str()));

        PeerInfo peer = PeerInfo::create(Command::node->getId(), port, alternative);

        auto future = node->announcePeer(peer);
        auto result = future.get();
        std::cout << "----------------------------------------------" << std::endl;
        if (result)
            std::cout << "Peer [" << peer.getId().toBase58String() << "] announced." << std::endl;
        else
            std::cout << "Peer [" << peer.getId().toBase58String() << "] announce failed." << std::endl;
        std::cout << "----------------------------------------------" << port << std::endl;
    };

private:
    std::string name {};
    uint16_t port {0};
    std::string alt {};
};
