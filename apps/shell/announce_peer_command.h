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
#include "utils/hex.h"

class AnnouncePeerCommand : public Command {
public:
    AnnouncePeerCommand() : Command("announcepeer", "Announce a service peer.") {};

protected:
    void setupOptions() override {
        add_flag("-p, --persistent", persistent, "Persistent peer, default is false.");
        add_option("-k, --private-key", privateKey, "The private key.");
        add_option("-n, --node-id", nodeId, "The node id.");
        add_option("-a, --alternative-url", alt, "The alternative URL.");
        add_option("PORT", port, "The peer port to be announce.");
        require_option(1, 5);
    };

    void execute() override {
        Signature::KeyPair keypair {};
        if (!privateKey.empty())
            keypair = Signature::KeyPair::fromPrivateKey(Hex::decode(privateKey));

        Id peerNodeId = Command::node->getId();
        if (!nodeId.empty())
            peerNodeId = Id(nodeId);

        if (port <= 0) {
            std::cout << "Invalid port: " << std::to_string(port) << std::endl;
            return;
        }

        PeerInfo peer = PeerInfo::create(keypair, peerNodeId, Command::node->getId(), port, alt);

        auto future = node->announcePeer(peer, persistent);
        future.get();
        std::cout << "Peer " + peer.getId().toBase58String() << " announced with private key " <<
                Hex::encode(peer.getPrivateKey().blob()) << std::endl;
    };

private:
    bool persistent {false};
    std::string privateKey {};
    std::string nodeId {};
    std::string alt {};
    uint16_t port {0};
};
