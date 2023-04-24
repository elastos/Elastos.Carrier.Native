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

#include "serializers.h"
#include "lookup_request.h"

namespace elastos {
namespace carrier {

int LookupRequest::getWant() const {
    return (want4 ? 0x01 : 0) | (want6 ? 0x02 : 0) | (wantToken ? 0x04 : 0);
}

void LookupRequest::setWant(int want) {
    want4 = (want & 0x01);
    want6 = (want & 0x02);
    wantToken = (want & 0x04);
}

void LookupRequest::serializeInternal(nlohmann::json& root) const {
    nlohmann::json json = {
        {Message::KEY_REQ_TARGET, target},
        {Message::KEY_REQ_WANT, getWant()}
    };
    _serialize(json);

    Message::serializeInternal(root);
    root[getKeyString()] = json;
}

void LookupRequest::parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName != Message::KEY_REQUEST || !object.is_object())
        throw std::invalid_argument("Invalid request message");

    for (const auto& [key, value]: object.items()) {
        if (key == Message::KEY_REQ_TARGET) {
            value.get_to(target);
        } else if(key == Message::KEY_REQ_WANT) {
            setWant(value.get<int>());
        } else {
            _parse(key, value);
        }
    }
}

#ifdef MSG_PRINT_DETAIL
void LookupRequest::toString(std::stringstream& ss) const {
    ss << "\n" << "Request: \n    Target: " << target << "\n    Want: "
        << (want4 ? "ipv4": "")
        << (want6 ? "ipv6": "")
        << (wantToken ? "token": "");
    _toString(ss);
}
#else
void LookupRequest::toString(std::stringstream& ss) const {
    ss << ",q:{t:" << target
        << ",w:" << std::to_string(getWant());
    _toString(ss);
    ss << "}";
}
#endif

}
}
