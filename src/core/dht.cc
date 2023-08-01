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

#include <atomic>
#include <memory>

#include "carrier/node.h"
#include "carrier/peer_info.h"
#include "carrier/value.h"

#include "utils/time.h"
#include "utils/log.h"
#include "task/node_lookup.h"
#include "task/task_manager.h"
#include "task/value_lookup.h"
#include "task/value_announce.h"
#include "task/peer_lookup.h"
#include "task/peer_announce.h"

#include "messages/message.h"
#include "messages/ping_request.h"
#include "messages/ping_response.h"
#include "messages/find_node_request.h"
#include "messages/find_node_response.h"
#include "messages/find_value_request.h"
#include "messages/find_value_response.h"
#include "messages/find_peer_request.h"
#include "messages/find_peer_response.h"
#include "messages/store_value_request.h"
#include "messages/store_value_response.h"
#include "messages/announce_peer_request.h"
#include "messages/announce_peer_response.h"
#include "messages/error_message.h"

#include "error_code.h"
#include "rpccall.h"
#include "routing_table.h"
#include "data_storage.h"
#include "kclosest_nodes.h"
#include "dht.h"

namespace elastos {
namespace carrier {

DHT::DHT(Type _type, const Node& _node, const SocketAddress& _addr)
    :type(_type), node(_node), addr(_addr), bootstrapping(false) {

    log = Logger::get("dht");
}

Sp<NodeInfo> DHT::getNode(const Id& nodeId) const {
    return routingTable.getEntry(nodeId);
}

void DHT::bootstrap() {
  if (!isRunning() || bootstrapNodes.empty()
       || currentTimeMillis() - lastBootstrap < Constants::BOOTSTRAP_MIN_INTERVAL)
       return;

    bool expected {false};
    if (!bootstrapping.compare_exchange_weak(expected, true))
        return;

    log->info("DHT {} bootstraping...", getTypeName());

    auto nodes = std::make_shared<std::list<Sp<NodeInfo>>>();
    auto count = std::make_shared<int>(0);
    int len = bootstrapNodes.size();

    for (auto node: bootstrapNodes) {
        std::promise<std::list<Sp<NodeInfo>>> promise {};
        auto q = std::make_shared<FindNodeRequest>(Id::random());

        q->setWant4(type == Type::IPV4);
        q->setWant6(type == Type::IPV6);

        auto call = std::make_shared<RPCCall>(this, node, q);
        call->addStateChangeHandler([=](RPCCall* call, RPCCall::State previous, RPCCall::State current) {
            log->debug("RPCCall::OnStateChange for FindNodeRequest message invoked .....");
            if (current == RPCCall::State::RESPONDED || current == RPCCall::State::ERR
                    || current == RPCCall::State::TIMEOUT) {
                auto r = std::dynamic_pointer_cast<FindNodeResponse>(call->getResponse());
                if (r != nullptr) {
                    auto list = r->getNodes(getType());
                    nodes->insert(nodes->end(), list.begin(), list.end());
                }
                (*count)++;
                if (*count == len) {
                    this->lastBootstrap = currentTimeMillis();
                    this->fillHomeBucket(*nodes);
                }
            }
        });
        rpcServer->sendCall(call);
    }
}

void DHT::bootstrap(const NodeInfo& ni) {
    if (!canUseSocketAddress(ni.getAddress()) || ni.getId() == node.getId())
        return;

    auto nodes = getBootstraps();
    auto pos = std::find_if(nodes.begin(), nodes.end(), [&](Sp<NodeInfo> item) {
        return (*item == ni);
    });

    if (pos == nodes.end()) {
        bootstrapNodes.push_back(std::make_shared<NodeInfo>(ni));
        lastBootstrap = 0;
        bootstrap();
    }
}

void DHT::fillHomeBucket(const std::list<Sp<NodeInfo>>& nodes) {
    if (routingTable.getNumBucketEntries() == 0 && nodes.empty()) {
        bootstrapping = false;
        return;
    }

    auto task = std::make_shared<NodeLookup>(this, node.getId());
    task->setBootstrap(true);
    task->setName("Bootstrap: filling home bucket");
    task->injectCandidates(nodes);
    task->addListener([&](Task* t) {
        bootstrapping = false;

        if (!isRunning())
            return;

        if (routingTable.getNumBucketEntries() > Constants::MAX_ENTRIES_PER_BUCKET + 2)
            routingTable.fillBuckets();
    });

    taskMan.add(task);
}

void DHT::update () {
    if (!isRunning())
        return;

    log->trace("DHT {} regularly update...", getTypeName());

    uint64_t now = currentTimeMillis();

    rpcServer->updateReachability(now);
    routingTable.maintenance();

    if (routingTable.getNumBucketEntries() < Constants::BOOTSTRAP_IF_LESS_THAN_X_PEERS ||
            now - lastBootstrap > Constants::SELF_LOOKUP_INTERVAL)
        // Regularly search for our id to update routing table
        bootstrap();

    if (persistFile != "" && (now - lastSave) > Constants::ROUTING_TABLE_PERSIST_INTERVAL) {
        log->info("Persisting routing table ...");
        routingTable.save(persistFile);
        lastSave = now;
    }
}

void DHT::start(std::vector<Sp<NodeInfo>>& nodes) {
    if (running)
        return;

    if (persistFile != "" /* && persistFile.exists() && persistFile.isFile()*/) {
        log->info("Loading routing table from {} ...", persistFile);
        routingTable.load(persistFile);
    }

    for (auto node: nodes) {
        if (canUseSocketAddress(node->getAddress()))
            bootstrapNodes.emplace_back(node);
    }

    log->info("Starting DHT/{} on {}", getTypeName(), addr.toString());

    running = true;

    auto& scheduler = rpcServer->getScheduler();

    scheduler.add([&]() {
        // tasks maintenance that should run all the time, before the first queries
        taskMan.dequeue();
    }, 5000, Constants::DHT_UPDATE_INTERVAL);

    // Ping check if the routing table loaded from cache
    for (auto& bucket: routingTable.getBuckets()) {
        if (bucket->size() == 0)
            continue;

        std::vector<PingRefreshTask::Options> options{PingRefreshTask::Options::removeOnTimeout};
        auto task = std::make_shared<PingRefreshTask>(this, bucket, options);
        task->setName("Bootstrap cached table ping for " + bucket->getPrefix().toString());
        taskMan.add(task);
    }

    bootstrap();

    // fix the first time to persist the routing table: 2 min
    lastSave = currentTimeMillis() - Constants::ROUTING_TABLE_PERSIST_INTERVAL + (120 * 1000);

    // Regularly DHT update
    scheduler.add([&]() {
        update();
    }, 5000, Constants::DHT_UPDATE_INTERVAL);


    // send a ping request to a random node to check socket liveness
    scheduler.add([&]() {
        if (rpcServer->getNumberOfActiveRPCCalls() > 0)
            return;

        auto entry = routingTable.getRandomEntry();
        if (!entry)
            return;

        auto q = std::make_shared<PingRequest>();
        auto c = std::make_shared<RPCCall>(this, entry, q);
        rpcServer->sendCall(c);
    }, Constants::RANDOM_PING_INTERVAL, Constants::RANDOM_PING_INTERVAL);

    // deep lookup to make ourselves known to random parts of the keyspace
    scheduler.add([&]() {
        auto task = std::make_shared<NodeLookup>(this, Id::random());
        task->addListener([](Task* t) {});
        task->setName(getTypeName() + ":Random Refresh Lookup");
        taskMan.add(task);
    }, Constants::RANDOM_LOOKUP_INTERVAL, Constants::RANDOM_LOOKUP_INTERVAL);
}

void DHT::stop() {
    if (!running)
        return;

    log->info("{} initated DHT shutdown...", getTypeName());
    log->info("stopping servers");
    running = false;

    if (persistFile != "") {
        log->info("Persisting routing table on shutdown...");
        routingTable.save(persistFile);
    }

    taskMan.cancelAll();
}

void DHT::onMessage(Sp<Message> msg) {
    if (!isRunning())
        return;

    // ignore the messages we get from ourself
    if (node.isSelfId(msg->getId()) || isSelfAddress(msg->getOrigin()))
        return;

    switch (msg->getType()) {
    case Message::Type::REQUEST:
        onRequest(msg);
        break;

    case Message::Type::RESPONSE:
        onResponse(msg);
        break;

    case Message::Type::ERR:
        onError(msg);
        break;
    }

    received(msg);
}

void DHT::received(Sp<Message> msg) {
    auto addr = msg->getOrigin();
    bool bogon = false;

#ifdef CARRIER_DEVELOPMENT
    bogon = !addr.isAnyUnicast();
#else
    bogon = addr.isBogon();
#endif
    if (bogon) {
        log->info("Received a message from bogon address {}, ignored the potential routing table operation", addr.toString());
        return;
    }

    Id id = msg->getId();
    auto call = msg->getAssociatedCall();

    // we only want remote nodes with stable ports in our routing table,
    // so apply a stricter check here
    if (call != nullptr && (!call->matchesAddress() || !call->matchesId()))
        return;

    auto old = routingTable.getEntry(id);
    if (old != nullptr && old->getAddress() != addr) {
        // this might happen if one node changes ports (broken NAT?) or IP address
        // ignore until routing table entry times out
        return;
    }

    auto iter = knownNodes.find(addr);
    if (iter != knownNodes.end() && iter->second != id) {
        Id knownId = iter->second;
        auto knownEntry = routingTable.getEntry(knownId);
        if (knownEntry != nullptr) {
            // 1. a node with that address is in our routing table
            // 2. the ID does not match our routing table entry
            //
            // That means we are certain that the node either changed its
            // node ID or does some ID-spoofing.
            // In either case we don't want it in our routing table
            log->warn("force-removing routing table entry {} because ID-change was detected; new ID {}", knownEntry->toString(),
                    static_cast<std::string>(id));
            routingTable.remove(knownId);

            // might be pollution attack, check other entries in the same bucket too in case
            // random
            // pings can't keep up with scrubbing.
            auto bucket = routingTable.getBucket(knownId);
            auto name = "Checking bucket " + bucket->getPrefix().toString() + " after ID change was detected";
            routingTable.tryPingMaintenance(bucket, {PingRefreshTask::Options::checkAll}, name);
            knownNodes[addr] = id;
            return;
        }
        else {
            knownNodes.erase(iter);
        }
    }

    knownNodes[addr] = id;
    auto newEntry = std::make_shared<KBucketEntry>(id, addr, msg->getVersion());

    if (call != nullptr) {
        newEntry->signalResponse();
        newEntry->mergeRequestTime(call->getSentTime());
    }
    else if (old == nullptr) {
        // Verify the node, speedup the bootstrap process
        auto q = std::make_shared<PingRequest>();

        auto c = std::make_shared<RPCCall>(this, newEntry, q);
        // Maybe we are in the RPCSever's callback
        rpcServer->sendCall(c);
    }

    routingTable.put(newEntry);
}

void DHT::sendError(Sp<Message> q, int code, const std::string& msg) {
    rpcServer->sendError(q, code, msg);
}

void DHT::onRequest(Sp<Message> message) {
    switch (message->getMethod()) {
    case Message::Method::PING:
        onPing(message);
        break;

    case Message::Method::FIND_NODE:
        onFindNode(message);
        break;

    case Message::Method::FIND_VALUE:
        onFindValue(message);
        break;

    case Message::Method::STORE_VALUE:
        onStoreValue(message);
        break;

    case Message::Method::FIND_PEER:
        onFindPeers(message);
        break;

    case Message::Method::ANNOUNCE_PEER:
        onAnnouncePeer(message);
        break;

    case Message::Method::UNKNOWN:
        sendError(message, ErrorCode::ProtocolError, "Invalid request method");
        break;
    }
}

void DHT::onResponse(Sp<Message> msg) {
    //Nothing to do
}

void DHT::onError(Sp<Message> msg) {
    auto e = std::static_pointer_cast<ErrorMessage>(msg);
    log->warn("Error from {}/{} - {}:{}, txid {}", e->getOrigin().toString(),
        e->getReadableVersion(), e->getCode(), e->getMessage(), e->getTxid());
}

void DHT::onPing(const Sp<Message>& msg) {
    auto response = std::make_shared<PingResponse>(msg->getTxid());
    response->setRemote(msg->getId(), msg->getOrigin());
    rpcServer->sendMessage(response);
}

void DHT::onFindNode(const Sp<Message>& msg) {
    auto request = std::dynamic_pointer_cast<FindNodeRequest>(msg);
    auto response = std::make_shared<FindNodeResponse>(msg->getTxid());

    int want4 = request->doesWant4() ? Constants::MAX_ENTRIES_PER_BUCKET : 0;
    int want6 = request->doesWant6() ? Constants::MAX_ENTRIES_PER_BUCKET : 0;
    populateClosestNodes(response, request->getTarget(), want4, want6);

    if (request->doesWantToken()) {
        auto token = tokenManager->generateToken(request->getId(), request->getOrigin(), request->getTarget());
        response->setToken(token);
    }

    response->setRemote(request->getId(), request->getOrigin());
    rpcServer->sendMessage(response);
}

void DHT::onFindValue(const Sp<Message>& msg) {
    auto request = std::dynamic_pointer_cast<FindValueRequest>(msg);

    auto response = std::make_shared<FindValueResponse>(msg->getTxid());

    auto token = tokenManager->generateToken(request->getId(), request->getOrigin(), node.getId());
    response->setToken(token);

    auto hasValue {false};
    auto value = node.getStorage()->getValue(request->getTarget());
    if (value != nullptr) {
        if (request->getSequenceNumber() < 0 || value->getSequenceNumber() < 0
                || request->getSequenceNumber() <= value->getSequenceNumber()) {
            hasValue = true;
            response->setValue(*value);
        }
    }

    if (!hasValue) {
        int want4 = request->doesWant4() ? Constants::MAX_ENTRIES_PER_BUCKET : 0;
        int want6 = request->doesWant6() ? Constants::MAX_ENTRIES_PER_BUCKET : 0;
        populateClosestNodes(response, request->getTarget(), want4, want6);
    }

    response->setRemote(request->getId(), request->getOrigin());
    rpcServer->sendMessage(response);
}

void DHT::onStoreValue(const Sp<Message>& msg) {
    auto request = std::dynamic_pointer_cast<StoreValueRequest>(msg);
    auto value = request->getValue();
    auto valueId = value.getId();
    if (!tokenManager->verifyToken(request->getToken(), request->getId(), request->getOrigin(), valueId)) {
        log->warn("Received a store value request with invalid token from {}", request->getOrigin().toString());
        sendError(request, ErrorCode::ProtocolError, "Invalid token for STORE VALUE request");
        return;
    }

    if (!value.isValid()) {
        sendError(request, ErrorCode::ProtocolError, "Invalid value");
        return;
    }

    node.getStorage()->putValue(value, request->getExpectedSequenceNumber());

    auto response = std::make_shared<StoreValueResponse>(request->getTxid());
    response->setRemote(request->getId(), request->getOrigin());
    rpcServer->sendMessage(response);
}

void DHT::onFindPeers(const Sp<Message>& msg) {
    auto request = std::static_pointer_cast<FindPeerRequest>(msg);
    auto response = std::make_shared<FindPeerResponse>(msg->getTxid());

    auto storage = node.getStorage();
    auto target = request->getTarget();
    auto token = tokenManager->generateToken(request->getId(), request->getOrigin(), target);
    response->setToken(token);

    bool hasPeers {false};
    auto peers = storage->getPeer(target, 8);
    if (!peers.empty()) {
        response->setPeers(peers);
        hasPeers = true;
    }

    if (!hasPeers) {
        int want4 = request->doesWant4() ? Constants::MAX_ENTRIES_PER_BUCKET : 0;
        int want6 = request->doesWant6() ? Constants::MAX_ENTRIES_PER_BUCKET : 0;
        populateClosestNodes(response, request->getTarget(), want4, want6);
    }

    response->setRemote(request->getId(), request->getOrigin());
    rpcServer->sendMessage(response);
}

void DHT::onAnnouncePeer(const Sp<Message>& msg) {
    auto request = std::static_pointer_cast<AnnouncePeerRequest>(msg);
    bool bogon {false};

#ifdef CARRIER_DEVELOPMENT
    bogon = !request->getOrigin().isAnyUnicast();
#else
    bogon = request->getOrigin().isBogon();
#endif
    if (bogon) {
        log->info("Received an announce peer request from bogon address {}, ignored ", request->getOrigin().toString());
        return;
    }

    if (!tokenManager->verifyToken(request->getToken(), request->getId(), request->getOrigin(), request->getTarget())) {
        log->warn("Received an announce peer request with invalid token from {}", request->getOrigin().toString());
        sendError(request, ErrorCode::ProtocolError, "Invalid token for ANNOUNCE PEER request");
        return;
    }


    auto peer = request->getPeer();
    log->debug("Received an announce peer request from {}, saving peer {}", request->getOrigin().toString(),
                    request->getTarget());
    node.getStorage()->putPeer(peer);

    auto response = std::make_shared<AnnouncePeerResponse>(request->getTxid());
    response->setRemote(request->getId(), request->getOrigin());
    rpcServer->sendMessage(response);
}

void DHT::onTimeout(RPCCall* call) {
    // ignore the timeout if the DHT is stopped or the RPC server is offline
    if (!isRunning() || !rpcServer->isReachable())
        return;

    routingTable.onTimeout(call->getTargetId());
}

void DHT::onSend(const Id& id) {
    if (!isRunning())
         return;

    routingTable.onSend(id);
}

#ifdef CARRIER_CRAWLER
void DHT::ping(Sp<NodeInfo> node, std::function<void(Sp<NodeInfo>)> completeHandler) {
    auto q = std::make_shared<PingRequest>();

    auto call = std::make_shared<RPCCall>(this, node, q);
    call->addStateChangeHandler([=](RPCCall* call, RPCCall::State previous, RPCCall::State current) {
        log->debug("RPCCall::OnStateChange for FindNodeRequest message invoked .....");
        if (current == RPCCall::State::RESPONDED) {
            auto r = std::dynamic_pointer_cast<PingResponse>(call->getResponse());
            if (r != nullptr) {
                node->setVersion(r->getVersion());
            }
            completeHandler(node);
        }
        else if (current == RPCCall::State::ERR || current == RPCCall::State::TIMEOUT) {
            completeHandler(NULL);
        }
    });
    rpcServer->sendCall(call);
}

void DHT::getNodes(const Id& id, Sp<NodeInfo> node, std::function<void(std::list<Sp<NodeInfo>>)> completeHandler) {
    auto q = std::make_shared<FindNodeRequest>(id);

    q->setWant4(type == Type::IPV4);
    q->setWant6(type == Type::IPV6);

    auto call = std::make_shared<RPCCall>(this, node, q);
    call->addStateChangeHandler([=](RPCCall* call, RPCCall::State previous, RPCCall::State current) {
        log->debug("RPCCall::OnStateChange for FindNodeRequest message invoked .....");
        if (current == RPCCall::State::RESPONDED || current == RPCCall::State::ERR
                || current == RPCCall::State::TIMEOUT) {
            auto r = std::dynamic_pointer_cast<FindNodeResponse>(call->getResponse());
            if (r != nullptr) {
                auto list = r->getNodes(getType());
                completeHandler(list);
            }
        }
    });
    rpcServer->sendCall(call);
}
#endif

Sp<Task> DHT::findNode(const Id& id, std::function<void(Sp<NodeInfo>)> completeHandler) {
    auto task = std::make_shared<NodeLookup>(this, id);

    task->addListener([=](Task* t) {
        Sp<NodeInfo> ni = routingTable.getEntry(id);
        completeHandler(ni);
    });
    task->setName("User-level node lookup");
    taskMan.add(task);
    return task;
}

Sp<Task> DHT::findValue(const Id& id, LookupOption option, std::function<void(Sp<Value>)> completeHandler) {
    auto task = std::make_shared<ValueLookup>(this, id);
    Sp<Sp<Value>> valuePtr = std::make_shared<Sp<Value>>();

    task->setResultHandler([=](const Value& value, Task* t) {
        if (!*valuePtr)
            *valuePtr = std::make_shared<Value>(value);
        else if (value.isMutable() && (*valuePtr)->getSequenceNumber() < value.getSequenceNumber())
            *valuePtr = std::make_shared<Value>(value);

        if (option != LookupOption::CONSERVATIVE || !value.isMutable()) {
            t->cancel();
        }
    });

    task->addListener([=](Task*) {
        completeHandler(*valuePtr);
    });
    task->setName("User-level value lookup");
    taskMan.add(task);
    return task;
}

Sp<Task> DHT::storeValue(const Value& value, std::function<void(std::list<Sp<NodeInfo>>)> completeHandler) {
    auto task = std::make_shared<NodeLookup>(this, value.getId());
    task->setWantToken(true);
    task->addListener([=](Task* t) {
        if (t->getState() != Task::State::FINISHED)
            return;

        auto closestSet = (static_cast<NodeLookup*>(t))->getClosestSet();
        if (closestSet.size() == 0) {
            // this should never happen
            log->warn("!!! Value announce task not started because the node lookup task got the empty closest nodes.");
            completeHandler({});
            return;
        }

        auto announce = std::make_shared<ValueAnnounce>(this, closestSet, value);
        announce->addListener([=](Task*) {
            std::list<Sp<NodeInfo>> result{};
            for(const auto& item: closestSet.getEntries()) {
                result.push_back(item);
            }
            completeHandler(result);
        });
        announce->setName("Nested value Store");
        t->setNestedTask(announce);
        taskMan.add(announce);
    });

    task->setName("StoreValue task");
    taskMan.add(task);
    return task;
}

Sp<Task> DHT::findPeer(const Id& id, int expected, LookupOption option, std::function<void(std::list<PeerInfo>)> completeHandler) {
    // NOTICE: Concurrent threads adding to ArrayList
    //
    // There is no guaranteed behavior for what happens when add is
    // called concurrently by two threads on ArrayList.
    // However, it has been my experience that both objects have been
    // added fine. Most of the thread safety issues related to lists
    // deal with iteration while adding/removing.
    auto task = std::make_shared<PeerLookup>(this, id);
    auto peers = std::make_shared<std::list<PeerInfo>>();

    task->setResultHandler([=](std::list<PeerInfo>& listOfPeers, Task* self){
        peers->splice(peers->end(), listOfPeers);

        if (option != LookupOption::CONSERVATIVE && peers->size() >= expected) {
            self->cancel();
            return;
        }
    });

    task->addListener([=](Task*) {
        completeHandler(*peers);
    });

    task->setName("User-level peer lookup");
    taskMan.add(task);
    return task;
}

Sp<Task> DHT::announcePeer(const PeerInfo& peer, const std::function<void(std::list<Sp<NodeInfo>>)> completeHandler) {
    auto task = std::make_shared<NodeLookup>(this, peer.getId());
    task->setWantToken(true);
    task->addListener([=](Task* t) {
        if (t->getState() != Task::State::FINISHED)
            return;

        auto closestSet = (static_cast<NodeLookup*>(t))->getClosestSet();
        if (closestSet.size() == 0) {
            // this should never happen
            log->warn("!!! Peer announce task not started because the node lookup task got the empty closest nodes.");
            completeHandler({});
            return;
        }

        auto announce = std::make_shared<PeerAnnounce>(this, closestSet, peer);
        announce->addListener([=](Task* t) {
            std::list<Sp<NodeInfo>> result{};
            for(const auto& item: closestSet.getEntries()) {
                result.push_back(item);
            }
            completeHandler(result);
        });
        announce->setName("Nested peer announce");

        t->setNestedTask(announce);
        taskMan.add(announce);
    });

    task->setName("AnoouncePeer Task");
    taskMan.add(task);
    return task;
}

void DHT::populateClosestNodes(Sp<LookupResponse> response, const Id& target, int v4, int v6) {
    if (v4 > 0) {
        auto& dht4 = (type == Type::IPV4) ? *this : *node.getDHT(Type::IPV4);
        auto kclosestNodes = std::make_shared<KClosestNodes>(dht4, target, v4);
        kclosestNodes->fill(type == Type::IPV4);
        auto nodes = kclosestNodes->asNodeList();
        response->setNodes4(nodes);
    }

    if (v6 > 0) {
        auto& dht6 = (type == Type::IPV6) ? *this : *node.getDHT(Type::IPV6);
        auto kclosestNodes = std::make_shared<KClosestNodes>(dht6, target, v6);
        kclosestNodes->fill(type == Type::IPV6);
        auto nodes = kclosestNodes->asNodeList();
        response->setNodes6(nodes);
    }
}

std::string DHT::toString() const {
    std::string str {};

    str.append("DHT: ").append(getTypeName()).append(1, '\n');
    str.append("Address: ").append(addr.toString()).append(1, '\n');
    str.append(routingTable.toString());

    return str;
}

}
}
