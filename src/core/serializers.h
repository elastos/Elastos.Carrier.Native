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
#include "carrier/node_info.h"
#include "carrier/peer_info.h"
#include "carrier/id.h"

namespace elastos {
namespace carrier {

    inline void to_json(nlohmann::json& json, const SocketAddress& sa) {
        json = sa.toString();
    }
    void from_json(const nlohmann::json& json, SocketAddress& sa);

    void to_json(nlohmann::json& json, const NodeInfo& ni);
    void from_json(const nlohmann::json& json, NodeInfo& ni);

    inline void to_json(nlohmann::json& json, const Id& id) {
        json = nlohmann::json::binary_t {std::vector<uint8_t>(id.data(), id.data() + id.size())};
    }

    inline void from_json(const nlohmann::json& json, Id& id) {
        id = Id(json.get_binary());
    }

    inline void from_json(const nlohmann::json& json, std::optional<Id>& id) {
        id = Id(json.get_binary());
    }

    void to_json(nlohmann::json& json, const PeerInfo& pi);
    void from_json(const nlohmann::json& json, PeerInfo& pi);

}
}
