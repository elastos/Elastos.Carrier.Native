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

/**
 * @brief Carrier节点
 *
 */
class CARRIER_PUBLIC Node{
public:
    using StatusCallback = std::function<void(NodeStatus, NodeStatus)>;

    /**
     * @brief 根据配置来创建Carrier节点对象
     *
     * @param config
     */
    Node(Sp<Configuration> config);

    /**
     * @brief 销毁Carrier节点对象
     *
     */
    ~Node() {}

    /**
     * @brief 获取节点Id
     *
     * @return const Id& 返回Id
     */
    const Id& getId() const {
        return id;
    }
    /**
     * @brief 判断id是否为当前Carrier节点的Id
     *
     * @param id 待判断Id
     * @return true id是当前Carrier节点的Id
     * @return false id不是当前Carrier节点的Id
     */
    inline bool isSelfId(const Id& id) const {
        return this->id == id;
    }
    /**
     * @brief 获取当前Carrier节点的配置信息
     *
     * @return Sp<Configuration> 返回配置指针
     */
    inline Sp<Configuration> getConfig() const {
        return config;
    }
    /**
     * @brief 设置当前Carrier节点的查找节点模式
     *
     * @param option 被设置的查找模式，
     */
    inline void setDefaultLookupOption(LookupOption option) {
        defaultLookupOption = option;
    }
    /**
     * @brief 添加节点状态接受器
     *
     * @param listener 待添加节点状态接受器
     */
    inline void addStatusListener(NodeStatusListener& listener) {
        statusListeners.emplace_back(listener);
    }
    /**
     * @brief 移除节点状态接受器
     *
     * @param listener 待移除节点状态接受器
     */
    void removeStatusListener(const NodeStatusListener& listener) {
        //TODO:: need to implement the NodeStatusListener operator '=='
        //       Or use Sp<NodeStatusListener>?
        // statusListeners.remove(listener);
    }
    /**
     * @brief 当前节点发起对node节点的链接，从而加入node节点所在的DHT网络
     *
     * @param node DHT网络中的节点
     */
    void bootstrap(const NodeInfo& node);
    /**
     * @brief 启动Carrier节点
     *
     */
    void start();
    /**
     * @brief 停止Carrier节点
     *
     */
    void stop();
    /**
     * @brief 判断当前节点是否在运行状态中
     *
     * @return true 当前节点处于运行中
     * @return false 当前节点不处于运行中
     */
    bool isRunning() const {
        return status == NodeStatus::Running;
    }
    /**
     * @brief 当前节点在DHT网络上查找Carrier节点
     *
     * @param id 待查找Carrier节点Id
     * @return std::future<std::list<Sp<NodeInfo>>> 返回查找到的节点信息列表
     */
    std::future<std::list<Sp<NodeInfo>>> findNode(const Id& id) const {
        return findNode(id, defaultLookupOption);
    }
    /**
     * @brief 当前节点在DHT网络上查找Value信息
     *
     * @param id 待查找的Value Id
     * @return std::future<Sp<Value>> 返回查找到的Value列表
     */
    std::future<Sp<Value>> findValue(const Id& id) const {
        return findValue(id, defaultLookupOption);
    }
    /**
     * @brief 当前节点在DHT网络上查找Peer信息
     *
     * @param id 待查找的Peer Id
     * @param expectedNum 返回PeerInfo列表的最大容量
     * @return std::future<std::list<Sp<PeerInfo>>> 返回查找到的PeerInfo列表
     */
    std::future<std::list<Sp<PeerInfo>>> findPeer(const Id &id, int expectedNum) const {
        return findPeer(id, expectedNum, defaultLookupOption);
    }

#ifdef CARRIER_CRAWLER
    /**
     * @brief 当前Carrier节点ping 给定的Carrier节点
     *
     * @param node 待ping的Carrier节点
     * @param completeHandler ping结果返回
     */
    void ping(Sp<NodeInfo> node, std::function<void(Sp<NodeInfo>)> completeHandler) const;
    /**
     * @brief 获取目标Carrier节点的节点列表
     *
     * @param id 目标节点Id
     * @param node 目标节点信息
     * @param completeHandler 返回得到节点列表
     */
    void getNodes(const Id& id, Sp<NodeInfo> node, std::function<void(std::list<Sp<NodeInfo>>)> completeHandler) const;
#endif

    /**
     * @brief 当前节点按照option模式在DHT网络上查找Carrier节点
     *
     * @param id 被查找的Carrier节点Id
     * @param option 查找模式
     * @return std::future<std::list<Sp<NodeInfo>>> 返回查找到的Node列表
     */
    std::future<std::list<Sp<NodeInfo>>> findNode(const Id& id, LookupOption option) const;
    /**
     * @brief 当前节点按照option模式在DHT网络上查找Value信息
     *
     * @param id 被查找的Value Id
     * @param option 查找模式
     * @return std::future<Sp<Value>> 返回查找到的Value列表
     */
    std::future<Sp<Value>> findValue(const Id& id, LookupOption option) const;
    /**
     * @brief 当前节点在DHT网络上保存Value
     *
     * @param value 待保存的Value对象
     * @return std::future<bool> 返回保存结果
     */
    std::future<bool> storeValue(const Sp<Value> value) const;
    /**
     * @brief 当前节点在DHT网络上查找Peer
     *
     * @param id 被查找Peer的Id
     * @param expectedNum 返回PeerInfo列表的最大容量
     * @param option 查找模式
     * @return std::future<std::list<Sp<PeerInfo>>> 返回查找到的Peer列表
     */
    std::future<std::list<Sp<PeerInfo>>> findPeer(const Id &id, int expectedNum, LookupOption option) const;
    /**
     * @brief 当前节点在DHT网络上发布Peer
     *
     * @param id 待发布的Peer Id
     * @param port 该Peer对应的端口
     * @return std::future<bool> 返回发布结果
     */
    std::future<bool> announcePeer(const Id& id, int port) const;
    /**
     * @brief 获取当前节点对应的数据存储
     *
     * @return Sp<DataStorage> 返回数据存储指针
     */
    Sp<DataStorage> getStorage() const {
		return storage;
	}
    /**
     * @brief 获取当前节点对应的端口
     *
     * @return int 返回端口号
     */
    int getPort();
    /**
     * @brief 获取当前节点所在的DHT网络
     *
     * @param type 选择DHT网络类型, DHT::Type::IPV4或者DHT::Type::IPV6
     * @return Sp<DHT> 返回DHT网络指针
     */
    Sp<DHT> getDHT(int type) const noexcept;

    /******************************************************************************
    * Value from Node
    *****************************************************************************/

    /**
     * @brief 当前节点创建普通Value，一经生成不可更改
     *
     * @param data Value原始数据
     * @return Sp<Value> 返回Value指针
     */
    Sp<Value> createValue(const std::vector<uint8_t>& data) {
        return Value::create(data);
    }
    /**
     * @brief 当前节点创建带签名Value
     *
     * @param data Value原始数据
     * @return Sp<Value> 返回Value指针
     */
    Sp<Value> createSignedValue(const std::vector<uint8_t>& data) {
        return Value::createSigned(data);
    }
    /**
     * @brief 当前节点创建加密Value
     *
     * @param target 该Value的目标节点，只有目标节点可以通过解密看到原始数据
     * @param data Value原始数据
     * @return Sp<Value> 返回Value指针
     */
    Sp<Value> createEncryptedValue(const Id& target, const std::vector<uint8_t>& data){
        return Value::createEncrypted(target, data);
    }
    /**
     * @brief 当前节点更新已经存在的非普通Value（SignedValue和EncryptedValue）
     *
     * @param valueId 待更新的Value Id
     * @param data 待更新的原始数据
     * @return Sp<Value> 返回Value指针
     */
    Sp<Value> updateValue(const Id& valueId, const std::vector<uint8_t>& data);
    /**
     * @brief 当前节点和目标节点经过算法对原始数据加密
     *
     * @param recipient 目标节点Id
     * @param plain 待加密的原始数据
     * @return std::vector<uint8_t> 返回加密的二进制数据
     */
    std::vector<uint8_t> encrypt(const Id& recipient, const Blob& plain) const;
    /**
     * @brief 当前节点和发送节点经过算法对接受数据解密
     *
     * @param sender 发送数据的Node Id
     * @param cipher 接受到的数据，即待解密的数据
     * @return std::vector<uint8_t> 返回原始数据包
     */
    std::vector<uint8_t> decrypt(const Id& sender, const Blob& cipher) const;
    /**
     * @brief 使用Node Id的私钥来加密数据
     *
     * @param recipient 接受Node Id
     * @param cipher 加密后得到的数据
     * @param plain 被加密的裸数据
     */
    void encrypt(const Id& recipient, Blob& cipher, const Blob& plain) const;
    /**
     * @brief 使用Node Id来解密数据
     *
     * @param sender 发送Node Id
     * @param plain 解密后得到的裸数据
     * @param cipher 待解密的加密数据
     */
    void decrypt(const Id& sender, Blob& plain, const Blob& cipher) const;
    /**
     * @brief 以字符串形式获取Node内容信息
     *
     * @return std::string Node内容字符串
     */
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
