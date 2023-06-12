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
 * @brief 保存节点信息（节点Id，节点地址，节点版本）
 *
 */
struct CARRIER_PUBLIC NodeInfo {
    /**
     * @brief 创建一个空NodeInfo对象
     *
     */
    NodeInfo() = default;
    /**
     * @brief 创建节点信息对象
     *
     * @param id 节点Id
     * @param ip 节点ip地址
     * @param port 节点端口号
     */
    explicit NodeInfo(const Blob& id, const Blob& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}
    /**
     * @brief  创建节点信息对象
     *
     * @param id 节点Id
     * @param ip 节点ip地址
     * @param port 节点端口号
     */
    explicit NodeInfo(const std::string& id, const std::string& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}
    /**
     * @brief 创建节点信息对象
     *
     * @param id 节点Id
     * @param ip 节点ip地址
     * @param port 节点端口号
     */
    explicit NodeInfo(const Id& id, const std::string& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}
    /**
     * @brief 创建节点信息对象
     *
     * @param id 节点Id
     * @param ip 节点ip地址
     * @param port 节点端口号
     */
    explicit NodeInfo(const Id& id, const Blob& ip, int port)
        : nodeId(id), sockaddr(ip, port) {}
    /**
     * @brief 创建节点信息对象
     *
     * @param id 节点Id
     * @param addr 节点地址
     */
    explicit NodeInfo(const Id& id, const sockaddr* addr) noexcept
        : nodeId(id), sockaddr(addr) {}
    /**
     * @brief 创建节点信息对象
     *
     * @param id 节点Id
     * @param addr 节点地址
     */
    explicit NodeInfo(const Id& id, const SocketAddress& addr) noexcept
        : nodeId(id), sockaddr(addr)  {}
    /**
     * @brief 创建节点信息对象
     *
     * @param ni 节点信息对象
     */
    NodeInfo(const NodeInfo& ni)
        : nodeId(ni.nodeId), sockaddr(ni.sockaddr) {}
    /**
     * @brief 获取节点Id
     *
     * @return const Id& 返回Id对象
     */
    const Id& getId() const noexcept {
        return nodeId;
    }
    /**
     * @brief 获取节点地址
     *
     * @return const SocketAddress& 返回地址
     */
    const SocketAddress& getAddress() const noexcept {
        return sockaddr;
    }
    /**
     * @brief 获取节点端口
     *
     * @return int 返回节点端口号
     */
    int getPort() const noexcept {
        return sockaddr.port();
    }
    /**
     * @brief 获取节点版本
     *
     * @return int 返回版本号
     */
    int getVersion() const noexcept {
        return version;
    }
    /**
     * @brief 获取可读的节点版本
     *
     * @return std::string 返回版本字符串
     */
    std::string getReadableVersion() const;
    /**
     * @brief 节点地址是否为ipv4类型
     *
     * @return true 节点地址是ipv4类型
     * @return false 节点地址不是ipv4类型
     */
    bool isIPv4() const noexcept { return sockaddr.family() == AF_INET;  }
    /**
     * @brief 节点地址是否为ipv6类型
     *
     * @return true 节点地址是ipv6类型
     * @return false 节点地址不是ipv6类型
     */
    bool isIPv6() const noexcept { return sockaddr.family() == AF_INET6; }
    /**
     * @brief 当前NodeInfo与other NodeInfo是否在节点Id或者节点地址上有相同
     *
     * @param other 用来比较的NodeInfo
     * @return true 两者有相同
     * @return false 两者完全不同
     */
    virtual bool match(const NodeInfo& other) const {
        return nodeId == other.nodeId || sockaddr == other.sockaddr;
    }
    /**
     * @brief 当前NodeInfo是否与other NodeInfo相同
     *
     * @param other 用来比较的NodeInfo
     * @return true 两者相同
     * @return false 两者不同
     */
    virtual bool equals(const NodeInfo& other) const {
        return nodeId == other.nodeId && sockaddr == other.sockaddr;;
    }
    /**
     * @brief 当前NodeInfo是否与other NodeInfo相同
     *
     * @param other 用来比较的NodeInfo
     * @return true 两者相同
     * @return false 两者不同
     */
    bool operator==(const NodeInfo& other) const {
        return equals(other);
    }
    /**
     * @brief 以字符串形式获取NodeInfo内容
     *
     * @return std::string 返回字符串
     */
    operator std::string() const;
    friend std::ostream& operator<< (std::ostream& s, const NodeInfo& ni);
    /**
     * @brief 设置NodeInfo版本
     *
     * @param version 版本号
     */
    void setVersion(int version) {
        this->version = version;
    }

private:
    Id nodeId;
    SocketAddress sockaddr;
    int version {0};
};

}
}
