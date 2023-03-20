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

#include <memory>
#include <list>
#include <cstdio>
#include <atomic>

#include "carrier/id.h"
#include "carrier/node_info.h"
#include "carrier/value.h"
#include "carrier/lookup_option.h"
#include "carrier/peer_info.h"

#include "utils/misc.h"

#include "task/task_manager.h"
#include "rpcserver.h"
#include "routing_table.h"
#include "token_manager.h"
#include "data_storage.h"

namespace elastos {
namespace carrier {

class Message;
class PingRequest;
class RoutingTable;
class LookupResponse;
class Node;

class DHT {
public:
    enum Type {
        IPV4,
        IPV6
    };

    explicit DHT(Type _type, const Node& _node, const SocketAddress& _addr);

    bool canUseSocketAddress(const SocketAddress& addr) const {
		return addr.family() == (type == Type::IPV4 ? AF_INET : AF_INET6);
	}

    Type getType() const noexcept {
        return type;
    }

    const std::string getTypeName() const noexcept {
        switch (type) {
        case Type::IPV4:
            return "IPV4";
        case Type::IPV6:
            return "IPV6";
        default:
            return "Invalid Type";
        }
    }

    const Node& getNode() const noexcept {
        return node;
    }

    Sp<NodeInfo> getNode(const Id&) const;

    RPCServer& getServer() const noexcept {
        assert(rpcServer);
        return *rpcServer;
    };

    void setServer(const Sp<RPCServer> server) noexcept {
        this->rpcServer = server;
    }

    void setTokenManager(const Sp<TokenManager> manager) noexcept {
        this->tokenManager = manager;
    }

    const SocketAddress& getOrigin() const noexcept {
        return addr;
    }

    RoutingTable& getRoutingTable() noexcept {
        return routingTable;
    }

    TaskManager& getTaskManager() noexcept {
		return taskMan;
	}

    void enablePersistence(const std::string& path) noexcept {
        persistFile = path;
    }

    const std::vector<Sp<NodeInfo>>& getBootstraps() const noexcept {
        return bootstrapNodes;
    }

    std::vector<Id> getBootstrapIds() const {
        std::vector<Id> ids {};
        for (const auto& node: bootstrapNodes) {
            ids.push_back(node->getId());
        }
        return ids;
	}

    void bootstrap();
    void bootstrap(const NodeInfo&);

    void fillHomeBucket(const std::list<Sp<NodeInfo>>&);

    void start(std::vector<Sp<NodeInfo>>& bootstrapNodes);
    void stop();

    bool isRunning() const noexcept {
        return running;
    }

    Sp<Task> findNode(const Id& id, std::function<void(Sp<NodeInfo>)> completeHandler);
    Sp<Task> findValue(const Id& id, LookupOption option, std::function<void(Sp<Value>)> completeHandler);
    Sp<Task> storeValue(const Sp<Value> value, std::function<void(std::list<Sp<NodeInfo>>)> completeHandler);
    Sp<Task> findPeer(const Id& id, int expected, LookupOption option, std::function<void(std::list<Sp<PeerInfo>>)> completeHandler);
    Sp<Task> announcePeer(const Id& peerId, int port, std::function<void(std::list<Sp<NodeInfo>>)> completeHandler);

    void onTimeout(RPCCall* call);
    void onSend(const Id& id);

    void onMessage(Sp<Message>);
    std::string toString() const;

private:
    void received(Sp<Message>);
    void update();
    void sendError(Sp<Message> q, int code, const std::string& msg);

    void onRequest(Sp<Message>);
    void onResponse(Sp<Message>);
    void onError(Sp<Message>);

    void onPing(const Sp<Message>&);
    void onFindNode(const Sp<Message>&);
    void onFindValue(const Sp<Message>&);
    void onStoreValue(const Sp<Message>&);
    void onFindPeers(const Sp<Message>&);
    void onAnnouncePeer(const Sp<Message>&);

    void populateClosestNodes(Sp<LookupResponse> r, const Id& target, int v4, int v6);

private:
    Type type;

    const Node& node;
    Sp<RPCServer> rpcServer {};
    Sp<TokenManager> tokenManager {};

    SocketAddress addr;

    RoutingTable routingTable {*this};
    TaskManager taskMan {};

    std::vector<Sp<NodeInfo>> bootstrapNodes = {};
    std::map<SocketAddress, Id> knownNodes = {};
    std::atomic<bool> bootstrapping;
    uint64_t lastBootstrap;

    uint64_t lastSave;
    bool running = false;

    std::string persistFile;

    Sp<Logger> log;
};

} // namespace carrier
} // namespace elastos
