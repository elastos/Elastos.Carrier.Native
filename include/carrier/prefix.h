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
 * @brief
 *
 */
class CARRIER_PUBLIC Prefix : public Id {
public:
    /**
     * @brief 创建空Prefix对象
     *
     */
    Prefix() noexcept : Id() {}
    /**
     * @brief 创建新的Prefix对象
     *
     * @param id Prefix基于的Node Id
     * @param _depth  identifies the first bit of a key that has to be equal to be considered as
	 *                covered by this prefix -1 = prefix matches whole keyspace 0 = 0th bit must
	 *                match 1 = ...
     */
    Prefix(const Id& id, int _depth) noexcept : depth(_depth) {
        bitsCopy(id, *this, _depth);
    }
    /**
     * @brief Prefix的复制拷贝
     *
     * @param _prefix 被拷贝的PeerInfo对象
     */
    Prefix(const Prefix& _prefix) noexcept
        : Prefix(_prefix, _prefix.getDepth()) {}
    /**
     * @brief 获取Prefix的depth
     *
     * @return int 返回depth
     */
    int getDepth() const noexcept {
        return depth;
    }
    /**
     * @brief 判断当前Prefix是否关联指定节点
     *
     * @param id 被判断的Node Id
     * @return true 当前Prefix关联指定节点
     * @return false 当前Prefix不关联指定节点
     */
    bool isPrefixOf(const Id& id) const noexcept {
        return bitsEqual(*this, id, depth);
    }
    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool isSplittable() const noexcept {
        return depth < (int)ID_BITS - 1;
    }
    /**
     * @brief
     *
     * @return Id
     */
    Id first() const noexcept {
        return Id(*this);
    }
    /**
     * @brief
     *
     * @return Id
     */
    Id last() const;
    /**
     * @brief Get the Parent object
     *
     * @return Prefix
     */
    Prefix getParent() const;
    /**
     * @brief
     *
     * @param highBranch
     * @return Prefix
     */
    Prefix splitBranch(bool highBranch) const;
    /**
     * @brief
     *
     * @param other
     * @return true
     * @return false
     */
    bool isSiblingOf(const Prefix& other) const;
    /**
     * @brief 从当前Perfix产生随机的Id
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
     * @brief 判断当前PeerInfo和指定的PeerInfo是否相同
     *
     * @param prefix 指定的PeerInfo对象
     * @return true 两者相同
     * @return false 两者不同
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

    /**
     * identifies the first bit of a key that has to be equal to be considered as
     * covered by this prefix -1 = prefix matches whole keyspace 0 = 0th bit must
     * match 1 = ...
     */
    int depth {-1};
};

} // namespace carrier
} // namespace elastos
