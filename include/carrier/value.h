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

#include <vector>
#include <memory>

#include "def.h"
#include "types.h"
#include "id.h"
#include "socket_address.h"
#include "crypto_core.h"

namespace elastos {
namespace carrier {

/**
 * @brief 记录Value信息
 *
 */
class CARRIER_PUBLIC Value {
public:
    /**
     * @brief 创建一个新的空Value
     *
     */
    Value() = default;
    /**
     * @brief Value的复制拷贝函数
     *
     */
    Value(const Value&) = delete;
    /**
     * @brief Value的复制拷贝函数
     *
     */
    Value(Value&&) = delete;
    /**
     * @brief 重载Value的operator=
     *
     * @return Value& 被复制的Value对象
     */
    Value& operator=(const Value&) = delete;
    /**
     * @brief 计算指定Value的Id
     *
     * @param value Value对象
     * @return Id 返回Id对象
     */
    static Id calculateId(const Value& value);
    /**
     * @brief 获取Value的Id
     *
     * @return Id 返回Id对象
     */
    Id getId() const {
        return Value::calculateId(*this);
    }
    /**
     * @brief 获取生成Id的公钥
     *
     * @return const Id& 返回Id对象
     */
    const Id& getPublicKey() const noexcept {
        return publicKey;
    }
    /**
     * @brief 获取Value的接受节点
     *
     * @return const Id& 返回接受节点Id
     */
    const Id& getRecipient() const noexcept {
        return recipient;
    }
    /**
     * @brief 判断Value是否含有私钥
     *
     * @return true Value含有私钥
     * @return false Value不含有私钥
     */
    bool hasPrivateKey() const noexcept {
        return static_cast<bool>(privateKey);
    }
    /**
     * @brief 获取Value的序列号
     *
     * @return int 返回序列号
     */
    int getSequenceNumber() const noexcept {
        return sequenceNumber;
    }
    /**
     * @brief 获取Value的Nonce对象
     *
     * @return const CryptoBox::Nonce& 返回Nonce对象
     */
    const CryptoBox::Nonce& getNonce() const  noexcept{
        return nonce;
    }
    /**
     * @brief 获取Value的签名内容
     *
     * @return const std::vector<uint8_t>& 返回签名数据包
     */
    const std::vector<uint8_t>& getSignature() const noexcept {
        return signature;
    }
    /**
     * @brief 获取Value的数据包
     *
     * @return const std::vector<uint8_t>& 返回二进制数据包
     */
    const std::vector<uint8_t>& getData() const noexcept {
        return data;
    }
    /**
     * @brief 获取Value的大小（数据大小加上签名数据大小）
     *
     * @return size_t 返回Value大小
     */
    size_t size() const noexcept {
        return data.size() + signature.size();
    }
    /**
     * @brief 判断Value是否经过加密
     *
     * @return true Value经过加密
     * @return false Value未经过加密
     */
    bool isEncrypted() const noexcept {
        return static_cast<bool>(recipient);
    }
    /**
     * @brief 判断Value是否经过签名
     *
     * @return true Value经过签名
     * @return false Value未经过签名
     */
    bool isSigned() const noexcept {
        return !signature.empty();
    }
    /**
     * @brief 判断Value是否可更改数据内容
     *
     * @return true Value可被更改数据
     * @return false Value不可被更改数据
     */
    bool isMutable() const noexcept {
        return static_cast<bool>(publicKey);
    }
    /**
     * @brief 判断Value是否是有效Value
     *
     * @return true Value有效
     * @return false Value无效（如签名无效，加解密无效等）
     */
    bool isValid() const;
    /**
     * @brief 重载Value的operator==
     *
     * @param other 被比较的Value对象
     * @return true 当前Value和被比较的Value的相等
     * @return false 当前Value和被比较的Value的不等
     */
    bool operator== (const Value& other) const;
    /**
     * @brief 获取可读的Value的信息
     *
     * @return std::string 返回可读的信息字符串
     */
    operator std::string() const;

private: // internal methods used in friend class.
    friend class SqliteStorage;
    friend class FindValueResponse;
    friend class StoreValueRequest;
    friend class Node;

    // internal setts used in SqliteStorage type.
    void setPrivateKey(const Blob& val) noexcept {
        this->privateKey = Signature::PrivateKey(val);
    }
    void setPublicKey(const Blob& val) {
        this->publicKey = Id(val);
    }
    void setRecipient(const Blob& val) {
        this->recipient = Id(val);
    }
    void setSignature(const Blob& val) {
        signature.resize(val.size());
        std::memcpy(signature.data(), val.ptr(), val.size());
    }
    void setNonce(const Blob& val) {
        this->nonce = CryptoBox::Nonce(val);
    }
    void setData(const Blob& val) {
        data.resize(val.size());
        std::memcpy(data.data(), val.ptr(), val.size());
    }

    void setSequenceNumber(int seqNumber) {
        this->sequenceNumber = seqNumber;
    }

    const Signature::PrivateKey& getPrivateKey() const {
        return privateKey;
    }

    void purgePrivateKey() {
        privateKey.clear();
    }

    static Sp<Value> create(const std::vector<uint8_t>& data);
    static Sp<Value> createSigned(const std::vector<uint8_t>& data);
    static Sp<Value> createEncrypted(const Id& to, const std::vector<uint8_t>& data);
    static Sp<Value> updateValue(const Sp<Value> oldValue, const std::vector<uint8_t>& newData);

    void createSignature();
    bool verifySignature() const;

    Id publicKey {};
    Id recipient {};
    Signature::PrivateKey privateKey {};
    CryptoBox::Nonce nonce {};
    std::vector<std::uint8_t> signature {};
    std::vector<std::uint8_t> data {};
    int32_t sequenceNumber {0};
};
}
}
