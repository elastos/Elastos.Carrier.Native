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

#include <string>
#include <vector>

#include "def.h"
#include "configuration.h"

namespace elastos {
namespace carrier {

/**
 * @brief 提供配置Node的默认方法集
 *
 */
class CARRIER_PUBLIC DefaultConfiguration final : public Configuration {
public:
    DefaultConfiguration() = delete;
    /**
     * @brief 创建新的默认配置方法集
     *
     * @param ip4 节点ipv4地址
     * @param ip6 节点ipv6地址
     * @param port 节点端口
     * @param path 节点的存储路径
     * @param nodes 节点的引导节点列表
     * @param _services 节点的服务列表
     */
    DefaultConfiguration(const std::string& ip4, const std::string& ip6, int port,
        std::string path, std::vector<Sp<NodeInfo>> nodes, std::map<std::string, std::any> _services)
        : storagePath(path), bootstrapNodes(nodes), services(_services) {
            try {
                if (!ip4.empty())
                    addr4 = SocketAddress(ip4, port);
                if (!ip6.empty())
                    addr6 = SocketAddress(ip6, port);
            } catch(const std::exception& e) {
                // TODO:
            }
    }
    /**
     * @brief 从配置中读取ipv4地址
     *
     * @return SocketAddress& 返回ipv4地址
     */
    SocketAddress& ipv4Address() override {
        return addr4;
    }
    /**
     * @brief 从配置中读取ipv6地址
     *
     * @return SocketAddress& 返回ipv6地址
     */
    SocketAddress& ipv6Address() override {
        return addr6;
    }
    /**
     * @brief 从配置中读取监听端口
     *
     * @return int 返回监听端口
     */
    int listeningPort() override {
        return addr4.port() ? addr4.port() : addr6.port();
    }
    /**
     * @brief 从配置中获取节点存储路径
     *
     * @return const std::string& 返回节点存储路径
     */
    const std::string& getStoragePath() override {
        return storagePath;
    }
    /**
     * @brief 从配置中获取引导节点
     *
     * @return std::vector<Sp<NodeInfo>>& 返回引导节点列表
     */
    std::vector<Sp<NodeInfo>>& getBootstrapNodes() override {
        return bootstrapNodes;
    }
    /**
     * @brief 从配置中获取服务列表
     *
     * @return std::map<std::string, std::any>& 返回服务列表
     */
    std::map<std::string, std::any>& getServices() override {
        return services;
    }

    /**
     * @brief 默认配置方法集构造器，用于修改默认配置的各参数
     *
     */
    class CARRIER_PUBLIC Builder {
    public:
        /**
         * @brief 创建空构造器
         *
         */
        Builder() {}
        /**
         * @brief 设置是否使用ipv4地址
         *
         * @param automatic automatic == true：使用；automatic == false：不使用
         */
        void setAutoIPv4Address(bool automatic) {
            this->autoAddr4 = automatic;
        }
        /**
         * @brief 设置是否使用ipv6地址
         *
         * @param automatic automatic == true：使用；automatic == false：不使用
         */
        void setAutoIPv6Address(bool automatic) {
            this->autoAddr6 = automatic;
        }
        /**
         * @brief 设置是否使用ipv4和ipv6的地址
         *
         * @param automatic automatic == true：使用；automatic == false：不使用
         */
        void setAutoIPAddress(bool automatic) {
            this->autoAddr4 = automatic;
            this->autoAddr6 = automatic;
        }
        /**
         * @brief 设置或者重置节点ipv4地址
         *
         * @param ip ipv4地址字符串
         */
        void setIPv4Address(const std::string& ip) {
            this->ip4 = ip;
        }
        /**
         * @brief 设置或者重置节点ipv6地址
         *
         * @param ip ipv6地址字符串
         */
        void setIPv6Address(const std::string& ip) {
            this->ip6 = ip;
        }
        /**
         * @brief 设置或者重置节点的监听端口
         *
         * @param port 被设置的端口号
         */
        void setListeningPort(int port) {
            if (port <= 0 || port > 65535)
                throw std::invalid_argument("Invalid port: " + std::to_string(port));

            this->port = port;
        }
        /**
         * @brief 设置或者重设节点的存储路径
         *
         * @param path 存储路径字符串
         */
        void setStoragePath(const std::string& path) {
            this->storagePath = path;
        }
        /**
         * @brief 获取原配置的节点存储路径
         *
         * @return const std::string& 返回存储路径
         */
        const std::string& getStoragePath() {
            return this->storagePath;
        }
        /**
         * @brief 添加引导节点
         *
         * @param idstr 引导节点的id字符串
         * @param addr 引导节点的地址字符串
         * @param port 引导节点的端口号
         */
        void addBootstrap(const std::string& idstr, const std::string& addr, int port) {
            auto id = Id(idstr);
            auto address = SocketAddress(addr, port);
            auto node = std::make_shared<NodeInfo>(id, address);
            bootstrapNodes.emplace_back(node);
        }
        /**
         * @brief 添加引导节点
         *
         * @param id 引导节点的id内容
         * @param addr 引导节点的地址内容
         */
        void addBootstrap(const Id& id, SocketAddress addr) {
            auto node = std::make_shared<NodeInfo>(id, addr);
            bootstrapNodes.emplace_back(node);
        }
        /**
         * @brief 添加引导节点
         *
         * @param node 引导节点的节点信息
         */
        void addBootstrap(Sp<NodeInfo> node) {
            bootstrapNodes.emplace_back(node);
        }
        /**
         * @brief 导入节点配置文件
         *
         * @param path 配置文件路径
         */
        void load(const std::string& path);
        /**
         * @brief 重置节点配置编辑器
         *
         */
        void reset();
        /**
         * @brief 结束构造
         *
         * @return Sp<Configuration> 返回Configuration对象指针
         */
        Sp<Configuration> build();

    private:
        bool autoAddr4 { true };
        bool autoAddr6 { false };
        std::string ip4 {};
        std::string ip6 {};
        int port = 39001;
        std::string storagePath {};
        std::vector<Sp<NodeInfo>> bootstrapNodes {};
        std::map<std::string, std::any> services {};
    };

private:
    SocketAddress addr4 {};
    SocketAddress addr6 {};

    std::string storagePath {};
    std::vector<Sp<NodeInfo>> bootstrapNodes {};
    std::map<std::string, std::any> services {};
};

} // namespace carrier
} // namespace elastos
