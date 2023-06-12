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
 * @brief 用于记录版本信息（版本名和版本号）
 *
 */
class CARRIER_PUBLIC Version {
public:
    /**
     * @brief 获取版本号
     *
     * @param name 版本名称
     * @param version 版本号
     * @return int 返回版本名称和版本号经过算法得到的版本内容号
     */
    static int build(std::string& name, int version);
    /**
     * @brief 获取可读的版本信息
     *
     * @param version 版本内容号
     * @return const std::string 返回版本内容字符串
     */
    static const std::string toString(int version);
};

CARRIER_PUBLIC const char* version();

}
}
