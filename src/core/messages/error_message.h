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

#include <nlohmann/json.hpp>
#include "message.h"

namespace elastos {
namespace carrier {

class ErrorMessage : public Message {
public:
    ErrorMessage(Method method, int txid, int _code, const std::string& _message)
        : Message(Message::Type::ERROR, method, txid), code(_code), message(_message) {}

    ErrorMessage(Method method): Message(Message::Type::ERROR, method) {}

    ~ErrorMessage() = default;

    int getCode() const {
        return code;
    }

    const std::string& getMessage() const {
        return message;
    }

    int estimateSize() const override {
        return Message::estimateSize() + 16 + message.size();
    }

protected:
    void serializeInternal(nlohmann::json& root) const override;
    void parse(const std::string& fieldName, nlohmann::json& object) override;
    void toString(std::stringstream& ss) const override;

private:
    std::string message {};
    int code {0};
};

} // namespace carrier
} // namespace elastos
