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

namespace elastos {
namespace carrier {

/**
 * @brief 查找节点的方式选项
 *
 */
enum class CARRIER_PUBLIC LookupOption {
    /**
     * @brief 保留字段
     *
     */
    LOCAL, /* reserved */
    /**
     * @brief 简便模式：若节点本地有待查找节点信息，直接使用本地存储的信息，返回结果；若本地没有待查找节点，则进入optimistic模式
     *
     */
    ARBITRARY,
    /**
     * @brief 快速模式：无论节点本地是否有待查节点信息，发起DHT查找请求，第一次拿到lookup的结果即刻结束，并返回结果
     *
     */
    OPTIMISTIC,
    /**
     * @brief 全流程模式：OPTIMISTIC的进化，节点要求执行完整的DHT查找请求
     *                  将过程中得到的结果进行相应处理，并返回结果
     *
     */
    CONSERVATIVE
};

} /* namespace carrier */
} /* namespace elastos */
