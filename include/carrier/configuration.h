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

#include <list>
#include <any>
#include <map>

#include "def.h"
#include "types.h"
#include "node_info.h"
#include "socket_address.h"

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC Configuration {
public:
    virtual SocketAddress& ipv4Address() = 0;
    virtual SocketAddress& ipv6Address() = 0;

    virtual int listeningPort() = 0;

    /**
     * If a Path that points to an existing, writable directory is returned then the routing table
     * will be persisted to that directory periodically and during shutdown
     */
    virtual const std::string& getStoragePath() = 0;

    virtual std::vector<Sp<NodeInfo>>& getBootstrapNodes() = 0;

    virtual std::map<std::string, std::any>& getAddons() = 0;
};

} // namespace carrier
} // namespace elastos
