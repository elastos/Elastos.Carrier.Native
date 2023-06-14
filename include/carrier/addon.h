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
#include <map>
#include <any>
#include <future>

#include "node.h"

namespace elastos {
namespace carrier {

/**
 * @brief 插件类
 *
 */
class CARRIER_PUBLIC Addon {
public:
    /**
     * @brief 初始化插件
     *
     * @param node 插件对应的节点
     * @param config 插件配置
     * @return std::future<void> 无返回
     */
    virtual std::future<void> initialize(Sp<Node> node, const std::map<std::string, std::any>& config) = 0;
    /**
     * @brief 销毁插件
     *
     * @return std::future<void> 撤销插件
     */
    virtual std::future<void> deinitialize() = 0;
    /**
     * @brief 判断插件是否已被初始化
     *
     * @return true 插件已初始化
     * @return false 插件未初始化
     */
    virtual bool isInitialized() = 0;
};

/**
 * @brief 安装指定节点的多个插件
 *
 * @param node 指定节点
 * @param addons 多个插件的配置列表
 * @return true 安装成功
 * @return false 安装失败
 */
CARRIER_PUBLIC bool loadAddons(Sp<Node> node, std::map<std::string, std::any>& addons);
/**
 * @brief 卸载所有插件
 */
CARRIER_PUBLIC void unloadAddons();
/**
 * @brief 获取插件列表
 *
 * @return std::map<std::string, std::shared_ptr<Addon>>& 返回插件列表
 */
CARRIER_PUBLIC std::map<std::string, std::shared_ptr<Addon>>& getAddons();

} // namespace carrier
} // namespace elastos
