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
#include <nlohmann/json.hpp>
#include <map>

#include "constants.h"
#include "carrier/socket_address.h"
#include "carrier/id.h"
#include "carrier/version.h"

// #define MSG_PRINT_DETAIL 1

namespace elastos {
namespace carrier {

class RPCCall;
class RPCServer;

class Message {
public:
    static const std::string KEY_REQ_SID;
    static const std::string KEY_REQ_NODES4;
    static const std::string KEY_REQ_NODES6;
    static const std::string KEY_REQ_EXPIRED;
    static const std::string KEY_REQ_REFRESHED;

    static const std::string KEY_TYPE;
    static const std::string KEY_ID;
    static const std::string KEY_TXID ;
    static const std::string KEY_VERSION;

    static const std::string KEY_REQUEST;
    static const std::string KEY_REQ_TARGET;
    static const std::string KEY_REQ_WANT;
    static const std::string KEY_REQ_PORT;
    static const std::string KEY_REQ_TOKEN;
    static const std::string KEY_REQ_PUBLICKEY;
    static const std::string KEY_REQ_RECIPIENT;
    static const std::string KEY_REQ_NONCE;
    static const std::string KEY_REQ_SIGNATURE;
    static const std::string KEY_REQ_VALUE;
    static const std::string KEY_REQ_CAS;
    static const std::string KEY_REQ_SEQ;
    static const std::string KEY_REQ_PROXY_ID;
    static const std::string KEY_REQ_ALT;

    static const std::string KEY_RESPONSE;
    static const std::string KEY_RES_NODES4;
    static const std::string KEY_RES_NODES6;
    static const std::string KEY_RES_TOKEN;
    // static const std::string KEY_RES_PEERS_ID {"pid"};
    static const std::string KEY_RES_PEERS4;
    static const std::string KEY_RES_PEERS6;
    static const std::string KEY_RES_PUBLICKEY;
    static const std::string KEY_RES_RECIPIENT;
    static const std::string KEY_RES_NONCE;
    static const std::string KEY_RES_SIGNATURE;
    static const std::string KEY_RES_VALUE;
    static const std::string KEY_RES_SEQ;

    static const std::string KEY_ERROR;
    static const std::string KEY_ERR_CODE;
    static const std::string KEY_ERR_MESSAGE;

    static const int MSG_VERSION = 0x01;

    static const int BASE_SIZE = 56;

public:
    enum class Method {
        UNKNOWN     = 0x00,
        PING        = 0x01,
        FIND_NODE   = 0x02,
        ANNOUNCE_PEER   = 0x03,
        FIND_PEER   = 0x04,
        STORE_VALUE = 0x05,
        FIND_VALUE  = 0x06
    };

    enum class Type {
        ERR         = 0x00,
        REQUEST     = 0x20,
        RESPONSE    = 0x40
    };

    Method getMethod() const {
        return ofMethod(type);
    }

    Type getType() const {
        return ofType(type);
    }

    const std::string& getMethodString() const;
    const std::string& getTypeString() const;
    const std::string& getKeyString() const;

    void setId(const Id& id) noexcept {
        this->id = id;
    }

    const Id& getId() const noexcept {
        return id;
    }

    void setTxid(int txid) noexcept {
        this->txid = txid;
    }

    int getTxid() const noexcept {
        return txid;
    }

    void setVersion(int version) noexcept {
        this->version = version;
    }

    int getVersion() const noexcept {
        return version;
    }

    const SocketAddress& getOrigin() const noexcept {
        return origin;
    }

    void setOrigin(const SocketAddress& origin) noexcept {
        this->origin = origin;
    }

    const SocketAddress& getRemoteAddress() noexcept {
        return remoteAddr;
    }

    const Id& getRemoteId() noexcept {
        return remoteId;
    }

    void setRemote(const Id& id, const SocketAddress& address) noexcept {
        this->remoteId = id;
        this->remoteAddr = address;
    }

    std::string getReadableVersion() const noexcept {
        return Version::toString(version);
    }

    void setAssociatedCall(RPCCall* call) noexcept {
        associatedCall = call;
    }

    RPCCall* getAssociatedCall() const noexcept {
        return associatedCall;
    }

    static Sp<Message> parse(const uint8_t* buf, size_t buflen);

    operator std::string() const;
    std::vector<std::uint8_t> serialize() const;

    virtual int estimateSize() const {
        return BASE_SIZE;
    }

#ifdef MSG_PRINT_DETAIL
    std::string name {};

    void setName(const std::string& name) {
        this->name = name;
    }
    const std::string& getName() const {
        return name;
    }
#endif

protected:
    explicit Message(Type _type, Method _method, int _txid = 0)
        : type((int)_type | (int)_method), txid(_txid), version(0) {}

    explicit Message(const Message&) = delete;

    virtual void parse(const std::string& fieldName, nlohmann::json& object) {}
    virtual void toString(std::stringstream& ss) const {}
    virtual void serializeInternal(nlohmann::json& root) const;

private:
    static Sp<Message> createMessage(int type);
    static Type ofType(int messageType);
    static Method ofMethod(int messageType);

    static const int MSG_TYPE_MASK;
    static const int MSG_METHOD_MASK;

    SocketAddress origin;
    SocketAddress remoteAddr;

    Id id;
    Id remoteId;

    RPCCall* associatedCall = nullptr;

    int type {0};
    int txid {0};
    int version {0};
};

} // namespace carrier
} // namespace elastos
