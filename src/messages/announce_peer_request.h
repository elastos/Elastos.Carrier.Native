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

#pragma once

#include "carrier/id.h"
#include "message.h"

namespace elastos {
namespace carrier {

class AnnouncePeerRequest : public Message {
public:
    AnnouncePeerRequest(const Id& _target, int _port, int _token)
        : Message(Message::Type::REQUEST, Message::Method::ANNOUNCE_PEER),
        target(_target), port(_port), token(_token) {}

    AnnouncePeerRequest()
        : Message(Message::Type::REQUEST, Message::Method::ANNOUNCE_PEER) {}

    const Id& getTarget() const {
        return target;
    }

    void setTarget(const Id& target) {
        this->target = target;
    }

    int getPort() const {
        return port;
    }

    void setPort(int port) {
        this->port = port;
    }

    int getToken() const {
        return token;
    }

    void setToken(int token) {
        this->token = token;
    }

    int estimateSize() const override {
        return Message::estimateSize() + 54; // + (name != null ? name.length() + 5 : 0);
    }

protected:
    void serializeInternal(nlohmann::json& root) const override;
    void parse(const std::string& fieldName, nlohmann::json& object) override;
    void toString(std::stringstream& ss) const override;

private:
    Id target;
    int port;
    int token;
};

}
}


