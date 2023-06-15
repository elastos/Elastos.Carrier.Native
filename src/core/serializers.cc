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

#include "serializers.h"

namespace elastos {
namespace carrier {

    void from_json(const nlohmann::json& json, SocketAddress& sa) {
        auto s = json.get<std::string>();
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

    void to_json(nlohmann::json& json, const NodeInfo& ni) {
        auto addr = nlohmann::json::binary_t {
            std::vector<uint8_t>(ni.getAddress().inaddr(), ni.getAddress().inaddr() + ni.getAddress().inaddrLength())
        };

        json = nlohmann::json::array();
        json.push_back(ni.getId());
        json.push_back(addr);
        json.push_back(ni.getAddress().port());
    }

    void from_json(const nlohmann::json& json, NodeInfo& ni) {
        auto id = json[0].get_binary();
        auto ip = json[1].get_binary();
        auto port = json[2].get<int>();

        ni = NodeInfo(id, ip, port);
    }

    void to_json(nlohmann::json& json, const PeerInfo& pi) {
        json = nlohmann::json::array();
        json.push_back(pi.getNodeId());
        json.push_back(pi.getProxyId());
        json.push_back(pi.getPort());
        json.push_back(pi.getAlt());
        json.push_back(nlohmann::json::binary_t(pi.getSignature()));
    }

    void from_json(const nlohmann::json& json, PeerInfo& pi) {
        auto id = json[0].get_binary();
        auto proxyId = json[1].get_binary();
        auto port = json[2].get<uint16_t>();
        auto alt = json[3].get<std::string>();
        auto sig = json[4].get_binary();

        pi = PeerInfo(id, proxyId, port, alt, sig);
    }
}
}
