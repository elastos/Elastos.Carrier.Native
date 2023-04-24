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

#include "lookup_response.h"
#include "serializers.h"

namespace elastos {
namespace carrier {

const std::list<Sp<NodeInfo>>& LookupResponse::getNodes(DHT::Type type) const {
    switch (type) {
    case DHT::Type::IPV4:
        return getNodes4();
    case DHT::Type::IPV6:
        return getNodes6();
    default:
        throw std::invalid_argument("Invalid DHT type");
    }
}

int LookupResponse::estimateSize() const {
    const int node4Size = 44;
    const int node6Size = 56;
    const int tokenSize = 9;
    int size = Message::estimateSize() + 4;

    if (!nodes4.empty())
        size += (5 + node4Size * nodes4.size());
    if (!nodes6.empty())
        size += (5 + node6Size * nodes6.size());

    size += (token == 0) ? 0 : tokenSize;
    return size;
}

void LookupResponse::serializeInternal(nlohmann::json& root) const {
    nlohmann::json object = nlohmann::json::object();
    for (const auto& pair: {
        std::make_pair(std::ref(KEY_RES_NODES4), std::ref(nodes4)),
        std::make_pair(std::ref(KEY_RES_NODES6), std::ref(nodes6))
    }) {
        if (!pair.second.empty())
            serializeNodes(object, pair.first, pair.second);
    }

    if (token != 0)
        object[KEY_RES_TOKEN] = token;

    _serialize(object);
    Message::serializeInternal(root);
    root[getKeyString()] = object;
}

void LookupResponse::parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName != KEY_RESPONSE || !object.is_object())
        throw std::invalid_argument("Invalid lookup response message");

    for (const auto& [key, value] : object.items()) {
        if (key == KEY_RES_NODES4) {
            parseNodes(value, nodes4);
        } else if (key == KEY_RES_NODES6) {
            parseNodes(value, nodes6);
        } else if (key == KEY_RES_TOKEN) {
            value.get_to(token);
        } else {
            _parse(key, value);
        }
    }
}

void LookupResponse::serializeNodes(nlohmann::json& object, const std::string& fieldName, const std::list<Sp<NodeInfo>>& nodes) const {
    nlohmann::json jsonNodes;
    for (const auto& node: nodes) {
        jsonNodes.push_back(*node);
    }

    object[fieldName] = jsonNodes;
}

void LookupResponse::parseNodes(const nlohmann::json &object, std::list<Sp<NodeInfo>>& nodes) {
    if (!object.is_array())
        throw std::invalid_argument("Invalid response nodes message");

    for (const auto& elem : object) {
        nodes.emplace_back(std::make_shared<NodeInfo>(elem.get<NodeInfo>()));
    }
}

#ifdef MSG_PRINT_DETAIL
void LookupResponse::toString(std::stringstream& ss) const {
    ss << "\nResponse: ";

    if (!nodes4.empty()) {
        ss << "\n    n4: \n";
        for (const auto& node : nodes4) {
            ss << "      " << *node << "\n";
        }
    }
    if (!nodes6.empty()) {
        ss << "\n    n6: \n";
        for (const auto& node : nodes4) {
            ss << "      " << *node << "\n";
        }
    }

    if (token != 0)
        ss << "\nToken: " << std::to_string(token);

    _toString(ss);
}
#else
void LookupResponse::toString(std::stringstream& ss) const {
    ss << ",r:{";

    bool first_node4 {true};
    if (!nodes4.empty()) {
        ss << "n4:";
        for (const auto& node : nodes4) {
            ss << (first_node4 ? "": ",") << "[" << *node << "]";
            first_node4 = false;
        }
    }
    if (!nodes6.empty()) {
        ss << (first_node4 ? "": ",") << "n6:";
        bool first_node6 {true};
        for (const auto& node : nodes6) {
            ss << (first_node6 ? "": ",") << "[" << *node << "]";
            first_node6 = false;
        }
    }

    if (token != 0)
        ss << ",tok:" << std::to_string(token);

    _toString(ss);
    ss << "}";
}
#endif

}
}
