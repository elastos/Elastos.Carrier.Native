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

#include <list>
#include "message.h"
#include "dht.h"
#include "carrier/node_info.h"

namespace elastos {
namespace carrier {

class LookupResponse : public Message {
public:
    LookupResponse(Method method, int txid) : Message(Message::Type::RESPONSE, method, txid) {}

    void setNodes4(const std::list<Sp<NodeInfo>>& nodes4) {
        this->nodes4 = nodes4;
    }

    const std::list<Sp<NodeInfo>>& getNodes4() const {
        return nodes4;
    }

    void setNodes6(const std::list<Sp<NodeInfo>>& nodes6) {
        this->nodes6 = nodes6;
    }

    const std::list<Sp<NodeInfo>>& getNodes6() const {
        return nodes6;
    }

    const std::list<Sp<NodeInfo>>& getNodes(DHT::Type type) const;

    int getToken() const {
        return token;
    }

    void setToken(int token) {
        this->token = token;
    }

    int estimateSize() const override;

protected:
    virtual void _serialize(nlohmann::json& json) const {}
    virtual void _parse(const std::string& fieldName, nlohmann::json &object) {}
    virtual void _toString(std::stringstream& str) const {}

    void serializeInternal(nlohmann::json& root) const override;
    void parse(const std::string& fieldName, nlohmann::json& object) override;
    void toString(std::stringstream& str) const override;

private:
    void serializeNodes(nlohmann::json &object, const std::string& fieldName, const std::list<Sp<NodeInfo>>& nodes) const;
    void parseNodes(const nlohmann::json &object, std::list<Sp<NodeInfo>>& nodes);

    std::list<Sp<NodeInfo>> nodes4 {};
    std::list<Sp<NodeInfo>> nodes6 {};
    int token {0};
};

}
}
