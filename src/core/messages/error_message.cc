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

#include <sstream>

#include "error_message.h"

namespace elastos {
namespace carrier {

void ErrorMessage::serializeInternal(nlohmann::json& root) const {
    nlohmann::json object = {
        {Message::KEY_ERR_CODE, code},
        {Message::KEY_ERR_MESSAGE, message}
    };
    Message::serializeInternal(root);
    root[getKeyString()] = object;
}

void ErrorMessage::parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName != Message::KEY_ERROR || !object.is_object())
        throw std::invalid_argument("Invalid request message");

    for (const auto& [key, value]: object.items()) {
        if (key == Message::KEY_ERR_CODE) {
            value.get_to(code);
        } else if(key == Message::KEY_ERR_MESSAGE) {
            value.get_to(message);
        } else {
            throw std::invalid_argument("Invalid " + getMethodString() + " request message");
        }
    }
}

#ifdef MSG_PRINT_DETAIL
void ErrorMessage::toString(std::stringstream& ss) const {
    ss << "\nError:\n    Code:"
        << std::to_string(code)
        << "\nMessage: '" << message
        << "'}";
}
#else
void ErrorMessage::toString(std::stringstream& ss) const {
    ss << ",e:{c:"
        << std::to_string(code)
        << ".m:'" << message
        << "'}";
}
#endif

}
}
