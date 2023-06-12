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

#include "def.h"
#include "id.h"
#include "socket_address.h"

namespace elastos {
namespace carrier {

/**
 * @brief
 *
 */
struct CARRIER_PUBLIC PeerInfo {
    /**
     * @brief 创建空PeerInfo对象
     *
     */
    PeerInfo() = default;
    /**
     * @brief 创建新PeerInfo对象
     *
     * @param id 发布Peer服务的Node Id
     * @param ip 发布Peer服务的Node ip地址
     * @param port Peer服务对应的节点端口号
     */
    explicit PeerInfo(const Blob& id, const Blob& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}
    /**
     * @brief 创建新PeerInfo对象
     *
     * @param id 发布Peer服务的Node Id
     * @param ip 发布Peer服务的Node ip地址
     * @param port Peer服务对应的节点端口号
     */
    explicit PeerInfo(const Id& id, const std::string& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}
    /**
     * @brief 创建新PeerInfo对象
     *
     * @param id 发布Peer服务的Node Id
     * @param ip 发布Peer服务的Node ip地址
     * @param port Peer服务对应的节点端口号
     */
    explicit PeerInfo(const Id& id, const Blob& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}
    /**
     * @brief 创建新PeerInfo对象
     *
     * @param id 发布Peer服务的Node Id
     * @param _addr 发布Peer服务的Node信息
     */
    PeerInfo(const Id& id, const SocketAddress& _addr) noexcept
        : nodeId(id), sockaddr(_addr)  {}
    /**
     * @brief PeerInfo复制拷贝
     *
     * @param ni PeerInfo信息
     */
    PeerInfo(const PeerInfo& ni)
        : nodeId(ni.nodeId), sockaddr(ni.sockaddr) {}
    /**
     * @brief 获取Peer服务的发布节点Id
     *
     * @return const Id& 返回节点Id
     */
    const Id& getNodeId() const noexcept {
        return nodeId;
    }
    /**
     * @brief 获取Peer服务的发布节点地址
     *
     * @return const SocketAddress& 返回节点地址对象
     */
    const SocketAddress& getAddress() const noexcept {
        return sockaddr;
    }
    /**
     * @brief 获取Peer服务对应的端口号
     *
     * @return int 端口号
     */
    int getPort() const noexcept {
        return sockaddr.port();
    };
    /**
     * @brief 判断Peer服务是否发布在Ipv4网络上
     *
     * @return true Peer服务发布在Ipv4网络上
     * @return false Peer服务不是发布在Ipv4网络上
     */
    bool isIPv4() const noexcept { return sockaddr.family() == AF_INET;  }
    /**
     * @brief 判断Peer服务是否发布在Ipv6网络上
     *
     * @return true Peer服务发布在Ipv6网络上
     * @return false Peer服务发布在Ipv6网络上
     */
    bool isIPv6() const noexcept { return sockaddr.family() == AF_INET6; }
    /**
     * @brief 判断当前PeerInfo和other PeerInfo是否相同
     *
     * @param other 参照的PeerInfo
     * @return true 当前PeerInfo和other PeerInfo相同
     * @return false 当前PeerInfo和other PeerInfo不同
     */
    bool operator==(const PeerInfo& other) const {
        return nodeId == other.nodeId && sockaddr == other.sockaddr;
    }
    /**
     * @brief 判断当前Peer服务和other Peer服务的大小关系（实际是根据判断Peer服务的发布Node大小而定）
     *
     * @param other 用于比较的PeerInfo
     * @return true 当前PeerInfo小于other PeerInfo
     * @return false 当前PeerInfo不小于other PeerInfo
     */
    bool operator<(const PeerInfo& other) const {
        return nodeId.compareTo(other.nodeId) < 0;
    }
    /**
     * @brief 获取可读的PeerInfo信息
     *
     * @return std::string 返回PeerInfo字符串内容
     */
    operator std::string() const;
    friend std::ostream& operator<< (std::ostream& s, const PeerInfo& pi);

private:
    friend class SqliteStorage;
    void setNodeId(const Blob& id) {
        this->nodeId = Id(id);
    }

    void setSocketAddress(const Blob& ip, in_port_t port) {
        this->sockaddr = SocketAddress(ip, port);
    }

private:
    Id nodeId {};
    SocketAddress sockaddr {};
};

}
}
