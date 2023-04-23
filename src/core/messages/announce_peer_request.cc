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

#include <string>
#include <sstream>

#include "announce_peer_request.h"
#include "serializers.h"

namespace elastos {
namespace carrier {

void AnnouncePeerRequest::serializeInternal(nlohmann::json& root) const {
    nlohmann::json object = {
        {Message::KEY_REQ_TARGET, target},
        {Message::KEY_REQ_PORT, port},
        {Message::KEY_REQ_TOKEN, token}
    };
    Message::serializeInternal(root);
    root[getKeyString()] = object;
}

void AnnouncePeerRequest::parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName != Message::KEY_REQUEST || !object.is_object())
        throw std::invalid_argument("Invalid " + std::to_string((int)getMethod()) + "reqeust message");

    for (const auto& [key, value] : object.items()) {
        if (key == Message::KEY_REQ_TARGET)
            value.get_to(target);
        else if(key == Message::KEY_REQ_PORT)
            value.get_to(port);
        else if(key == Message::KEY_REQ_TOKEN)
            value.get_to(token);
        else
            throw std::invalid_argument("Invalid message with unkown key: " + key);
    }
}

void AnnouncePeerRequest::toString(std::stringstream& ss) const {
    ss << ",q:{"
        << "t:" << target
        << ",p:" << std::to_string(port)
        << ",tok:" << std::to_string(token)
        << "}";
}
}
}
