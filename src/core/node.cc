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

#include <fstream>
#include <chrono>
#include <sys/stat.h>
#include <exception>
#include <sstream>
#include <set>
#include <filesystem>

#include "carrier/node.h"
#include "carrier/node_status.h"
#include "utils/check.h"
#include "sqlite_storage.h"
#include "crypto_cache.h"
#include "dht.h"

namespace fs = std::filesystem;

#if defined(_WIN32) || defined(_WIN64)
    static const std::string PATH_SEP = "\\";
    #define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#else
    static const std::string PATH_SEP = "/";
#endif

static const std::string PATH_CWD = ".";

namespace elastos {
namespace carrier {

bool Node::checkPersistence(const std::string& path) {
    if (path.empty()) {
        log->info("Storage path disabled, DHT node will not try to persist");
        return false;
    }

    if (path == PATH_CWD)
        return true;

    struct stat _stat;
    if (stat(path.c_str(), &_stat) != 0) {
        if (errno != ENOENT) {
            log->warn("Failed to access storage path {}. DHT node will not be able to persist state", path);
            return false;
        }
        if (!fs::create_directories(path)) {
            log->warn("Creating storage path {} failed. DHT node will not be able to persist state", path);
            return false;
        }
        return true;
    }

    if (!S_ISDIR(_stat.st_mode)) {
        log->warn("Storage path {} is not a directory. DHT node will not be able to persist state", path);
        return false;
    }
    return true;
}

void Node::loadKey(const std::string& keyPath) {
    std::ifstream in(keyPath, std::ifstream::in);
    if (!in.is_open())
        throw std::runtime_error("Can not read the key file.");

    // TODO: should read as binary
    std::stringstream ss;
    ss << in.rdbuf();
    auto keystr = ss.str();
    in.close();

    if (keystr.empty())
        throw std::runtime_error("Key file is empty.");

    keyPair = Signature::KeyPair({(uint8_t*)keystr.c_str(), keystr.size()});
}

void Node::initKey(const std::string& keyPath) {
    keyPair = Signature::KeyPair();

    std::ofstream os(keyPath);
    if (!os.is_open())
        throw std::runtime_error("Failed to open file for writing.");

    os.write(reinterpret_cast<const char*>(keyPair.privateKey().bytes()), keyPair.privateKey().size());
}

void Node::writeIdFile(const std::string& idPath) {
    if (idPath.empty())
        throw std::invalid_argument("File path is empty");

    std::ofstream os(idPath);
    if (!os.is_open())
        throw std::runtime_error("Failed to open file " + idPath);

    auto idstr = static_cast<std::string>(id);
    os.write(idstr.c_str(), idstr.size());
    os.close();
}

void Node::setStatus(NodeStatus expected, NodeStatus newStatus) {
    if (status != expected) {
        log->warn("Set status failed, expected is {}, actual is {}", statusToString(expected), statusToString(status));
        return;
    }

    auto old = status;
    status = newStatus;
    if (!statusListeners.empty()) {
        for (auto& listener: statusListeners)
            listener.statusChanged(newStatus, old);
    }
}

Node::Node(std::shared_ptr<Configuration> _config): config(_config)
{
    log = Logger::get("node");

    if (!config->ipv4Address() && !config->ipv6Address()) {
        log->error("No valid IPv4 or IPv6 address specified");
        throw std::invalid_argument("No listening address");
    }

#ifdef CARRIER_DEVELOPMENT
    log->info("Carrier node running in development environment.");
#endif

    storagePath = config->getStoragePath().empty() ? PATH_CWD: config->getStoragePath();
    persistent  = checkPersistence(storagePath);
    std::string keyPath {};
    if (persistent) {
        keyPath.reserve(storagePath.size() + 5);
        keyPath += storagePath;
        keyPath += PATH_SEP;
        keyPath += "key";

        struct stat _stat {};
        auto rc = stat(keyPath.c_str(), &_stat);
        if (rc == -1) {
            initKey(keyPath);
        } else if (S_ISDIR(_stat.st_mode)) {
            log->warn("Key file path {} is an existing directory. DHT node will not be able to persist node key", keyPath);
        } else {
            loadKey(keyPath);
        }
    }

    id = Id(keyPair.publicKey());
    if (persistent) {
        std::string idPath {};
        idPath.reserve(storagePath.size() + 4);
        idPath += storagePath;
        idPath += PATH_SEP;
        idPath += "id";
        writeIdFile(idPath);
    }

    log->info("Carrier Kademlia node {}", static_cast<std::string>(id));

    encryptionKeyPair = CryptoBox::KeyPair::fromSignatureKeyPair(keyPair);

    setupCryptoBoxesCache();

    tokenManager = std::make_shared<TokenManager>();
    defaultLookupOption = LookupOption::CONSERVATIVE;
    status = NodeStatus::Stopped;
}

void Node::bootstrap(const NodeInfo& node) {
    // checkArgument(node != null, "Invalid bootstrap node");

    if (node.getId() == id) {
        log->warn("Can not bootstrap from local node: {}", node.getId());
        return;
    }

    if (dht4 != nullptr)
        dht4->bootstrap(node);
    if (dht6 != nullptr)
        dht6->bootstrap(node);
}

void Node::start() {
    if (status != NodeStatus::Stopped)
        return;

    setStatus(NodeStatus::Stopped, NodeStatus::Initializing);
    log->info("Carrier node {} is starting...", static_cast<std::string>(id));

    if (config->ipv4Address()) {
        dht4 = std::make_shared<DHT>(DHT::Type::IPV4, *this, config->ipv4Address());
        if (persistent)
            dht4->enablePersistence(storagePath + "/dht4.cache");
    }
    if (config->ipv6Address()) {
        dht6 = std::make_shared<DHT>(DHT::Type::IPV6, *this, config->ipv6Address());
        if (persistent)
            dht6->enablePersistence(storagePath + "/dht6.cache");
    }

    setStatus(NodeStatus::Initializing, NodeStatus::Running);

    server = std::make_shared<RPCServer>(*this, dht4, dht6);
    auto& scheduler = server->getScheduler();

    std::string dbPath {};
    dbPath.reserve(storagePath.size() + 10);
    dbPath += storagePath;
    dbPath += PATH_SEP;
    dbPath += "node.db";

    storage = SqliteStorage::open(dbPath, scheduler);

    //Start crypto context loading cache check expriration
    scheduler.add([&]() {
        cryptoContexts->handleExpiration();
    }, CryptoCache::EXPIRED_CHECK_INTERVAL, CryptoCache::EXPIRED_CHECK_INTERVAL);

    server->start();

    auto nodes = config->getBootstrapNodes();
    if (dht4 != nullptr) {
        dht4->setServer(server);
        dht4->setTokenManager(tokenManager);
        dht4->start(nodes);
        numDHTs++;
    }
    if (dht6 != nullptr) {
        dht6->setServer(server);
        dht6->setTokenManager(tokenManager);
        dht6->start(nodes);
        numDHTs++;
    }

    auto job = scheduler.add([&]() {
        persistentAnnounce();
    }, 60000, Constants::RE_ANNOUNCE_INTERVAL);
    scheduledActions.emplace_back(job);
}

void Node::stop() {
    if (status == NodeStatus::Stopped)
        return;

    log->info("Carrier Kademlia node {} is stopping...", static_cast<std::string>(id));

    for (auto any : scheduledActions) {
        auto job = std::any_cast<Sp<Scheduler::Job>>(any);
        job->cancel();
    }
    scheduledActions.clear();

    if (server != nullptr) {
        server->stop();
        server.reset();
    }

    if (dht4 != nullptr) {
        dht4->stop();
        dht4.reset();
    }
    if (dht6 != nullptr) {
        dht6->stop();
        dht6.reset();
    }

    try {
        if (storage != nullptr) {
            storage->close();
            storage.reset();
        }
    } catch (std::exception& e) {
        log->error("Close data storage failed: {}", e.what());
    }

    setStatus(NodeStatus::Running, NodeStatus::Stopped);
    log->info("Carrier Kademlia node {} stopped", static_cast<std::string>(id));
}

void Node::persistentAnnounce() {
    log->info("Re-announce the persistent values and peers...");

    std::list<std::future<void>> futures {};

    auto ts = currentTimeMillis() - Constants::MAX_VALUE_AGE +
            Constants::RE_ANNOUNCE_INTERVAL * 2;
    std::list<Value> vs = storage->getPersistentValues(ts);
    for (auto v : vs) {
        log->debug("Re-announce the value: {}", v.getId());
        storage->updateValueLastAnnounce(v.getId());
        futures.emplace_back(doStoreValue(v));
    }

    ts = currentTimeMillis() - Constants::MAX_PEER_AGE +
            Constants::RE_ANNOUNCE_INTERVAL * 2;

    std::list<PeerInfo> ps = storage->getPersistentPeers(ts);
    for (auto p : ps) {
        log->debug("Re-announce the peer: {}", p.getId());
        storage->updatePeerLastAnnounce(p.getId(), p.getOrigin());
        futures.emplace_back(doAnnouncePeer(p));
    }

    for (std::list<std::future<void>>::iterator it = futures.begin(); it != futures.end(); it++) {
        (*it).get();
    }
}

#ifdef CARRIER_CRAWLER
void Node::ping(Sp<NodeInfo> node, std::function<void(Sp<NodeInfo>)> completeHandler) const {
    if (node->isIPv4()) {
        dht4->ping(node, completeHandler);
    }
    else {
        dht6->ping(node, completeHandler);
    }
}

void Node::getNodes(const Id& id, Sp<NodeInfo> node, std::function<void(std::list<Sp<NodeInfo>>)> completeHandler) const {
    if (node->isIPv4()) {
        dht4->getNodes(id, node, completeHandler);
    }
    else {
        dht6->getNodes(id, node, completeHandler);
    }

}
#endif

std::future<std::list<Sp<NodeInfo>>> Node::findNode(const Id& id, LookupOption option) const {
    checkState(isRunning(), "Node not running");
	checkArgument(id != Id::MIN_ID, "Invalid peer id");

    auto promise = std::make_shared<std::promise<std::list<Sp<NodeInfo>>>>();
    auto results = std::make_shared<std::list<Sp<NodeInfo>>>();

    if (option == LookupOption::ARBITRARY) {
        if (dht4 != nullptr) {
            const auto ni = dht4->getNode(id);
            if (ni != nullptr)
                results->emplace_back(ni);
        }
        if (dht6 != nullptr) {
            const auto ni = dht6->getNode(id);
            if (ni != nullptr)
                results->emplace_back(ni);
        }

        if (!results->empty()) {
            promise->set_value(std::move(*results));
            return promise->get_future();
        }
    }

    auto completion = std::make_shared<std::atomic<int>>(0);
    auto completeHandler = [=](Sp<NodeInfo> ni) {
        (*completion)++;
        if (ni != nullptr)
            results->emplace_back(ni);

        if ((option == LookupOption::OPTIMISTIC && !results->empty()) || *completion >= numDHTs) {
            promise->set_value(std::move(*results));
        }
    };

    if (dht4 != nullptr)
        dht4->findNode(id, completeHandler);
    if (dht6 != nullptr)
        dht6->findNode(id, completeHandler);

    return promise->get_future();
}

std::future<Sp<Value>> Node::findValue(const Id& id, LookupOption option) const {
    checkState(isRunning(), "Node not running");
	checkArgument(id != Id::MIN_ID, "Invalid peer id");

    auto promise = std::make_shared<std::promise<Sp<Value>>>();
    auto valuePtr = std::make_shared<Sp<Value>>();

    auto localVal = getStorage()->getValue(id);
    if (localVal != nullptr && (option == LookupOption::ARBITRARY || !localVal->isMutable())) {
        promise->set_value(localVal);
        return promise->get_future();
    }

    *valuePtr = localVal;

    auto completion = std::make_shared<std::atomic<int>>(0);
    auto completeHandler = [=](Sp<Value> value) {
        (*completion)++;

        if (value != nullptr) {
            if (!*valuePtr || !value->isMutable() || (*valuePtr)->getSequenceNumber() < value->getSequenceNumber()) {
                *valuePtr = value;
            }
        }

        if ((option == LookupOption::OPTIMISTIC && value != nullptr) || *completion >= numDHTs) {
            try {
                if ((*valuePtr) != nullptr)
                    getStorage()->putValue(**valuePtr);
            } catch (const std::exception& e) {
                log->warn("Perisist value in local storage failed {}", e.what());
            }
            promise->set_value((*valuePtr));
        }
    };

    if (dht4 != nullptr)
        dht4->findValue(id, option, completeHandler);
    if (dht6 != nullptr)
        dht6->findValue(id, option, completeHandler);

    return promise->get_future();
}

std::future<void> Node::storeValue(const Value& value, bool persistent) const {
    checkState(isRunning(), "Node not running");
    // checkArgument(value != nullptr, "Invalid value: null");
    checkArgument(value.isValid(), "Invalid value");

    if (!isRunning())
        throw std::runtime_error("Node is not running");

    auto promise = std::promise<void>();
    try {
        getStorage()->putValue(value, persistent);
    } catch (std::exception& ex) {
        log->error("Perisist value in local storage failed {}", ex.what());
        promise.set_exception(std::current_exception());
        return promise.get_future();
    }

    return doStoreValue(value);
}

std::future<void> Node::doStoreValue(const Value& value) const {
    auto promise = std::make_shared<std::promise<void>>();
    auto completion = std::make_shared<std::atomic<int>>(0);
    auto completeHandler = [=](std::list<Sp<NodeInfo>> nl) {
        (*completion)++;
        if (*completion >= numDHTs)
            promise->set_value();
    };

    if (dht4 != nullptr)
        dht4->storeValue(value, completeHandler);
    if (dht6 != nullptr)
        dht6->storeValue(value, completeHandler);

    return promise->get_future();
}

std::future<std::list<PeerInfo>> Node::findPeer(const Id& id, int expected, LookupOption option) const {
    checkState(isRunning(), "Node not running");
	checkArgument(id != Id::MIN_ID, "Invalid peer id");

    auto promise = std::make_shared<std::promise<std::list<PeerInfo>>>();
    auto dedup_result = std::make_shared<std::set<PeerInfo>>();
    auto results = std::make_shared<std::list<PeerInfo>>();

    auto peers = getStorage()->getPeer(id, expected);
    for (const auto& item : peers) {
        auto rc = dedup_result->insert(item);
        if (rc.second)
            results->push_back(item);
    }
    if (expected > 0 && results->size() >= expected && option == LookupOption::ARBITRARY) {
        promise->set_value(std::move(*results));
        return promise->get_future();
    }

    // TODO exception

    // NOTICE: Concurrent threads adding to ArrayList
    //
    // There is no guaranteed behavior for what happens when add is
    // called concurrently by two threads on ArrayList.
    // However, it has been my experience that both objects have been
    // added fine. Most of the thread safety issues related to lists
    // deal with iteration while adding/removing.

    auto completion = std::make_shared<std::atomic<int>>(0);
    auto completeHandler = [=](std::list<PeerInfo> peers) {
        (*completion)++;

        for (const auto &item : peers) {
            auto rc = dedup_result->insert(item);
            if (rc.second)
                results->push_back(item);
        }

        getStorage()->putPeer(*results);

        if (*completion >= numDHTs) {
            promise->set_value(std::move(*results));
        }
    };

    if (dht4 != nullptr)
        dht4->findPeer(id, expected, option, completeHandler);
    if (dht6 != nullptr)
        dht6->findPeer(id, expected, option, completeHandler);

    return promise->get_future();
}

std::future<void> Node::announcePeer(const PeerInfo& peer, bool persistent) const {
    checkState(isRunning(), "Node not running");
    // checkArgument(peer != nullptr, "Invalid peer: null");
    checkArgument(!peer.getOrigin() == getId(), "Invaid peer: not belongs to current node");
    checkArgument(peer.isValid(), "Invalid peer");

    auto promise = std::promise<void>();

    try {
        getStorage()->putPeer(peer, persistent);
    } catch (std::exception& ex) {
        log->error("Perisist peer in local storage failed {}", ex.what());
        promise.set_exception(std::current_exception());
        return promise.get_future();
    }

    return doAnnouncePeer(peer);
}

std::future<void> Node::doAnnouncePeer(const PeerInfo& peer) const {
    auto promise = std::make_shared<std::promise<void>>();

    auto completion = std::make_shared<std::atomic<int>>(0);
    auto completeHandler = [=](std::list<Sp<NodeInfo>> nl) {
        (*completion)++;
        if (*completion >= numDHTs)
            promise->set_value();
    };

    if (dht4 != nullptr)
        dht4->announcePeer(peer, completeHandler);
    if (dht6 != nullptr)
        dht6->announcePeer(peer, completeHandler);

	return promise->get_future();
}

Sp<Value> Node::getValue(const Id& valueId) {
    checkArgument(valueId != Id::MIN_ID, "Invalid value id");

	return getStorage()->getValue(valueId);
}

bool Node::removeValue(const Id& valueId) {
    checkArgument(valueId != Id::MIN_ID, "Invalid value id");

	return getStorage()->removeValue(valueId);
}

Sp<PeerInfo> Node::getPeer(const Id& peerId) {
    checkArgument(peerId != Id::MIN_ID, "Invalid peer id");

	return getStorage()->getPeer(peerId, this->getId());
}

bool Node::removePeer(const Id& peerId) {
    checkArgument(peerId != Id::zero(), "Invalid peer id");

	return getStorage()->removePeer(peerId, this->getId());
}

void Node::setupCryptoBoxesCache() {
    cryptoContexts = std::make_shared<CryptoCache>(encryptionKeyPair);
}

std::vector<uint8_t> Node::encrypt(const Id& recipient, const Blob& plain) const {
    auto ctx = cryptoContexts->get(recipient);
    return ctx.encrypt(plain);
}

std::vector<uint8_t> Node::decrypt(const Id& sender, const Blob& cipher) const {
    auto ctx = cryptoContexts->get(sender);
    return ctx.decrypt(cipher);
}

void Node::encrypt(const Id& recipient, Blob& cipher, const Blob& plain) const {
    auto ctx = cryptoContexts->get(recipient);
    ctx.encrypt(cipher, plain);
}

void Node::decrypt(const Id& sender, Blob& plain, const Blob& cipher) const {
    auto ctx = cryptoContexts->get(sender);
    ctx.decrypt(plain, cipher);
}

std::vector<uint8_t> Node::sign(const std::vector<uint8_t>& data) const {
    return keyPair.privateKey().sign(data);
}

bool Node::verify(const std::vector<uint8_t>& data, const std::vector<uint8_t>& signature) const {
    return keyPair.publicKey().verify(signature, data);
}

int Node::getPort() {
    int port = config->listeningPort();
    return port <= 0 || port > 65535 ? Constants::DEFAULT_DHT_PORT : port;
}

Sp<DHT> Node::getDHT(int type) const noexcept {
    return type == DHT::Type::IPV4 ? dht4 : dht6;
}

std::string Node::toString() const {
    std::string str {};

    str.append("Node: ").append(id).append(1, '\n');
    if (dht4 != nullptr)
        str.append(dht4->toString());

    if (dht6 != nullptr)
        str.append(dht6->toString());

    return str;
}

}
}
