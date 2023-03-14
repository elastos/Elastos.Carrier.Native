/*
 * Copyright (c) 2022 - 2023 Elastos Foundation
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

#include "message.h"
#include "ping_request.h"
#include "ping_response.h"
#include "error_message.h"
#include "find_node_request.h"
#include "announce_peer_request.h"
#include "find_peer_request.h"
#include "store_value_request.h"
#include "find_value_request.h"
#include "announce_peer_response.h"
#include "find_node_response.h"
#include "find_peer_response.h"
#include "find_value_response.h"
#include "store_value_response.h"

namespace elastos {
namespace carrier {

const std::string Message::KEY_REQ_SID        = "sid";
const std::string Message::KEY_REQ_NODES4     = "n4";
const std::string Message::KEY_REQ_NODES6     = "n6";
const std::string Message::KEY_REQ_EXPIRED    = "exp";
const std::string Message::KEY_REQ_REFRESHED  = "re";

const std::string Message::KEY_TYPE           = "y";
const std::string Message::KEY_ID             = "i";
const std::string Message::KEY_TXID           = "t";
const std::string Message::KEY_VERSION        = "v";

const std::string Message::KEY_REQUEST        = "q";
const std::string Message::KEY_REQ_TARGET     = "t";
const std::string Message::KEY_REQ_WANT       = "w";
const std::string Message::KEY_REQ_PORT       = "p";
const std::string Message::KEY_REQ_TOKEN      = "tok";
const std::string Message::KEY_REQ_PUBLICKEY  = "k";
const std::string Message::KEY_REQ_RECIPIENT  = "rec";
const std::string Message::KEY_REQ_NONCE      = "n";
const std::string Message::KEY_REQ_SIGNATURE  = "sig";
const std::string Message::KEY_REQ_VALUE      = "v";
const std::string Message::KEY_REQ_CAS        = "cas";
const std::string Message::KEY_REQ_SEQ        = "seq";

const std::string Message::KEY_RESPONSE       = "r";
const std::string Message::KEY_RES_NODES4     = "n4";
const std::string Message::KEY_RES_NODES6     = "n6";
const std::string Message::KEY_RES_TOKEN      = "tok";
// const std::string Message::KEY_RES_PEERS_ID    = "pid";
const std::string Message::KEY_RES_PEERS4     = "p4";
const std::string Message::KEY_RES_PEERS6     = "p6";
const std::string Message::KEY_RES_PUBLICKEY  = "k";
const std::string Message::KEY_RES_RECIPIENT  = "rec";
const std::string Message::KEY_RES_NONCE      = "n";
const std::string Message::KEY_RES_SIGNATURE  = "sig";
const std::string Message::KEY_RES_VALUE      = "v";
const std::string Message::KEY_RES_SEQ        = "seq";

const std::string Message::KEY_ERROR          = "e";
const std::string Message::KEY_ERR_CODE       = "c";
const std::string Message::KEY_ERR_MESSAGE    = "m";

const int Message::MSG_TYPE_MASK = 0xE0;
const int Message::MSG_METHOD_MASK = 0x1F;

Message::Type Message::ofType(int messageType) {
    static const std::unordered_map<int, Type> typeMap {
        {0x20, Type::REQUEST},
        {0x40, Type::RESPONSE},
        {0x00, Type::ERROR}
    };

    int type = messageType & MSG_TYPE_MASK;
    auto it = typeMap.find(type);
    if (it == typeMap.end())
        throw std::invalid_argument("Invalid Type: " + std::to_string(type));

    return it->second;
}

Message::Method Message::ofMethod(int messageType) {
    static const std::unordered_map<int, Method> methodMap {
        {0x01, Method::PING},
        {0x02, Method::FIND_NODE},
        {0x03, Method::ANNOUNCE_PEER},
        {0x04, Method::FIND_PEER},
        {0x05, Method::STORE_VALUE},
        {0x06, Method::FIND_VALUE},
        {0x00, Method::UNKNOWN}
    };

    int method = messageType & MSG_METHOD_MASK;
    auto it = methodMap.find(method);
    if (it == methodMap.end())
        throw std::invalid_argument("Invalid method: " + std::to_string(method));

    return it->second;
}

const std::string& Message::getKeyString() const {
    static std::unordered_map<Message::Type, std::string> keyMap = {
        { Type::REQUEST, KEY_REQUEST },
        { Type::RESPONSE, KEY_RESPONSE },
        { Type::ERROR, KEY_ERROR }
    };
    return keyMap[getType()];
}

const std::string& Message::getTypeString() const {
    static std::unordered_map<Message::Type, std::string> nameMap = {
#ifdef MSG_PRINT_DETAIL
        { Type::REQUEST, "request" },
        { Type::RESPONSE, "response" },
        { Type::ERROR, "error" }
#else
        { Type::REQUEST, KEY_REQUEST },
        { Type::RESPONSE, KEY_RESPONSE },
        { Type::ERROR, KEY_ERROR }
#endif
    };
    return nameMap[getType()];
}

const std::string& Message::getMethodString() const {
    static std::unordered_map<Message::Method, std::string> nameMap = {
        { Method::UNKNOWN, "unknown" },
        { Method::PING, "ping" },
        { Method::FIND_NODE, "find_node" },
        { Method::ANNOUNCE_PEER, "announce_peer" },
        { Method::FIND_PEER, "find_peer" },
        { Method::STORE_VALUE, "store_value" },
        { Method::FIND_VALUE, "find_value" }
    };
    return nameMap[getMethod()];
}

Sp<Message> Message::parse(const uint8_t* buf, size_t buflen) {
    auto root = nlohmann::json::from_cbor(std::vector<uint8_t>{buf, buf + buflen});
    if (!root.is_object())
        throw std::runtime_error("Invalid message: not a CBOR object");

    auto type = root.find(KEY_TYPE);
    if (type == root.end()) {
        throw std::runtime_error("Invalid message: missing type field");
    }

    auto message = Message::createMessage(type->get<uint8_t>());
    for (const auto& [key, value]: root.items()) {
        if (key == KEY_TXID) {
            value.get_to(message->txid);
        } else if (key == KEY_VERSION) {
            value.get_to(message->version);
        } else if (key == KEY_REQUEST || key == KEY_RESPONSE || key == KEY_ERROR) {
            message->parse(key, value);
        }
    }
    return message;
}

Sp<Message> Message::createMessage(int messageType) {
    static const std::map<Method, std::function<Sp<Message>()>> reqFactory = {
        { Method::PING, []{ return std::make_shared<PingRequest>(); }},
        { Method::FIND_NODE, []{ return std::make_shared<FindNodeRequest>(); }},
        { Method::ANNOUNCE_PEER, []{ return std::make_shared<AnnouncePeerRequest>(); }},
        { Method::FIND_PEER, []{ return std::make_shared<FindPeerRequest>(); }},
        { Method::STORE_VALUE, []{ return std::make_shared<StoreValueRequest>(); }},
        { Method::FIND_VALUE, []{ return std::make_shared<FindValueRequest>(); }}
    };
    static const std::map<Method, std::function<Sp<Message>()>> rspFactory = {
        { Method::PING, []{ return std::make_shared<PingResponse>(); }},
        { Method::FIND_NODE, []{ return std::make_shared<FindNodeResponse>(); }},
        { Method::ANNOUNCE_PEER, []{ return std::make_shared<AnnouncePeerResponse>(); }},
        { Method::FIND_PEER, []{ return std::make_shared<FindPeerResponse>(); }},
        { Method::STORE_VALUE, []{ return std::make_shared<StoreValueResponse>(); }},
        { Method::FIND_VALUE, []{ return std::make_shared<FindValueResponse>(); }}
    };

    auto type = ofType(messageType);
    auto method = ofMethod(messageType);

    switch (type) {
    case Type::REQUEST: {
        auto reqCreator = reqFactory.find(method);
        if (reqCreator == reqFactory.end())
            throw std::invalid_argument("Invalid request method: " + std::to_string(static_cast<int>(method)));
        return reqCreator->second();
    }
    case Type::RESPONSE: {
        auto rspCreator = rspFactory.find(method);
        if (rspCreator == rspFactory.end())
            throw std::invalid_argument("Invalid response method: " + std::to_string(static_cast<int>(method)));
        return rspCreator->second();
    }
    case Type::ERROR: {
        return std::make_shared<ErrorMessage>(method);
    }
    default: {
        throw std::invalid_argument("INTERNAL ERROR: should never happen.");
    }
    }
}

#ifdef MSG_PRINT_DETAIL
Message::operator std::string() const {
    std::stringstream ss;
    if (!name.empty())
        ss << "Name: " << name << "\n";

    ss << "Type: " << getTypeString()
        << "\nMethod: " << getMethodString()
        << "\nTxid: " << std::to_string(txid);

    toString(ss);

    if (version != 0)
        ss << "\nVersion: " << getReadableVersion();

    return ss.str();
}
#else
Message::operator std::string() const {
    std::stringstream ss;
    ss.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    ss.str().reserve(1500);

    ss << "y:" << getTypeString()
        << ",m:" << getMethodString()
        << ",t:" << std::to_string(txid);
    toString(ss);
    if (version != 0)
        ss << ",v:" << getReadableVersion();
    return ss.str();
}
#endif

void Message::serializeInternal(nlohmann::json& root) const {
    root[KEY_TYPE] = type;
    root[KEY_TXID] = txid;
    root[KEY_VERSION] = version;
}

std::vector<std::uint8_t> Message::serialize() const {
    nlohmann::json root = nlohmann::json::object();
    serializeInternal(root);
    return nlohmann::json::to_cbor(root);
}

}
}


