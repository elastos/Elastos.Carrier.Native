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

#include "message.h"

namespace elastos {
namespace carrier {

class LookupRequest : public Message {
public:
    LookupRequest(Method _method, const Id& _target)
        : target(_target), Message(Message::Type::REQUEST, _method) {}

    LookupRequest(Method _method)
        : Message(Message::Type::REQUEST, _method) {}

    const Id& getTarget() const {
        return target;
    }

    void setWant4(bool want4) {
        this->want4 = want4;
    }

    bool doesWant4() const {
        return want4;
    }

    void setWant6(bool want6) {
        this->want6 = want6;
    }

    bool doesWant6() const {
        return want6;
    }

    int estimateSize() const override {
        return Message::estimateSize() + 43;
    }

protected:
    virtual void setWantToken(bool wantToken) {
        this->wantToken = wantToken;
    }

    virtual bool doesWantToken() const {
        return wantToken;
    }

    int getWant() const;
    void setWant(int want);

    virtual void _serialize(nlohmann::json& root) const {}
    void serializeInternal(nlohmann::json& root) const override;
    virtual void _parse(const std::string& fieldName, nlohmann::json &object) {}
    void parse(const std::string& fieldName, nlohmann::json &object) override;

    virtual void _toString(std::stringstream& ss) const {}
    void toString(std::stringstream &ss) const override;

private:
    Id target {};
    bool want4 {false};
    bool want6 {false};
    bool wantToken {false};
};

}
}
