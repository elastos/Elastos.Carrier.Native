/*
 * Copyright (c) 2023 trinity-tech.io
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

#include <string>
#include <vector>
#include <carrier.h>

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC CrawlerSetting {
public:

    void load(const std::string& path);

    uint32_t interval; /* minutes */
    uint32_t timeout;  /* seconds */

    uint32_t initial_nodes_list_size;
    uint32_t request_interval; /* seconds */
    uint32_t requests_per_interval;
    uint32_t random_requests;

    uint32_t node_limit {UINT32_MAX};

    std::string data_dir;

    int log_level;
    std::string log_file;

    std::string ip2location_database;

    std::vector<Sp<NodeInfo>> bootstrapNodes {};

    uint16_t port {39005};
};

} // namespace carrier
} // namespace elastos
