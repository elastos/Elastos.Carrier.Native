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

#include <array>
#include <vector>
#include <stdexcept>
#include <string>

#include "def.h"
#include "types.h"
#include "blob.h"
#include "crypto_core.h"

#define ID_BYTES   32u
#define ID_BITS    256u

namespace elastos {
namespace carrier {

/**
 * @brief 用来标识节点，具有唯一性
 *
 */
class CARRIER_PUBLIC Id {
public:
    static Id MIN_ID;
    static Id MAX_ID;

    Id() noexcept = default;
    Id(const Id& id) = default;

    /**
     * @brief 构造新节点Id
     *
     * @param id Id内容
     */
    Id(const Blob& id) {
        if (!id || id.size() != ID_BYTES)
            throw std::invalid_argument("Binary id should be " + std::to_string(ID_BYTES) + " bytes long.");

        std::memcpy(bytes.data(), id.ptr(), id.size());
    }
    /**
     * @brief 构造新节点Id。Id内容实质上由公钥数据填充，两者有强相关。
     *
     * @param key 用于生成Id内容的公钥
     */
    Id(const Signature::PublicKey& key)
        : Id(key.blob()) {}
    /**
     * @brief 构造新节点Id
     *
     * @param id 用于填充Id内容的字符串，字符串需要经过Base58编码或者Hex编码
     */
    explicit Id(const std::string& id) {
        id.find("0x") == 0 ? fromHexString(id) : fromBase58String(id);
    }
    /**
     * @brief 获取Id内容
     *
     * @return constexpr const uint8_t* 返回Id内容指针
     */
    constexpr const uint8_t* data() const noexcept {
        return bytes.data();
    }
    /**
     * @brief 获取Id内容容量
     *
     * @return constexpr size_t 返回容量大小
     */
    constexpr size_t size() const noexcept {
        return ID_BYTES;
    }
    /**
     * @brief 获取Id头指针，指向Id第一个二进制内容
     *
     * @return constexpr const uint8_t* 返回头指针
     */
    constexpr const uint8_t* cbegin() const noexcept {
        return bytes.data();
    }
    /**
     * @brief 获取Id尾指针，指向Id最后一个二进制内容
     *
     * @return constexpr const uint8_t* 返回尾指针
     */
    constexpr const uint8_t* cend() const noexcept {
        return bytes.data() + ID_BYTES;
    }
    /**
     * @brief 获取Id二进制内容
     *
     * @return const Blob 返回Id二进制包
     */
    const Blob blob() const noexcept {
        return static_cast<bool>(*this) ? Blob(bytes) : Blob();
    }
    /**
     * @brief 获取生成Id内容的公钥
     *
     * @return Sp<Signature::PublicKey> 返回公钥指针
     */
    Sp<Signature::PublicKey> toKey() const {
        return std::make_shared<Signature::PublicKey>(blob());
    }
    /**
     * @brief 根据Hex字符串来获取Id
     *
     * @param hexId 经过Hex编码的Id字符串，不符合要求会报错，无法得到Id对象
     * @return Id 返回Id对象
     */
    static Id ofHex(const std::string& hexId);
    /**
     * @brief 根据Base58编码字符串来获取Id
     *
     * @param base58Id 经过Base58编码的Id字符串，不符合要求会报错，无法得到Id对象
     * @return Id 返回Id对象
     */
    static Id ofBase58(const std::string& base58Id);
    /**
     * @brief 随机生成一个Id
     *
     * @return Id 返回Id对象
     */
    static Id random();
    /**
     * @brief 生成一个空内容Id
     *
     * @return Id 返回Id对象
     */
    static Id zero() {
        return Id();
    }

    /**
     * @brief 计算当前Id与other Id之间的距离，距离结果放在返回的Id对象中
     *
     * @param other another 参照物Id
     *
     * @return 返回由距离内容填充的Id对象
     */
    Id distance(const Id& other) const;

    /**
     * @brief 计算当前Id与other Id之间的距离，距离结果放在返回的Id对象中
     *
     * @param id1 用于计算距离的Id
     * @param id2 用于计算距离的Id
     * @return Id 返回由距离内容填充的Id对象
     */
    static Id distance(const Id& id1, const Id& id2);

    /**
     * @brief 使用异或度量方法来比较当前Id与两个Id之间的距离长短。
     *
     * @param id1 用于计算距离的第一个Id
     * @param id2 用于计算距离的第二个Id
     * @return 如果id1更接近当前Id，返回-1；如果id1和id2与当前Id距离相同，返回0；如果id2更接近当前Id，返回1
     */
    int threeWayCompare(const Id& id1, const Id& id2) const;

    /**
     * @brief 判断两个Id的前n个bits是否相等
     *
     * @param id1 用来比较的第一个Id
     * @param id2 用来比较的第二个Id
     * @param n 需要比较的前bits个数
     * @return true 两个Id的前n个bits相等
     * @return false 两个Id的前n个bits不等
     */
    static bool bitsEqual(const Id& id1, const Id& id2, int n);
    /**
     * @brief 将src Id的前depth个bits复制到dest Id中
     *
     * @param src 源Id
     * @param dest 目的Id
     * @param depth 需要复制的bits的个数
     */
    static void bitsCopy(const Id& src, Id& dest, int depth);
    /**
     * @brief 比较当前Id和other Id
     *
     * @param other 待需要比较的Id
     * @return int 当前Id小于other Id，返回-1；当前Id等于other Id，返回0；当前Id大于other Id，返回1
     */
    int compareTo(const Id& other) const {
        return std::memcmp(data(), other.data(), ID_BYTES);
    }
    /**
     * @brief 获取Id的Hex编码字符串
     *
     * @return const std::string 返回Hex字符串
     */
    const std::string toHexString() const;
    /**
     * @brief 获取Id的Base58编码字符串
     *
     * @return const std::string 返回Base58字符串
     */
    const std::string toBase58String() const;
    /**
     * @brief 获取Id的二进制形式字符串
     *
     * @return const std::string 返回二进制字符串
     */
    const std::string toBinaryString() const;
    /**
     * @brief 判断当前Id和other Id是否相等
     *
     * @param other 待比较的Id
     * @return true 当前Id与other Id相等
     * @return false 当前Id与other Id不等
     */
    bool operator==(const Id& other) const { return compareTo(other) == 0; }
    /**
     * @brief 判断当前Id和other Id是否不等
     *
     * @param other 待比较的Id
     * @return true 当前Id与other Id不等
     * @return false 当前Id与other Id相等
     */
    bool operator!=(const Id& other) const { return compareTo(other) != 0; }
    /**
     * @brief 判断当前Id是否小于other Id
     *
     * @param other 待比较的Id
     * @return true 当前Id小于other Id
     * @return false 当前Id不小于other Id
     */
    bool operator<(const Id& other) const;
    /**
     * @brief 将other Id赋值给当前Id
     *
     * @param other 被赋值Id
     * @return Id& 返回当前Id
     */
    Id& operator=(const Id& other) noexcept {
        std::memcpy(bytes.data(), other.data(), ID_BYTES);
        return *this;
    }
    /**
     * @brief 将other Id内容传递给当前Id
     *
     * @param other 被传递Id
     * @return Id& 返回当前Id
     */
    Id& operator=(Id&& other) noexcept {
        std::memcpy(bytes.data(), other.bytes.data(), ID_BYTES);
        std::memset(other.bytes.data(), 0, ID_BYTES);
        return *this;
    }

    /**
     * @brief 获取当前Id字符串
     *
     * @return std::string 返回字符串
     */
    operator std::string() const { return toBase58String(); }
    /**
     * @brief 判断当前Id是否为空Id
     *
     * @return true 当前Id为空Id
     * @return false 当前Id为非空Id
     */
    operator bool() const {
        return !std::all_of(bytes.cbegin(), bytes.cend(), [](uint8_t i){ return !i; });
    }
    friend std::ostream& operator<< (std::ostream& os, const Id& id);

protected:
    std::array<uint8_t, ID_BYTES>& getData() { return bytes; }

private:
    void fromBase58String(const std::string&);
    void fromHexString(const std::string&);

    // Counts the number of leading 0's in this Id
    int getLeadingZeros();

    std::array<uint8_t, ID_BYTES> bytes {0};
};

} // namespace carrier
} // namespace elastos
