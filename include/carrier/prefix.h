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

namespace elastos {
namespace carrier {

/**
 * @brief Id空间的前缀类，根据depth来确定前缀的长度。该类主要用来计算路由表中bucket的地址空间。
 *
 */
class CARRIER_PUBLIC Prefix : public Id {
public:
    /**
     * @brief 创建一个空Prefix对象
     *
     */
    Prefix() noexcept : Id() {}
    /**
     * @brief 创建一个新Prefix对象
     *
     * @param id Prefix所处的Id
     * @param _depth Id中的前_depth位，也是前缀长度
     */
    Prefix(const Id& id, int _depth) noexcept : depth(_depth) {
        bitsCopy(id, *this, _depth);
    }
    /**
     * @brief 复制拷贝一个新Prefix
     *
     * @param _prefix 另一个Prefix
     */
    Prefix(const Prefix& _prefix) noexcept
        : Prefix(_prefix, _prefix.getDepth()) {}
    /**
     * @brief 获取Prefix的bits大小
     *
     * @return int 返回depth大小
     */
    int getDepth() const noexcept {
        return depth;
    }
    /**
     * @brief 判断当前Prefix是否代表指定的Id
     *
     * @param id 被判断的Id
     * @return true 当前Prefix代表指定节点
     * @return false 当前Prefix不能代表指定节点
     */
    bool isPrefixOf(const Id& id) const noexcept {
        return bitsEqual(*this, id, depth);
    }
    /**
     * @brief 判断Prefix所代表的Id是否可分裂
     *
     * @return true Prefix所代表的Id可分裂
     * @return false Prefix所代表的Id不可分裂
     */
    bool isSplittable() const noexcept {
        return depth < (int)ID_BITS - 1;
    }
    /**
     * @brief 获取Prefix可代表的第一个Id
     *
     * @return Id 返回Id对象
     */
    Id first() const noexcept {
        return Id(*this);
    }
    /**
     * @brief 获取Prefix可所代表的最后一个Id
     *
     * @return Id 返回Id对象
     */
    Id last() const;
    /**
     * @brief 获取Prefix的父Prefix
     *
     * @return Prefix 返回Prefix对象
     */
    Prefix getParent() const;
    /**
     * @brief 分裂Prefix
     *
     * @param highBranch true 分裂Prefix后获取高位空间
     *                   false 分裂Prefix后获取地位空间
     * @return Prefix 返回相应的Prefix
     */
    Prefix splitBranch(bool highBranch) const;
    /**
     * @brief 判断当前Prefix和指定Prefix是否为兄弟Prefix，即当前Prefix和指定Prefix是否有同样的depth-1数据
     *
     * @param other 另一个Prefix
     * @return true 两者是兄弟Prefix
     * @return false 两者不是兄弟Prefix
     */
    bool isSiblingOf(const Prefix& other) const;
    /**
     * @brief 从当前Perfix产生可代表的随机Id
     *
     * @return Id 返回Id对象
     */
    Id createRandomId() const;
    /**
     * @brief 获取PeerInfo的二进制形式字符串
     *
     * @return std::string 返回二进制形式字符串
     */
    std::string toBinaryString() const;
    /**
     * @brief 获取可读的PeerInfo信息
     *
     * @return std::string 返回PeerInfo字符串信息
     */
    std::string toString() const;
    /**
     * @brief 判断当前PeerInfo和指定的PeerInfo是否相等
     *
     * @param prefix 指定的PeerInfo对象
     * @return true 两个PeerInfo相等
     * @return false 两个PeerInfo不相等
     */
    bool operator==(const Prefix& prefix) const {
        return equals(prefix);
    }
private:
    bool equals(const Prefix &prefix) const;

    void setTail(int bit) noexcept {
        getData()[bit >> 3] &= ~(0x80 >> (bit & 0x07));
    }

    void setPos(int pos, uint8_t data) noexcept {
        getData()[pos] = data;
    }

    void setRemainZero(int pos, int j) noexcept {
        getData()[pos] = getData()[pos] & ~(0xff >> j);
    }

    int depth {-1};
};

} // namespace carrier
} // namespace elastos
