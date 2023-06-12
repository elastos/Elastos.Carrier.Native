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
#include "def.h"

namespace elastos {
namespace carrier {

/**
 * @brief 记录节点状态
 *
 */
enum class CARRIER_PUBLIC NodeStatus {
    /**
     * @brief 节点已停止
     *
     */
	Stopped,
    /**
     * @brief 节点仅初始化，未运行
     *
     */
	Initializing,
    /**
     * @brief 节点正在运行中
     *
     */
	Running
};

/**
 * @brief
 *
 * @param status
 * @return std::string
 */
inline std::string statusToString(NodeStatus status) noexcept {
    switch (status) {
    case NodeStatus::Initializing:
        return "initializing";
    case NodeStatus::Running:
        return "running";
    default:
        return "stopped";
    }
}

} /* namespace carrier */
} /* namespace elastos */
