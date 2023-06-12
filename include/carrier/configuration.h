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
#include <any>
#include <map>

#include "def.h"
#include "types.h"
#include "node_info.h"
#include "socket_address.h"

namespace elastos {
namespace carrier {

/**
 * @brief 提供配置Node的方法
 *
 */
class CARRIER_PUBLIC Configuration {
public:
    /**
     * @brief 从配置中读取ipv4地址
     *
     * @return SocketAddress& 返回ipv4地址
     */
    virtual SocketAddress& ipv4Address() = 0;
    /**
     * @brief 从配置中读取ipv6地址
     *
     * @return SocketAddress& 返回ipv6地址
     */
    virtual SocketAddress& ipv6Address() = 0;
    /**
     * @brief 从配置中读取监听端口
     *
     * @return int 返回监听端口
     */
    virtual int listeningPort() = 0;

    /**
     * @brief 从配置中获取Node存储路径
     * @return const std::string& 返回Node存储路径
     */
    virtual const std::string& getStoragePath() = 0;

    /**
     * @brief 从配置中获取已bootstrap的节点
     *
     * @return std::vector<Sp<NodeInfo>>& 返回节点列表
     */
    virtual std::vector<Sp<NodeInfo>>& getBootstrapNodes() = 0;
    /**
     * @brief 从配置中获取服务列表
     *
     * @return std::map<std::string, std::any>& 返回服务列表
     */
    virtual std::map<std::string, std::any>& getServices() = 0;
};

} // namespace carrier
} // namespace elastos
