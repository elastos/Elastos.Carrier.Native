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
#include <utf8proc.h>

#include "command.h"

class AnnouncePeerCommand : public Command {
public:
    AnnouncePeerCommand() : Command("announcepeer", "Announce a service peer.") {};

protected:
    void setupOptions() override {
        auto app = getApp();

        app->add_option("NAME", name, "The service name to be announce.");
        app->add_option("PORT", port, "The service port to be announce.");
        app->require_option(2, 2);
    };

    void execute() override {
        if (port <= 0) {
            std::cout << "----------------------------------------------" << std::endl
                      << "Invalid port: " << port << std::endl
                      << "----------------------------------------------" << std::endl;
            return;
        }

        const char* nname = (const char *)utf8proc_NFC((unsigned char *)(name.c_str()));
        std::vector<uint8_t> data;
        data.reserve(strlen(nname));
        std::memcpy((void*)data.data(), nname, strlen(nname));
        auto d = SHA256::digest(data);
        auto id = Id(d);

        auto future = node->announcePeer(id, port);
        auto result = future.get();
        std::cout << "----------------------------------------------" << std::endl;
        if (result)
            std::cout << "Peer [" << id << "] announced." << std::endl;
        else
            std::cout << "Peer [" << id << "] announce failed." << std::endl;
        std::cout << "----------------------------------------------" << port << std::endl;
    };

private:
    std::string name {};
    int port = 0;
};
