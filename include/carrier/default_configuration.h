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

#include <string>
#include <vector>

#include "def.h"
#include "configuration.h"

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC DefaultConfiguration final : public Configuration {
public:
    DefaultConfiguration() = delete;
    DefaultConfiguration(const std::string& ip4, const std::string& ip6, int port,
        std::string path, std::vector<Sp<NodeInfo>> nodes)
        : storagePath(path), bootstrapNodes(nodes) {
            try {
                addr4 = SocketAddress(ip4, port);
                addr6 = SocketAddress(ip6, port);
            } catch(const std::exception& e) {
                // TODO:
            }
    }

    SocketAddress& ipv4Address() override {
        return addr4;
    }
    SocketAddress& ipv6Address() override {
        return addr6;
    }

    int listeningPort() override {
        return addr4.port() ? addr4.port() : addr6.port();
    }

    const std::string& getStoragePath() override {
        return storagePath;
    }
    std::vector<Sp<NodeInfo>>& getBootstrapNodes() override {
        return bootstrapNodes;
    }

    class Builder {
    public:
        Builder() {
        }

        void setAutoIPv4Address(bool automatic) {
            this->autoAddr4 = automatic;
        }
        void setAutoIPv6Address(bool automatic) {
            this->autoAddr6 = automatic;
        }

        void setAutoIPAddress(bool automatic) {
            this->autoAddr4 = automatic;
            this->autoAddr6 = automatic;
        }

        void setIPv4Address(const std::string& ip) {
            this->ip4 = ip;
        }

        void setIPv6Address(const std::string& ip) {
            this->ip6 = ip;
        }

        void setListeningPort(int port) {
            if (port <= 0 || port > 65535)
                throw std::invalid_argument("Invalid port: " + std::to_string(port));

            this->port = port;
        }

        void setStoragePath(const std::string& path) {
            this->storagePath = path;
        }

        void addBootstrap(const std::string& idstr, const std::string& addr, int port) {
            auto id = Id(idstr);
            auto address = SocketAddress(addr, port);
            auto node = std::make_shared<NodeInfo>(id, address);
            bootstrapNodes.emplace_back(node);
        }
        void addBootstrap(const Id& id, SocketAddress addr) {
            auto node = std::make_shared<NodeInfo>(id, addr);
            bootstrapNodes.emplace_back(node);
        }
        void addBootstrap(Sp<NodeInfo> node) {
            bootstrapNodes.emplace_back(node);
        }

        void load(const std::string& path);
        void reset();

        Sp<Configuration> build();

    private:
        bool autoAddr4 { true };
        bool autoAddr6 { false };
        std::string ip4 {};
        std::string ip6 {};
        int port = 39001;
        std::string storagePath {};
        std::vector<Sp<NodeInfo>> bootstrapNodes {};
    };

private:
    SocketAddress addr4 {};
    SocketAddress addr6 {};

    std::string storagePath {};
    std::vector<Sp<NodeInfo>> bootstrapNodes {};
};

} // namespace carrier
} // namespace elastos
