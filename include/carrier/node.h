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

#include <list>
#include <vector>
#include <functional>
#include <future>

#include "def.h"
#include "types.h"
#include "id.h"
#include "socket_address.h"
#include "value.h"
#include "node_info.h"
#include "peer_info.h"
#include "log.h"
#include "configuration.h"
#include "lookup_option.h"
#include "node_status.h"
#include "node_status_listener.h"

namespace elastos {
namespace carrier {

class RPCServer;
class CryptoCache;
class TokenManager;
class DataStorage;
class DHT;

class CARRIER_PUBLIC Node{
public:
    using StatusCallback = std::function<void(NodeStatus, NodeStatus)>;

    Node(Sp<Configuration> config);

    ~Node() {}

    const Id& getId() const {
        return id;
    }

    inline bool isSelfId(const Id& id) const {
        return this->id == id;
    }

    inline Sp<Configuration> getConfig() const {
        return config;
    }

    inline void setDefaultLookupOption(LookupOption option) {
        defaultLookupOption = option;
    }

    inline void addStatusListener(NodeStatusListener& listener) {
        statusListeners.emplace_back(listener);
    }

    void removeStatusListener(const NodeStatusListener& listener) {
        //TODO:: need to implement the NodeStatusListener operator '=='
        //       Or use Sp<NodeStatusListener>?
        // statusListeners.remove(listener);
    }

    void bootstrap(const NodeInfo& node);
    void start();
    void stop();

    bool isRunning() const {
        return status == NodeStatus::Running;
    }

    std::future<std::list<Sp<NodeInfo>>> findNode(const Id& id) const {
        return findNode(id, defaultLookupOption);
    }

    std::future<Sp<Value>> findValue(const Id& id) const {
        return findValue(id, defaultLookupOption);
    }

    std::future<std::list<Sp<PeerInfo>>> findPeer(const Id &id, int expectedNum) const {
        return findPeer(id, expectedNum, defaultLookupOption);
    }

#ifdef CARRIER_CRAWLER
    void ping(Sp<NodeInfo> node, std::function<void(Sp<NodeInfo>)> completeHandler) const;
    void getNodes(const Id& id, Sp<NodeInfo> node, std::function<void(std::list<Sp<NodeInfo>>)> completeHandler) const;
#endif

    std::future<std::list<Sp<NodeInfo>>> findNode(const Id& id, LookupOption option) const;
    std::future<Sp<Value>> findValue(const Id& id, LookupOption option) const;
    std::future<bool> storeValue(const Sp<Value> value) const;
    std::future<std::list<Sp<PeerInfo>>> findPeer(const Id &id, int expectedNum, LookupOption option) const;
    std::future<bool> announcePeer(const Id& id, int port) const;

    Sp<DataStorage> getStorage() const {
		return storage;
	}

    int getPort();

    Sp<DHT> getDHT(int type) const noexcept;

    /******************************************************************************
    * Value from Node
    *****************************************************************************/
    Sp<Value> createValue(const std::vector<uint8_t>& data) {
        return Value::create(data);
    }

    Sp<Value> createSignedValue(const std::vector<uint8_t>& data) {
        return Value::createSigned(data);
    }

    Sp<Value> createEncryptedValue(const Id& target, const std::vector<uint8_t>& data){
        return Value::createEncrypted(target, data);
    }

    Sp<Value> updateValue(const Id& valueId, const std::vector<uint8_t>& data);

    std::vector<uint8_t> encrypt(const Id& recipient, const Blob& plain) const;
    std::vector<uint8_t> decrypt(const Id& sender, const Blob& cipher) const;

    void encrypt(const Id& recipient, Blob& cipher, const Blob& plain) const;
    void decrypt(const Id& sender, Blob& plain, const Blob& cipher) const;

    std::string toString() const;
private:
    bool checkPersistence(const std::string&);
    void loadKey(const std::string&);
    void initKey(const std::string&);
    void writeIdFile(const std::string&);
    void setStatus(NodeStatus expected, NodeStatus newStatus);
    void setupCryptoBoxesCache();

    Signature::KeyPair keyPair {};
    CryptoBox::KeyPair encryptionKeyPair {};
    Id id;

    bool persistent { false };
	std::string storagePath {};

    Sp<DHT> dht4 {};
    Sp<DHT> dht6 {};
    int numDHTs {0};
    LookupOption defaultLookupOption { LookupOption::CONSERVATIVE };

    NodeStatus status;
    StatusCallback statusCb {nullptr};
    std::list<NodeStatusListener> statusListeners {};

    Sp<Configuration> config {};
    Sp<TokenManager> tokenManager {};
    Sp<DataStorage> storage {};
    Sp<RPCServer> server {};
    Sp<CryptoCache> cryptoContexts {};
    Sp<Logger> log {};
};

}
}
