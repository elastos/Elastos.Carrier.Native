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

#include "carrier/id.h"
#include "message.h"

namespace elastos {
namespace carrier {

class AnnouncePeerRequest : public Message {
public:
    AnnouncePeerRequest(const Id& _target, const Id& _proxyId, int _port,
        const std::string _alt,  const std::vector<uint8_t>& sig, int _token)
        : Message(Message::Type::REQUEST, Message::Method::ANNOUNCE_PEER),
        peerId(_target), proxyId(_proxyId), port(_port), alt(_alt), signature(sig), token(_token) {
            if (proxyId != Id::zero()) {
                proxied = true;
            }
        }

    AnnouncePeerRequest()
        : Message(Message::Type::REQUEST, Message::Method::ANNOUNCE_PEER) {}

    const Id& getTarget() const {
        return peerId;
    }

    void setTarget(const Id& target) {
        this->peerId = target;
    }

    uint16_t getPort() const {
        return port;
    }

    void setPort(uint16_t port) {
        this->port = port;
    }

    int getToken() const {
        return token;
    }

    void setToken(int token) {
        this->token = token;
    }

    const Id& getProxyId() const {
        return proxyId;
    }

    const std::string& getAlt() const {
        return alt;
    }

    const std::vector<uint8_t>& getSignature() const {
        return signature;
    }

    int estimateSize() const override {
        auto size = Message::estimateSize() + peerId.size() + sizeof(port) + alt.size() + signature.size() + sizeof(token);
        if (proxied) {
            size += proxyId.size();
        }
        return size;
    }

protected:
    void serializeInternal(nlohmann::json& root) const override;
    void parse(const std::string& fieldName, nlohmann::json& object) override;
    void toString(std::stringstream& ss) const override;

private:
    Id peerId;
    Id proxyId;
    uint16_t port;
    int token;
    std::string alt {};
    std::vector<uint8_t> signature {};

    bool proxied {false};
};

}
}


