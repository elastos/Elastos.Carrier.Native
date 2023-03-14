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

#include "carrier/peer_info.h"
#include "carrier/socket_address.h"

namespace elastos {
namespace carrier {

PeerInfo::operator std::string() const {
    std::stringstream ss;
    ss.str().reserve(80);
    ss << "<" << nodeId.toBase58String()
        << "," << sockaddr.host()
        << "," << std::to_string(sockaddr.port())
        << ">";

    return ss.str();
}

std::ostream& operator<< (std::ostream& os, const PeerInfo& pi) {
    os << static_cast<std::string>(pi);
    return os;
}

void to_json(nlohmann::json& json, const PeerInfo& pi) {
    auto addr = nlohmann::json::binary_t {
        std::vector<std::uint8_t>(pi.sockaddr.inaddr(), pi.sockaddr.inaddr() + pi.sockaddr.inaddrLength())
    };

    json = nlohmann::json::array();
    json.push_back(pi.nodeId);
    json.push_back(addr);
    json.push_back(pi.sockaddr.port());
}

void from_json(const nlohmann::json& json, PeerInfo& pi) {
    auto id = json[0].get_binary();
    auto ip = json[1].get_binary();
    auto port = json[2].get<int>();

    pi = PeerInfo(id, ip, port);
}

}
}
