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
#include <memory>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#include "def.h"
#include "blob.h"

namespace elastos {
namespace carrier {

/**
 * @brief 签名类，包含签名所需要的密钥对
 *
 */
class CARRIER_PUBLIC Signature {
public:
    class KeyPair;
    /**
     * @brief 用于签名的私钥
     *
     */
    class PrivateKey {
    public:
        /**
         * @brief 裸私钥的长度
         *
         */
        static const uint32_t BYTES { 64 };
        /**
         * @brief 创建一个PrivateKey
         *
         */
        PrivateKey() noexcept {}
        /**
         * @brief 创建一个PrivateKey
         *
         * @param sk PrivateKey的裸私钥
         */
        PrivateKey(const Blob& sk);
        /**
         * @brief 复制拷贝一个PrivateKey
         *
         * @param o 另一个PrivateKey
         */
        PrivateKey(const PrivateKey& o) noexcept :
                PrivateKey(o.blob()) {}
        /**
         * @brief 复制拷贝一个PrivateKey
         *
         * @param o 另一个PrivateKey
         */
        PrivateKey(PrivateKey&& o) noexcept :
                PrivateKey(o.blob()) {
            o.clear();
        }
        /**
         * @brief 销毁PrivateKey
         *
         */
        ~PrivateKey() noexcept {
            clear();
        }
        /**
         * @brief 判断PrivateKey是否是有效PrivateKey（是否有裸私钥来判定）
         *
         * @return true PrivateKey是有效PrivateKey（有裸私钥）
         * @return false PrivateKey是无效PrivateKey（无裸私钥）
         */
        explicit operator bool() const noexcept {
            return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
        }
        /**
         * @brief 判断两个PrivateKey是否相等
         *
         * @param o 另一个PrivateKey
         * @return true 两者相等
         * @return false 两者不等
         */
        bool operator==(const PrivateKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) == 0;
        }
        /**
         * @brief 判断两个PrivateKey是否不等
         *
         * @param o 另一个PrivateKey
         * @return true 两者不等
         * @return false 两者相等
         */
        bool operator!=(const PrivateKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) != 0;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个PrivateKey
         * @return PrivateKey& 返回赋值结果
         */
        PrivateKey& operator=(const PrivateKey& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            return *this;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个PrivateKey
         * @return PrivateKey& 返回赋值结果
         */
        PrivateKey& operator=(PrivateKey&& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            o.clear();
            return *this;
        }
        /**
         * @brief 获取PrivateKey头指针
         *
         * @return const uint8_t* 返回裸私钥的头指针
         */
        const uint8_t* cbegin() const noexcept {
            return key.data();
        }
        /**
         * @brief 获取PrivateKey的尾指针
         *
         * @return const uint8_t* 返回裸私钥的尾指针
         */
        const uint8_t* cend() const noexcept {
            return key.data() + key.size();
        }
        /**
         * @brief 获取PrivateKey的裸私钥
         *
         * @return const uint8_t* 返回裸私钥包指针
         */
        const uint8_t* bytes() const noexcept {
            return key.data();
        }
        /**
         * @brief PrivateKey大小
         *
         * @return size_t 返回大小
         */
        size_t size() const noexcept {
            return BYTES;
        }
        /**
         * @brief 获取PrivateKey的裸私钥二进制包
         *
         * @return const Blob 返回二进制包
         */
        const Blob blob() const noexcept {
            return key;
        }
        /**
         * @brief 清空PrivateKey
         *
         */
        void clear() noexcept {
            key.fill(0);
        }
        /**
         * @brief 用PrivateKey签名数据
         *
         * @param sig 签名后的数据
         * @param data 原始数据
         */
        void sign(Blob& sig, const Blob& data) const;
        /**
         * @brief 用PrivateKey签名数据
         *
         * @param data 原始数据
         * @return std::vector<uint8_t> 返回签名后的数据
         */
        std::vector<uint8_t> sign(const Blob& data) const {
            std::vector<uint8_t> sig(Signature::BYTES);
            Blob _sig{ sig };
            sign(_sig, data);
            return sig;
        }
        /**
         * @brief 获取可读PrivateKey信息
         *
         * @return std::string 返回字符串信息
         */
        operator std::string() const noexcept;

        friend class KeyPair;

    private:
        std::array<uint8_t, BYTES> key { 0 };
    };

    /**
     * @brief 用于验签的公钥
     *
     */
    class PublicKey {
    public:
        /**
         * @brief 裸公钥长度
         *
         */
        static const uint32_t BYTES { 32 };
        /**
         * @brief 创建一个新的PublicKey
         *
         */
        PublicKey() noexcept {}
        /**
         * @brief 创建一个新的PublicKey
         *
         * @param pk 裸公钥二进制数据
         */
        PublicKey(const Blob& pk);
        /**
         * @brief 复制拷贝一个PublicKey
         *
         * @param o 另一个PublicKey
         */
        PublicKey(const PublicKey& o) noexcept :
                PublicKey(o.blob()) {}
        /**
         * @brief 复制拷贝一个PublicKey
         *
         * @param o 另一个PublicKey
         */
        PublicKey(PublicKey&& o) noexcept :
                PublicKey(o.blob()) {
            o.clear();
        }
        /**
         * @brief 销毁PublicKey
         *
         */
        ~PublicKey() noexcept {
            clear();
        }
        /**
         * @brief 判断PublicKey是否是有效PublicKey（有裸公钥）
         *
         * @return true PublicKey是有效PublicKey（有裸公钥）
         * @return false PublicKey是无效PublicKey（无裸公钥）
         */
        explicit operator bool() const noexcept {
            return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
        }
        /**
         * @brief 判断两个PublicKey是否相等
         *
         * @param o 另一个PublicKey
         * @return true 两个PublicKey相等
         * @return false 两个PublicKey不相等
         */
        bool operator==(const PublicKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) == 0;
        }
        /**
         * @brief 判断两个PublicKey是否不相等
         *
         * @param o 另一个PublicKey
         * @return true 两个PublicKey不相等
         * @return false 两个PublicKey相等
         */
        bool operator!=(const PublicKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) != 0;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个PublicKey
         * @return PublicKey& 返回赋值结果
         */
        PublicKey& operator=(const PublicKey& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            return *this;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个PublicKey
         * @return PublicKey& 返回赋值结果
         */
        PublicKey& operator=(PublicKey&& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            o.clear();
            return *this;
        }
        /**
         * @brief PublicKey头指针
         *
         * @return const uint8_t* 返回头指针
         */
        const uint8_t* cbegin() const noexcept {
            return key.data();
        }
        /**
         * @brief PublicKey尾指针
         *
         * @return const uint8_t* 返回尾指针
         */
        const uint8_t* cend() const noexcept {
            return key.data() + key.size();
        }
        /**
         * @brief 获取PublicKey数据指针
         *
         * @return const uint8_t* 返回裸公钥数据指针
         */
        const uint8_t* bytes() const noexcept {
            return key.data();
        }
        /**
         * @brief 获取PublicKey大小
         *
         * @return size_t 返回大小
         */
        size_t size() const noexcept {
            return BYTES;
        }
        /**
         * @brief 获取PublicKey数据包
         *
         * @return const Blob 返回二进制数据包
         */
        const Blob blob() const noexcept {
            return key;
        }
        /**
         * @brief 清空PublicKey
         *
         */
        void clear() noexcept {
            key.fill(0);
        }
        /**
         * @brief 验证签名过的数据
         *
         * @param sig 签名过的数据
         * @param data 验签后得到的原始数据
         * @return true 验签成功
         * @return false 验签失败
         */
        bool verify(const Blob& sig, const Blob& data) const;
        /**
         * @brief 获取可读PublicKey信息
         *
         * @return std::string 返回字符串信息
         */
        operator std::string() const noexcept;

        friend class KeyPair;

    private:
        std::array<uint8_t, BYTES> key { 0 };
    };

    class KeyPair {
    public:
        /**
         * @brief 生成KeyPair的种子大小
         *
         */
        static const uint32_t SEED_BYTES { 32 };
        /**
         * @brief 创建一个新KeyPair
         *
         */
        KeyPair() noexcept;
        /**
         * @brief 创建一个新KeyPair
         *
         * @param sk 生成密钥对需要的私钥
         */
        KeyPair(PrivateKey& sk) noexcept;
        /**
         * @brief 创建一个新KeyPair
         *
         * @param sk 生成密钥对需要的私钥
         */
        KeyPair(PrivateKey&& sk) noexcept;
        /**
         * @brief 创建一个新KeyPair
         *
         * @param sk 生成密钥对需要的私钥
         */
        KeyPair(const Blob& sk);
        /**
         * @brief 复制构造一个KeyPair
         *
         * @param o 另一个KeyPair
         */
        KeyPair(KeyPair& o) noexcept :
                sk(o.sk), pk(o.pk) {}
        /**
         * @brief 复制构造一个KeyPair
         *
         * @param o 另一个KeyPair
         */
        KeyPair(KeyPair&& o) noexcept :
                sk(std::move(o.sk)), pk(std::move(o.pk)) {}
        /**
         * @brief 由seed生成KeyPair
         *
         * @param seed 种子
         * @return KeyPair 返回KeyPair对象
         */
        static KeyPair fromSeed(const Blob& seed);
        /**
         * @brief 判断KeyPair是否是有效KeyPair（是否含有裸私钥）
         *
         * @return true KeyPair是有效KeyPair（有裸私钥）
         * @return false KeyPair是无效KeyPair（无裸私钥）
         */
        explicit operator bool() const noexcept {
            return static_cast<bool>(sk);
        }
        /**
         * @brief 判断两个KeyPair是否相等
         *
         * @param o 另一个KeyPair
         * @return true 两个KeyPair相等
         * @return false 两个KeyPair不等
         */
        bool operator==(const KeyPair& o) const noexcept {
            return sk == o.sk;
        }
        /**
         * @brief 判断两个KeyPair是否不相等
         *
         * @param o 另一个KeyPair
         * @return true 两个KeyPair不相等
         * @return false 两个KeyPair相等
         */
        bool operator!=(const KeyPair& o) const noexcept {
            return sk != o.sk;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个KeyPair
         * @return KeyPair& 返回赋值结果
         */
        KeyPair& operator=(const KeyPair& o) noexcept {
            sk = o.sk;
            pk = o.pk;
            return *this;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个KeyPair
         * @return KeyPair& 返回赋值结果
         */
        KeyPair& operator=(KeyPair&& o) noexcept {
            sk = o.sk;
            pk = o.pk;
            o.clear();
            return *this;
        }
        /**
         * @brief 获取KeyPair中的私钥
         *
         * @return const PrivateKey& 返回私钥
         */
        const PrivateKey& privateKey() const noexcept {
            return sk;
        }
        /**
         * @brief 获取KeyPair中的公钥
         *
         * @return const PublicKey& 返回公钥
         */
        const PublicKey& publicKey() const noexcept {
            return pk;
        }
        /**
         * @brief 清空KeyPair
         */
        void clear() noexcept {
            sk.clear();
            pk.clear();
        }

    private:
        PrivateKey sk;
        PublicKey pk;
    };
    /**
     * @brief 签名数据包大小
     *
     */
    static const uint32_t BYTES { 64 };
    /**
     * @brief 创建一个新的Signature
     *
     */
    Signature() noexcept {
        reset();
    }
    /**
     * @brief 复制拷贝一个Signature
     *
     * @param o 另一个Signature
     */
    Signature(const Signature& o) noexcept {
        std::memcpy(state.__opaque__, o.state.__opaque__, sizeof(state.__opaque__));
    }
    /**
     * @brief 复制拷贝一个Signature
     *
     * @param o 另一个Signature
     */
    Signature(Signature&& o) noexcept {
        std::memcpy(state.__opaque__, o.state.__opaque__, sizeof(state.__opaque__));
        std::memset(o.state.__opaque__, 0, sizeof(o.state.__opaque__));
    }
    /**
     * @brief 销毁Signature对象
     *
     */
    ~Signature() noexcept {}
    /**
     * @brief 判断两个Signature是否相等
     *
     * @param o 另一个Signature
     * @return true 两个Signature相等
     * @return false 两个Signature不相等
     */
    bool operator==(const Signature& o) const noexcept {
        return std::memcmp(o.state.__opaque__, state.__opaque__, sizeof(state.__opaque__)) == 0;
    }
    /**
     * @brief 判断两个Signature是否不相等
     *
     * @param o 另一个Signature
     * @return true 两个Signature不相等
     * @return false 两个Signature相等
     */
    bool operator!=(const Signature& o) const noexcept {
        return std::memcmp(o.state.__opaque__, state.__opaque__, sizeof(state.__opaque__)) != 0;
    }
    /**
     * @brief 赋值
     *
     * @param o 另一个Signature
     * @return Signature& 返回赋值结果
     */
    Signature& operator=(const Signature& o) noexcept {
        std::memcpy(state.__opaque__, o.state.__opaque__, sizeof(state.__opaque__));
        return *this;
    }
    /**
     * @brief 赋值
     *
     * @param o 另一个Signature
     * @return Signature& 返回赋值结果
     */
    Signature& operator=(Signature&& o) noexcept {
        std::memcpy(state.__opaque__, o.state.__opaque__, sizeof(state.__opaque__));
        std::memset(o.state.__opaque__, 0, sizeof(o.state.__opaque__));
        return *this;
    }
    /**
     * @brief 重置Signature
     *
     */
    void reset();
    /**
     * @brief 追加Signature内容
     *
     * @param part 待追加数据
     */
    void update(const Blob& part);
    /**
     * @brief 签名
     *
     * @param sig 签名后得到的数据
     * @param sk 用来签名的私钥
     */
    void sign(Blob& sig, const PrivateKey& sk) const;
    /**
     * @brief 签名
     *
     * @param sk 用来签名的私钥
     * @return std::vector<uint8_t> 返回签名后得到的数据包
     */
    std::vector<uint8_t> sign(const PrivateKey& sk) const {
        std::vector<uint8_t> sig(BYTES);
        Blob _sig{sig};
        sign(_sig, sk);
        return sig;
    }
    /**
     * @brief 验签
     *
     * @param sig 签名数据
     * @param pk 用来验签的公钥
     * @return true 验签成功
     * @return false 验签失败
     */
    bool verify(const Blob& sig, const PublicKey& pk) const;

private:
    struct SignState { uint8_t __opaque__[256]; };
    SignState state;
};

/**
 * @brief 加密盒子。如果应用程序向相同的接收者发送多条消息，或者从相同的发送者接收多条消息，
 *        则可以通过只计算一次对称密钥并在后续操作中重用它来提高性能。
 *        CryptoBox在构造之时根据节点私钥和另一节点公钥产生对称密钥，用于后续的加解密。
 */
class CryptoBox {
public:
    class KeyPair;
    /**
     * @brief 用于加密的私钥
     *
     */
    class PrivateKey {
    public:
        /**
         * @brief 私钥大小
         *
         */
        static const uint32_t BYTES { 32 };
        /**
         * @brief 创建一个空PrivateKey
         *
         */
        PrivateKey() noexcept {}
        /**
         * @brief 创建一个PrivateKey
         *
         * @param sk 私钥的裸数据
         */
        PrivateKey(const Blob& sk);
        /**
         * @brief 复制拷贝一个PrivateKey
         *
         * @param o 另一个PrivateKey
         */
        PrivateKey(PrivateKey& o) noexcept :
                PrivateKey(o.blob()) {}
        /**
         * @brief 复制拷贝一个PrivateKey
         *
         * @param o 另一个PrivateKey
         */
        PrivateKey(PrivateKey&& o) noexcept :
                PrivateKey(o.blob()) {
            o.clear();
        }
        /**
         * @brief 从签名私钥中获取加密私钥
         *
         * @param signSk 签名（Signature)私钥
         * @return PrivateKey 返回加密私钥
         */
        static PrivateKey fromSignatureKey(const Signature::PrivateKey& signSk);
        /**
         * @brief 销毁PrivateKey
         *
         */
        ~PrivateKey() noexcept {
            clear();
        }
        /**
         * @brief 判断PrivateKey是否为有效PrivateKey（是否有裸私钥）
         *
         * @return true PrivateKey为有效Privatekey（有裸私钥）
         * @return false PrivateKey为无效Privatekey（无裸私钥）
         */
        explicit operator bool() const noexcept {
            return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
        }
        /**
         * @brief 判断两个PrivateKey是否相等
         *
         * @param o 另一个PrivateKey
         * @return true 两个PrivateKey相等
         * @return false 两个PrivateKey不相等
         */
        bool operator==(const PrivateKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) == 0;
        }
        /**
         * @brief 判断两个PrivateKey是否不相等
         *
         * @param o 另一个PrivateKey
         * @return true 两个PrivateKey不相等
         * @return false 两个PrivateKey相等
         */
        bool operator!=(const PrivateKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) != 0;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个PrivateKey
         * @return PrivateKey& 返回赋值结果
         */
        PrivateKey& operator=(const PrivateKey& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            return *this;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个PrivateKey
         * @return PrivateKey& 返回赋值结果
         */
        PrivateKey& operator=(PrivateKey&& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            o.clear();
            return *this;
        }
        /**
         * @brief 获取PrivateKey的头指针
         *
         * @return const uint8_t* 返回头指针
         */
        const uint8_t* cbegin() const noexcept {
            return key.data();
        }
        /**
         * @brief 获取PrivateKey的尾指针
         *
         * @return const uint8_t* 返回尾指针
         */
        const uint8_t* cend() const noexcept {
            return key.data() + key.size();
        }
        /**
         * @brief 获取PrivateKey的裸私钥数据
         *
         * @return const uint8_t* 裸私钥数据指针
         */
        const uint8_t* bytes() const noexcept {
            return key.data();
        }
        /**
         * @brief 获取PrivateKey的大小
         *
         * @return size_t 返回大小
         */
        size_t size() const noexcept {
            return BYTES;
        }
        /**
         * @brief 获取PrivateKey的数据包
         *
         * @return const Blob 返回数据包
         */
        const Blob blob() const noexcept {
            return key;
        }
        /**
         * @brief 清空PrivateKey
         *
         */
        void clear() noexcept {
            key.fill(0);
        }
        /**
         * @brief 获取可读的PrivateKey信息
         *
         * @return std::string 返回信息字符串
         */
        operator std::string() const noexcept;

        friend class KeyPair;

    private:
        std::array<uint8_t, BYTES> key { 0 };
    };
    /**
     * @brief 用于解密的公钥
     *
     */
    class PublicKey {
    public:
        /**
         * @brief 裸公钥大小
         *
         */
        static const uint32_t BYTES { 32 };
        /**
         * @brief 创建一个空PublicKey
         *
         */
        PublicKey() noexcept {}
        /**
         * @brief 创建一个新PublicKey
         *
         * @param pk 裸公钥数据
         */
        PublicKey(const Blob& pk);
        /**
         * @brief 复制拷贝一个PublicKey
         *
         * @param o 另一个PublicKey
         */
        PublicKey(PublicKey& o) noexcept :
                PublicKey(o.blob()) {}
        /**
         * @brief 复制拷贝一个PublicKey
         *
         * @param o 另一个PublicKey
         */
        PublicKey(PublicKey&& o) noexcept :
                PublicKey(o.blob()) {
            o.clear();
        }
        /**
         * @brief 从验签公钥获取解密公钥
         *
         * @param signPk 验签公钥
         * @return PublicKey 返回解密公钥
         */
        static PublicKey fromSignatureKey(const Signature::PublicKey& signPk);
        /**
         * @brief 销毁PublicKey
         *
         */
        ~PublicKey() noexcept {
            clear();
        }
        /**
         * @brief 判断PublicKey是否是有效PublicKey（有裸公钥）
         *
         * @return true PublicKey是有效PublicKey（有裸公钥）
         * @return false PublicKey是无效PublicKey（无裸公钥）
         */
        explicit operator bool() const noexcept {
            return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
        }
        /**
         * @brief 判断两个PublicKey是否相等
         *
         * @param o 另一个PublicKey
         * @return true 两个PublicKey相等
         * @return false 两个PublicKey不相等
         */
        bool operator==(const PublicKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) == 0;
        }
        /**
         * @brief 判断两个PublicKey是否不相等
         *
         * @param o 另一个PublicKey
         * @return true 两个PublicKey不相等
         * @return false 两个PublicKey相等
         */
        bool operator!=(const PublicKey& o) const noexcept {
            return std::memcmp(key.data(), o.key.data(), BYTES) != 0;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个PublicKey
         * @return PublicKey& 返回赋值结果
         */
        PublicKey& operator=(const PublicKey& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            return *this;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个PublicKey
         * @return PublicKey& 返回赋值结果
         */
        PublicKey& operator=(PublicKey&& o) noexcept {
            std::memcpy(key.data(), o.key.data(), BYTES);
            o.clear();
            return *this;
        }
        /**
         * @brief 获取PublicKey头指针
         *
         * @return const uint8_t* 返回头指针
         */
        const uint8_t* cbegin() const noexcept {
            return key.data();
        }
        /**
         * @brief 获取PublicKey尾指针
         *
         * @return const uint8_t* 返回尾指针
         */
        const uint8_t* cend() const noexcept {
            return key.data() + key.size();
        }
        /**
         * @brief 获取PublicKey的裸公钥数据指针
         *
         * @return const uint8_t* 返回数据指针
         */
        const uint8_t* bytes() const noexcept {
            return key.data();
        }
        /**
         * @brief 获取PublicKey大小
         *
         * @return size_t 返回大小
         */
        size_t size() const noexcept {
            return BYTES;
        }
        /**
         * @brief 获取PublicKey数据包
         *
         * @return const Blob 返回裸公钥二进制数据包
         */
        const Blob blob() const noexcept {
            return key;
        }
        /**
         * @brief 清空PublicKey，即清空裸公钥数据
         *
         */
        void clear() noexcept {
            key.fill(0);
        }
        /**
         * @brief 获取可读的PublicKey信息
         *
         * @return std::string 返回信息字符串
         */
        operator std::string() const noexcept;

        friend class KeyPair;

    private:
        std::array<uint8_t, BYTES> key { 0 };
    };

    class CARRIER_PUBLIC Nonce {
    public:
        /**
         * @brief nonce数据大小
         *
         */
        static const uint32_t BYTES { 24 };
        /**
         * @brief 创建一个空Nonce
         *
         */
        Nonce() noexcept {};
        /**
         * @brief 创建一个新Nonce
         *
         * @param n 用于填充Nonce中nonce的原始数据
         */
        Nonce(const Blob& n);
        /**
         * @brief 复制拷贝一个新Nonce
         *
         * @param o 另一个Nonce
         */
        Nonce(const Nonce& o) noexcept :
                Nonce(o.blob()) {}
        /**
         * @brief 复制拷贝一个新Nonce
         *
         * @param o 另一个Nonce
         */
        Nonce(Nonce&& o) noexcept :
                Nonce(o.blob()) {
            o.clear();
        }
        /**
         * @brief 销毁Nonce
         *
         */
        ~Nonce() noexcept {
            clear();
        }
        /**
         * @brief 判断Nonce是否有效（根据Nonce中nonce是否为空）
         *
         * @return true Nonce有效（nonce数据不为空）
         * @return false Nonce无效（nonce数据为空）
         */
        explicit operator bool() const noexcept {
            return !std::all_of(nonce.cbegin(), nonce.cend(), [](uint8_t i){ return !i; });
        }
        /**
         * @brief 判断两个Nonce是否相等（判断两者的nonce数据是否相等）
         *
         * @param o 另一个Nonce
         * @return true 两个Nonce相等
         * @return false 两个Nonce不相等
         */
        bool operator==(const Nonce& o) const noexcept {
            return std::memcmp(nonce.data(), o.nonce.data(), BYTES) == 0;
        }
        /**
         * @brief 判断两个Nonce是否不相等（判断两者的nonce数据是否不相等）
         *
         * @param o 另一个Nonce
         * @return true 两个Nonce不相等
         * @return false 两个Nonce相等
         */
        bool operator!=(const Nonce& o) const noexcept {
            return std::memcmp(nonce.data(), o.nonce.data(), BYTES) != 0;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个Nonce
         * @return Nonce& 返回赋值结果
         */
        Nonce& operator=(const Nonce& o) noexcept {
            std::memcpy(nonce.data(), o.nonce.data(), BYTES);
            return *this;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个Nonce
         * @return Nonce& 返回赋值结果
         */
        Nonce& operator=(Nonce&& o) noexcept {
            std::memcpy(nonce.data(), o.nonce.data(), BYTES);
            o.clear();
            return *this;
        }
        /**
         * @brief 获取Nonce头指针
         *
         * @return const uint8_t* 返回头指针
         */
        const uint8_t* cbegin() const noexcept {
            return nonce.data();
        }
        /**
         * @brief 获取Nonce尾指针
         *
         * @return const uint8_t* 返回尾指针
         */
        const uint8_t* cend() const noexcept {
            return nonce.data() + nonce.size();
        }
        /**
         * @brief 获取Nonce中nonce数据包指针
         *
         * @return const uint8_t* 返回数据包指针
         */
        const uint8_t* bytes() const noexcept {
            return nonce.data();
        }
        /**
         * @brief 获取Nonce大小
         *
         * @return size_t 返回大小
         */
        size_t size() const noexcept {
            return BYTES;
        }
        /**
         * @brief 获取Nonce的nonce数据包
         *
         * @return const Blob 返回nonce数据包
         */
        const Blob blob() const noexcept {
            return nonce;
        }
        /**
         * @brief 清空Nonce
         *
         */
        void clear() noexcept {
            nonce.fill(0);
        }
        /**
         * @brief 对Nonce中nonce数据加1
         *
         * @return Nonce& 返回加1之后对Nonce对象
         */
        Nonce& increment() noexcept;
        /**
         * @brief 随机生成Nonce对象
         *
         * @return Nonce& 返回Nonce对象
         */
        Nonce& random() noexcept;
        /**
         * @brief 获取可读的Nonce信息
         *
         * @return std::string 返回信息字符串
         */
        operator std::string() const noexcept;

    private:
        std::array<uint8_t, BYTES> nonce {};
    };

    class KeyPair {
    public:
        /**
         * @brief 加密算法中的种子大小
         *
         */
        static const uint32_t SEED_BYTES { 32 };
        /**
         * @brief 创建一个空KeyPair
         */
        KeyPair() noexcept;
        /**
         * @brief 创建一个新KeyPair
         *
         * @param sk 用于生成KeyPair的裸私钥
         */
        KeyPair(PrivateKey& sk) noexcept;
        /**
         * @brief 创建一个新KeyPair
         *
         * @param sk 用于生成KeyPair的裸私钥
         */
        KeyPair(PrivateKey&& sk) noexcept;
        /**
         * @brief 创建一个新KeyPair
         *
         * @param sk 用于生成KeyPair的裸私钥
         */
        KeyPair(const Blob& sk);
        /**
         * @brief 复制拷贝一个新KeyPair
         *
         * @param o 另一个KeyPair
         */
        KeyPair(KeyPair& o) noexcept :
                sk(o.sk), pk(o.pk) {}
        /**
         * @brief 复制拷贝一个新KeyPair
         *
         * @param o 另一个KeyPair
         */
        KeyPair(KeyPair&& o) noexcept :
                sk(std::move(o.sk)), pk(std::move(o.pk)) {}
        /**
         * @brief 根据种子来创建KeyPair。seed生成的裸私钥来填充KeyPair
         *
         * @param seed 加密算法中的种子
         * @return KeyPair 返回KeyPair对象
         */
        static KeyPair fromSeed(const Blob& seed);
        /**
         * @brief 从签名密钥对中衍生得到加密密钥对
         *
         * @param signKeyPair 签名密钥对
         * @return KeyPair 返回加密密钥对
         */
        static KeyPair fromSignatureKeyPair(const Signature::KeyPair& signKeyPair);
        /**
         * @brief 判断KeyPair是否为有效（是否有裸私钥数据）
         *
         * @return true KeyPair有效（有裸私钥数据）
         * @return false KeyPair无效（无裸私钥数据）
         */
        explicit operator bool() const noexcept {
            return static_cast<bool>(sk);
        }
        /**
         * @brief 判断两个KeyPair是否相等，根据两者中保存的私钥来判定
         *
         * @param o 另一个KeyPair
         * @return true 两个KeyPair相等
         * @return false 两个KeyPair不相等
         */
        bool operator==(const KeyPair& o) const noexcept {
            return sk == o.sk;
        }
        /**
         * @brief 判断两个KeyPair是否不相等，根据两者中保存的私钥来判定
         *
         * @param o 另一个KeyPair
         * @return true 两个KeyPair不相等
         * @return false 两个KeyPair相等
         */
        bool operator!=(const KeyPair& o) const noexcept {
            return sk != o.sk;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个KeyPair
         * @return KeyPair& 返回赋值结果
         */
        KeyPair& operator=(const KeyPair& o) noexcept {
            sk = o.sk;
            pk = o.pk;
            return *this;
        }
        /**
         * @brief 赋值
         *
         * @param o 另一个KeyPair
         * @return KeyPair& 返回赋值结果
         */
        KeyPair& operator=(KeyPair&& o) noexcept {
            sk = o.sk;
            pk = o.pk;
            o.clear();
            return *this;
        }
        /**
         * @brief 获取KeyPair中私钥
         *
         * @return const PrivateKey& 返回私钥
         */
        const PrivateKey& privateKey() const noexcept {
            return sk;
        }
        /**
         * @brief 获取KeyPair中公钥
         *
         * @return const PublicKey& 返回公钥对象
         */
        const PublicKey& publicKey() const noexcept {
            return pk;
        }
        /**
         * @brief 清空KeyPair记录的密钥内容
         *
         */
        void clear() noexcept {
            sk.clear();
            pk.clear();
        }

    private:
        PrivateKey sk;
        PublicKey pk;
    };
    /**
     * @brief 对称密钥大小。
     *
     */
    static const uint32_t SYMMETRIC_KEY_BYTES { 32 };
    /**
     * @brief 加解密算法中Mac的大小
     *
     */
    static const uint32_t MAC_BYTES { 16 };
    /**
     * @brief 创建一个空CryptoBox
     *
     */
    CryptoBox() noexcept {};
    /**
     * @brief 创建一个新CryptoBox，通过发送者私钥和接受者公钥产生对称密钥。
     *
     * @param pk 接受者公钥
     * @param sk 发送着私钥
     */
    CryptoBox(const PublicKey& pk, const PrivateKey& sk);
    /**
     * @brief 复制拷贝一个新CryptoBox
     *
     * @param o 另一个CryptoBox
     */
    CryptoBox(const CryptoBox& o) noexcept :
            key(o.key) {}
    /**
     * @brief 复制拷贝一个新CryptoBox
     *
     * @param o 另一个CryptoBox
     */
    CryptoBox(CryptoBox&& o) noexcept :
            key(o.key) {
        o.clear();
    }
    /**
     * @brief 销毁CryptoBox
     *
     */
    ~CryptoBox() noexcept {
        clear();
    }
    /**
     * @brief 判断CryptoBox是否为有效（是否有对称密钥数据）
     *
     * @return true CryptoBox有效（有对称密钥数据）
     * @return false CryptoBox无效（无对称密钥数据）
     */
    explicit operator bool() const noexcept {
        return !std::all_of(key.cbegin(), key.cend(), [](uint8_t i){ return !i; });
    }
    /**
     * @brief 判断两个CryptoBox是否相等（根据两者的对称密钥是否相同来判定）
     *
     * @param o 另一个CryptoBox
     * @return true 两个CryptoBox相等
     * @return false 两个CryptoBox不相等
     */
    bool operator==(const CryptoBox& o) const noexcept {
        return std::memcmp(key.data(), o.key.data(), SYMMETRIC_KEY_BYTES) == 0;
    }
    /**
     * @brief 判断两个CryptoBox是否不相等（根据两者的对称密钥是否相同来判定）
     *
     * @param o 另一个CryptoBox
     * @return true 两个CryptoBox不相等
     * @return false 两个CryptoBox相等
     */
    bool operator!=(const CryptoBox& o) const noexcept {
        return std::memcmp(key.data(), o.key.data(), SYMMETRIC_KEY_BYTES) != 0;
    }
    /**
     * @brief 赋值
     *
     * @param o 另一个CryptoBox
     * @return CryptoBox& 返回赋值结果
     */
    CryptoBox& operator=(const CryptoBox& o) noexcept {
        std::memcpy(key.data(), o.key.data(), SYMMETRIC_KEY_BYTES);
        return *this;
    }
    /**
     * @brief 赋值
     *
     * @param o 另一个CryptoBox
     * @return CryptoBox& 返回赋值结果
     */
    CryptoBox& operator=(CryptoBox&& o) noexcept {
        std::memcpy( key.data(), o.key.data(), SYMMETRIC_KEY_BYTES);
        o.clear();
        return *this;
    }
    /**
     * @brief 获取CryptoBox的头指针
     *
     * @return const uint8_t* 返回头指针
     */
    const uint8_t* cbegin() const noexcept {
        return key.data();
    }
    /**
     * @brief 获取CryptoBox的尾指针
     *
     * @return const uint8_t* 返回尾指针
     */
    const uint8_t* cend() const noexcept {
        return key.data() + key.size();
    }
    /**
     * @brief 获取CryptoBox中对称密钥数据指针
     *
     * @return const uint8_t* 返回数据指针
     */
    const uint8_t* bytes() const noexcept {
        return key.data();
    }
    /**
     * @brief 获取CryptoBox中对称密钥大小
     *
     * @return size_t 返回密钥大小
     */
    size_t size() const noexcept {
        return SYMMETRIC_KEY_BYTES;
    }
    /**
     * @brief 获取CryptoBox中对称密钥数据包
     *
     * @return const Blob 返回二进制数据包
     */
    const Blob blob() const noexcept {
        return key;
    }
    /**
     * @brief 清空CryptoBox
     *
     */
    void clear() noexcept {
        key.fill(0);
    }
    /**
     * @brief 用CryptoBox加密数据
     *
     * @param cipher 得到的加密数据包
     * @param plain 待加密的原始数据
     * @param nonce nonce数据
     */
    void encrypt(Blob& cipher, const Blob& plain, const Nonce& nonce) const;
    /**
     * @brief 用CryptoBox加密数据
     *
     * @param plain 待加密的原始数据
     * @param nonce nonce数据
     * @return std::vector<uint8_t> 返回加密后的数据
     */
    std::vector<uint8_t> encrypt(const Blob& plain, const Nonce& nonce) const {
        std::vector<uint8_t> cipher(plain.size() + MAC_BYTES);
        Blob _cipher{cipher};
        encrypt(_cipher, plain, nonce);
        return cipher;
    }
    /**
     * @brief 用CryptoBox加密数据
     *
     * @param cipher 加密后的数据
     * @param plain 原始数据
     * @param nonce nonce数据
     * @param pk 接受者的公钥
     * @param sk 发送者的私钥
     */
    static void encrypt(Blob& cipher, const Blob& plain, const Nonce& nonce,
            const PublicKey& pk, const PrivateKey& sk);
    /**
     * @brief 用CryptoBox加密数据
     *
     * @param plain 原始数据
     * @param nonce nonce数据
     * @param pk 接受者的公钥
     * @param sk 发送者的私钥
     * @return std::vector<uint8_t> 返回加密数据
     */
    static std::vector<uint8_t> encrypt(const Blob& plain, const Nonce& nonce,
            const PublicKey& pk, const PrivateKey& sk) {
        std::vector<uint8_t> cipher(plain.size() + MAC_BYTES);
        Blob _cipher{cipher};
        encrypt(_cipher, plain, nonce, pk, sk);
        return cipher;
    }
    /**
     * @brief 用CryptoBox解密数据
     *
     * @param plain 解密后得到的原始数据
     * @param cipher 待解密的加密数据
     * @param nonce nonce数据
     */
    void decrypt(Blob& plain, const Blob& cipher, const Nonce& nonce) const;
    /**
     * @brief 用CryptoBox解密数据
     *
     * @param cipher 待解密的加密数据
     * @param nonce nonce数据
     * @return std::vector<uint8_t> 返回解密后的原始数据
     */
    std::vector<uint8_t> decrypt(const Blob& cipher, const Nonce &nonce) const {
        std::vector<uint8_t> plain(cipher.size() - MAC_BYTES);
        Blob _plain{plain};
        decrypt(_plain, cipher, nonce);
        return plain;
    }
    /**
     * @brief 用CryptoBox解密数据
     *
     * @param plain 解密后的原始数据
     * @param cipher 待解密的加密数据
     * @param nonce nonce数据
     * @param pk 发送者公钥
     * @param sk 接受者私钥
     */
    static void decrypt(Blob& plain, const Blob& cipher, const Nonce& nonce,
            const PublicKey& pk, const PrivateKey& sk);
    /**
     * @brief 用CryptoBox解密数据
     *
     * @param cipher 待解密的加密数据
     * @param nonce nonce数据
     * @param pk 发送者公钥
     * @param sk 接受者私钥
     * @return std::vector<uint8_t> 返回解密后的原始数据
     */
    static std::vector<uint8_t> decrypt(const Blob& cipher, const Nonce &nonce,
            const PublicKey& pk, const PrivateKey& sk) {
        std::vector<uint8_t> plain(cipher.size() - MAC_BYTES);
        Blob _plain{plain};
        decrypt(_plain, cipher, nonce, pk, sk);
        return plain;
    }

private:
    std::array<uint8_t, SYMMETRIC_KEY_BYTES> key {};
};

/**
 * @brief 用于处理sha256算法
 *
 */
class SHA256 {
public:
    /**
     * @brief SHA256大小
     *
     */
    static const uint32_t BYTES { 32 };
    /**
     * @brief 创建一个空SHA256
     *
     */
    SHA256() noexcept {
        reset();
    }
    /**
     * @brief 重设SHA256
     *
     */
    void reset();
    /**
     * @brief 用指定的数据更新SHA256
     *
     * @param part 用来更新SHA256的数据
     */
    void update(const Blob& part);
    /**
     * @brief 结束SHA256，并将摘要放在hash包里
     *
     * @param hash 存放hash数据
     */
    void digest(Blob& hash);
    /**
     * @brief 结束SHA256
     *
     * @return std::vector<uint8_t> 返回hash数据
     */
    std::vector<uint8_t> digest() {
        std::vector<uint8_t> hash(BYTES);
        Blob _hash{hash};
        digest(_hash);
        return hash;
    }
    /**
     * @brief 结束SHA256
     *
     * @param hash 存放hash数据
     * @param data 用来更新SHA256的最后数据
     */
    static void digest(Blob& hash, const Blob& data);
    /**
     * @brief 结束SHA256
     *
     * @param data 用来更新SHA256的最后数据
     * @return std::vector<uint8_t> 返回hash数据
     */
    static std::vector<uint8_t> digest(const Blob& data) {
        std::vector<uint8_t> hash(BYTES);
        Blob _hash{hash};
        digest(_hash, data);
        return hash;
    }

private:
    struct DigestState { uint8_t __opaque__[128]; };
    DigestState state;
};

class Random {
public:
    // [0, upbound)
    /**
     * @brief 获取介于0到0xff的8bits随机数
     *
     * @return uint8_t 8bits随机数
     */
    static uint8_t uint8();
    /**
     * @brief 获取介于0到upbound的8bits随机数
     *
     * @param upbound 上限数
     * @return uint8_t 8bits随机数
     */
    static uint8_t uint8(uint8_t upbound);
    /**
     * @brief 获取介于0到0xffff的16bits随机数
     *
     * @return uint16_t 16bits随机数
     */
    static uint16_t uint16();
    /**
     * @brief 获取介于0到upbound的16bits随机数
     *
     * @param upbound 上限数
     * @return uint16_t 16bits随机数
     */
    static uint16_t uint16(uint16_t upbound);
    /**
     * @brief 获取介于0到0xffffffff的32bits随机数
     *
     * @return uint32_t 32bits随机数
     */
    static uint32_t uint32();
    /**
     * @brief 获取介于0到upbound的32bits随机数
     *
     * @param upbound 上限数
     * @return uint32_t 32bits随机数
     */
    static uint32_t uint32(uint32_t upbound);
    /**
     * @brief 获取介于0到0xffffffffffffffff的64bits随机数
     *
     * @return uint64_t 64bits随机数
     */
    static uint64_t uint64();
    /**
     * @brief 获取介于0到upbound的64bits随机数
     *
     * @param upbound 上限数
     * @return uint64_t 64bits随机数
     */
    static uint64_t uint64(uint64_t upbound);
    /**
     * @brief 获取定长的二进制随机数
     *
     * @param buf 二进制数据空间
     * @param length 指定的随机数大小
     */
    static void buffer(void* buf, size_t length);
};

class CryptoError : public std::runtime_error
{
public:
    explicit CryptoError(const std::string& what) : runtime_error(what) {}
    explicit CryptoError(const char* what) : runtime_error(what) {}

    CryptoError(const CryptoError&) noexcept = default;
    virtual ~CryptoError() noexcept = default;
};

}
}
