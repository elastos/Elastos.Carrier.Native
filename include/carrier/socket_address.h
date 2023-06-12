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

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __ANDROID__
typedef uint16_t in_port_t;
#endif
#else
#include <iso646.h>
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef uint8_t sa_family_t;
typedef uint16_t in_port_t;
#endif

#include <string>
#include <vector>
#include <memory>
#include <cstring>

#include "def.h"
#include "blob.h"

namespace elastos {
namespace carrier {

/**
 * @brief 记录网络地址信息
 *
 */
class CARRIER_PUBLIC SocketAddress {
public:
    /**
     * @brief 创建一个空SocketAddress对象
     *
     */
    SocketAddress() {}
    /**
     * @brief SocketAddress复制拷贝
     *
     * @param o 被拷贝的SocketAddress对象
     */
    SocketAddress(const SocketAddress& o) noexcept
        : SocketAddress(o.ss) {}
    /**
     * @brief SocketAddress复制拷贝
     *
     * @param o 被拷贝的SocketAddress对象
     */
    SocketAddress(SocketAddress&& o) noexcept
        : SocketAddress(o.ss) {}
    /**
     * @brief 创建一个新的SocketAddress对象
     *
     * @param ss sockaddr_storage对象
     */
    SocketAddress(const sockaddr_storage& ss) noexcept {
        std::memcpy((uint8_t*)&this->ss, (const uint8_t*)&ss, sizeof(sockaddr_storage));
    }
    /**
     * @brief 创建一个新的SocketAddress对象
     *
     * @param sa sockaddr对象
     */
    SocketAddress(const sockaddr& sa) noexcept
        : SocketAddress((const sockaddr*)&sa) {}
    /**
     * @brief 创建一个新的SocketAddress对象
     *
     * @param sa sockaddr指针
     */
    SocketAddress(const sockaddr* sa) noexcept {
        std::memcpy((uint8_t*)&ss, (const uint8_t*)sa, sslen(sa->sa_family));
    }
    /**
     * @brief 创建一个新的SocketAddress对象
     *
     * @param ip 网络ip字符串
     * @param service 网络端口字符串
     */
    SocketAddress(const std::string& ip, const std::string& service = {});
    /**
     * @brief 创建一个新的SocketAddress对象
     *
     * @param ip 网络ip字符串
     * @param port 网络端口号
     */
    SocketAddress(const std::string& ip, in_port_t port)
        :  SocketAddress(ip, std::to_string(port)) {}
    /**
     * @brief 创建一个新的SocketAddress对象
     *
     * @param ip 网络ip字符串
     * @param port 网络地址端口号
     */
    SocketAddress(const char* ip, in_port_t port = 0)
        :  SocketAddress(std::string(ip), std::to_string(port)) {}
    /**
     * @brief 创建一个新的SocketAddress对象
     *
     * @param ip 网络地址ip字符串
     * @param port 网络地址端口号
     */
    SocketAddress(const Blob& ip, in_port_t port);
    /**
     * @brief 根据主机和服务名获取对应的网络地址集
     *
     * @param host 主机名
     * @param service 服务名
     * @return std::vector<SocketAddress> 返回网络地址集
     */
    static std::vector<SocketAddress> resolve(const std::string& host, const std::string& service = {});
    /**
     * @brief 根据主机和端口号获取对应的网络地址集
     *
     * @param host 主机名
     * @param port 服务名
     * @return std::vector<SocketAddress> 返回网络地址集
     */
    static std::vector<SocketAddress> resolve(const std::string& host, in_port_t port) {
        return resolve(host, std::to_string(port));
    }
    /**
     * @brief 重载SocketSAddress
     *
     * @param o 被重载的SocketSAddress对象
     * @return SocketAddress& 返回SocketSAddress对象
     */
    SocketAddress& operator=(const SocketAddress& o) noexcept {
        std::memcpy((uint8_t*)&ss, (const uint8_t*)&o.ss, sizeof(sockaddr_storage));
        return *this;
    }
    /**
     * @brief 重载SocketSAddress
     *
     * @param o 被重载的SocketSAddress对象
     * @return SocketAddress& 返回SocketSAddress对象
     */
    SocketAddress& operator=(SocketAddress&& o) noexcept {
        std::memcpy((uint8_t*)&ss, (const uint8_t*)&o.ss, sizeof(sockaddr_storage));
        return *this;
    }
    /**
     * @brief 重载SocketAddress的operator<
     *
     * @param o 被比较的SocketSAddress对象
     * @return true 当前SocketAddress小于指定的SocketAddress
     * @return false 当前SocketAddress不小于指定的SocketAddress
     */
    bool operator<(const SocketAddress& o) const noexcept {
        if (ss.ss_family != o.ss.ss_family)
            return ss.ss_family < o.ss.ss_family;
        return std::memcmp((const uint8_t*)addr(), (const uint8_t*)o.addr(), sslen(ss.ss_family)) < 0;
    }
    /**
     * @brief 重载SocketAddress的operator==
     *
     * @param o 被比较的SocketSAddress对象
     * @return true 当前SocketAddress等于指定的SocketAddress
     * @return false 当前SocketAddress不等于指定的SocketAddress
     */
    bool operator==(const SocketAddress& o) const noexcept {
        return ss.ss_family == o.ss.ss_family
            && std::memcmp((const uint8_t*)&ss, (const uint8_t*)&o.ss, sslen(ss.ss_family)) == 0;
    }
    /**
     * @brief 重载SocketAddress的operator!=
     *
     * @param o 被比较的SocketSAddress对象
     * @return true 当前SocketAddress不等于指定的SocketAddress
     * @return false 当前SocketAddress等于指定的SocketAddress
     */
    bool operator!=(const SocketAddress& o) const noexcept {
        return ss.ss_family != o.ss.ss_family
            || std::memcmp((const uint8_t*)&ss, (const uint8_t*)&o.ss, sslen(ss.ss_family)) != 0;
    }
    /**
     * @brief 判断当前SocketAddress是否为空对象
     *
     * @return true SocketAddress是空对象
     * @return false SocketAddress不是空对象
     */
    explicit operator bool() const noexcept {
        return !empty();
    }
    /**
     * @brief 获取SocketAddress大小
     *
     * @return socklen_t 返回大小
     */
    socklen_t length() const noexcept {
        return sslen(ss.ss_family);
    }
    /**
     * @brief 获取SocketAddress地址类型
     *
     * @return sa_family_t 返回地址族
     */
    sa_family_t family() const noexcept {
        return ss.ss_family;
    }
    /**
     * @brief 获取可读的SocketAddress的地址
     *
     * @return std::string 返回可读地址信息
     */
    std::string host() const noexcept;
    /**
     * @brief 获取SocketAddress的端口号
     *
     * @return in_port_t 返回端口号
     */
    in_port_t port() const noexcept;
    /**
     * @brief 获取SocketAddress对应的sockaddr指针
     *
     * @return const sockaddr* 返回sockaddr指针
     */
    const sockaddr* addr() const noexcept {
        return empty() ? nullptr : (const sockaddr*)&ss;
    }
    /**
     * @brief 获取SocketAddress对应的ipv4网络地址信息
     *
     * @return const sockaddr_in* 返回ipv4网络地址信息
     */
    const sockaddr_in* addr4() const noexcept {
        return reinterpret_cast<const sockaddr_in*>(addr());
    }
    /**
     * @brief 获取SocketAddress对应的ipv6网络地址信息
     *
     * @return const sockaddr_in6* 返回ipv6网络地址信息
     */
    const sockaddr_in6* addr6() const noexcept {
        return reinterpret_cast<const sockaddr_in6*>(addr());
    }
    /**
     * @brief 获取SocketAddress对应的ipv4地址指针
     *
     * @return const in_addr* 返回ipv4地址
     */
    const in_addr* inaddr4() const noexcept {
        return empty() ? nullptr : &((reinterpret_cast<const sockaddr_in*>(&ss))->sin_addr);
    }
    /**
     * @brief 获取SocketAddress对应的ipv6地址
     *
     * @return const in6_addr* 返回ipv6地址
     */
    const in6_addr* inaddr6() const noexcept {
        return empty() ? nullptr : &((reinterpret_cast<const sockaddr_in6*>(&ss))->sin6_addr);
    }
    /**
     * @brief 获取SocketAddress的二进制地址包
     *
     * @return const uint8_t* 返回二进制地址包指针
     */
    const uint8_t* inaddr() const;
    /**
     * @brief 获取SocketAddress对应的地址大小
     *
     * @return size_t 返回地址大小
     */
    size_t inaddrLength() const;
    /**
     * @brief 判断SocketAddress是否是本地通配符地址(0.0.0.0, ::0)
     *
     * @return true SocketAddress是本地通配符地址
     * @return false SocketAddress不是本地通配符地址
     */
    bool isAnyLocal() const;
    /**
     * @brief 判断SocketAddress是不是Loopback地址(127.0.0.0/8, ::1)
     *
     * @return true SocketAddress是Loopback地址
     * @return false SocketAddress不是Loopback地址
     */
    bool isLoopback() const;
    /**
     * @brief 判断SocketAddress是不是Private Ip range(10.0.0.0/8, 172.16.0.0/12,192.168.0.0/16, fc00::/7)
     *
     * @return true SocketAddress是Private ip
     * @return false SocketAddress不是Private ip
     */
    bool isSiteLocal() const;
    /**
     * @brief 判断SocketAddress是否是非法访问地址(255.255.255.255)
     *
     * @return true SocketAddress是非法访问地址
     * @return false SocketAddress不是非法访问地址
     */
    bool isBroadcast() const;
    /**
     * @brief 判断SocketAddress是否是多播地址
     *
     * @return true SocketAddress是多播地址
     * @return false SocketAddress不是多播地址
     */
    bool isMulticast() const;
    /**
     * @brief 判断SocketAddress是否是LinkLocal地址(169.254.0.0/16, fe80::/10)
     *
     * @return true SocketAddress是LinkLocal地址
     * @return false SocketAddress不是LinkLocal地址
     */
    bool isLinkLocal() const;
    /**
     * @brief 判断SocketAddress是否是虚拟地址
     *
     * @return true SocketAddress是虚拟地址
     * @return false SocketAddress不是虚拟地址
     */
    bool isBogon() const;
    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isGlobalUnicast() const;
    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isAnyUnicast() const;
    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isMappedIPv4() const;
    /**
     * @brief 获取匹配Ipv4的地址
     *
     * @return SocketAddress 返回SocketAddress对象
     */
    SocketAddress getMappedIPv4() const noexcept;
    /**
     * @brief 获取匹配Ipv6的地址
     *
     * @return SocketAddress 返回SocketAddress对象
     */
    SocketAddress getMappedIPv6() const noexcept;
    /**
     * @brief 获取可读的SocketAddress信息
     *
     * @return std::string 返回SocketAddress字符串
     */
    std::string toString() const noexcept {
        return toString(addr());
    }
    /**
     * @brief 获取可读的sockaddr信息
     *
     * @param sa sockaddr指针
     * @return std::string 返回地址信息字符串
     */
    static std::string toString(const sockaddr* sa) noexcept;
    /**
     * @brief 获取可读的sockaddr信息
     *
     * @param sa sockaddr对象
     * @return std::string 返回地址信息字符串
     */
    static std::string toString(const sockaddr& sa) noexcept {
        return toString(&sa);
    }
    /**
     * @brief 获取可读的sockaddr_storage信息
     *
     * @param ss sockaddr_storage指针
     * @return std::string 返回地址信息字符串
     */
    static std::string toString(const sockaddr_storage* ss) noexcept {
        return toString((const sockaddr*)ss);
    }
    /**
     * @brief 获取可读的sockaddr_storage信息
     *
     * @param ss sockaddr_storage对象
     * @return std::string 返回地址信息字符串
     */
    static std::string toString(const sockaddr_storage& ss) noexcept {
        return toString((const sockaddr*)&ss);
    }
    /**
     * @brief 比较SocketAddress内容是否相同
     *
     */
    class IpCompare {
    public:
        /**
         * @brief 比较SocketAddress内容是否相同
         *
         * @param a 被比较的SocketAddress
         * @param b 被比较的SocketAddress
         * @return true 两者相同
         * @return false 两者不同
         */
        bool operator()(const SocketAddress& a, const SocketAddress& b) const noexcept;
    };

private:
    static socklen_t sslen(sa_family_t family);

    bool empty() const noexcept {
        return ss.ss_family != AF_INET && ss.ss_family != AF_INET6;
    }

    sockaddr_storage ss { 0, AF_UNSPEC };
};

} // namespace carrier
} // namespace elastos
