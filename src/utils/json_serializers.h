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

#include "carrier/socket_address.h"

namespace elastos {
namespace carrier {
    void to_json(nlohmann::json& j, const SocketAddress& sa) {
        j = sa.toString();
    }

    void from_json(const nlohmann::json& j, SocketAddress& sa) {
        auto s = j.get<std::string>();
        int port = 0;

        int colon = s.find(":");
        if (colon < s.length()) {
            port = stoi(s.substr(colon + 1));
        } else {
            colon = s.length() - 1;
        }

        auto a = s.substr(0, colon);
        sa = SocketAddress(a, port);
    }

/*
    void from_json(const nlohmann::json& object, Value& value) {
        if (object.contains(VALUE_PUBLICKEY)) {
            value.publicKey = object.at(VALUE_PUBLICKEY).get<Id>();
            value.sequenceNumber = object.at(VALUE_SEQ).get<uint16_t>();
            auto bin = object.at(VALUE_NONCE).get_binary();
            std::memcpy((void*)value.nonce.bytes(), bin.data(), value.nonce.size());

            if (object.contains(VALUE_RECIPIENT)) {
                value.recipient = object.at(VALUE_RECIPIENT).get<Id>();
            }

            value.signature = object.at(VALUE_SIGNATURE).get_binary();
        }

        auto _data = object.at(VALUE_DATA).get_binary();
        value.data = _data;
    }

    void to_json(nlohmann::json& object, const Value& value) {
        if (value.isMutable()) {
            if (value.isEncrypted())
                object[VALUE_RECIPIENT] = value.recipient;

            object[VALUE_SIGNATURE] = nlohmann::json::binary_t {value.signature};
            object[VALUE_PUBLICKEY] = value.publicKey;
            object[VALUE_NONCE] = nlohmann::json::binary_t {{value.nonce.cbegin(), value.nonce.cend()}};
            object[VALUE_SEQ] = value.sequenceNumber;
        }

        object[VALUE_DATA] = nlohmann::json::binary_t {value.data};
    }
*/
}
}
