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

#include <fstream>
#include <memory>
#include <sys/stat.h>
#include <nlohmann/json.hpp>

#include "setting.h"
#include <carrier.h>

/* Minutes to wait between new crawler round */
#define CRAWLER_INTERVAL            30

/* Number of seconds to wait for new nodes before a crawler times out and exits */
#define CRAWLER_TIMEOUT             30

/* Default maximum number of nodes the nodes list can store */
#define INITIAL_NODES_LIST_SIZE     4096

/* Seconds to wait between getnodes requests */
#define REQUEST_INTERVAL            1

/* Max number of nodes to send getnodes requests to per GETNODES_REQUEST_INTERVAL */
#define REQUESTS_PER_INTERVAL       4

/* Number of random node requests to make for each node we send a request to */
#define RANDOM_REQUESTS             32

namespace elastos {
namespace carrier {

static std::string normailizePath(const std::string& path) {
    if (path.find("~") != std::string::npos) {
        return getenv("HOME") + path.substr(1);
    } else {
        return path;
    }
}

void CrawlerSetting::load(const std::string& filePath) {
    auto path = normailizePath(filePath);
    if (path.empty())
        throw std::invalid_argument("Invalid configuration file path: " + path);

    struct stat _stat;
    if (stat(path.c_str(), &_stat) == -1)
        throw std::invalid_argument("Configuration file: '" + path + "' not exist.");

    if (S_ISDIR(_stat.st_mode))
        throw std::invalid_argument("Invalid configuration file: " + path + ", should not be a directory.");

    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in.is_open())
        throw std::invalid_argument("Opening configuration failed " + path);

    auto root = nlohmann::json::parse(in);
    in.close();

    if (root.contains("logger")) {
        auto logSettings = root["logger"].get<nlohmann::json>();
        if (logSettings.contains("level"))
            Logger::setLogLevel(logSettings["level"].get<std::string>());

        if (logSettings.contains("logFile"))
            Logger::setLogFile(logSettings["logFile"].get<std::string>());

        if (logSettings.contains("pattern"))
            Logger::setLogPattern(logSettings["pattern"].get<std::string>());
    }

    if (root.contains("interval"))
        interval = root["interval"].get<int>();
    else
        interval = CRAWLER_INTERVAL;
    interval = interval * 60;

    if (root.contains("timeout"))
        timeout = root["timeout"].get<int>();
    else
        timeout = CRAWLER_TIMEOUT;

    if (root.contains("node_limit"))
        node_limit = root["node_limit"].get<uint32_t>();

    request_interval = REQUEST_INTERVAL;
    requests_per_interval = REQUESTS_PER_INTERVAL;
    random_requests = RANDOM_REQUESTS;

    if (root.contains("data_dir"))
        data_dir = normailizePath(root["data_dir"].get<std::string>());

    if (root.contains("ip2location_database"))
        ip2location_database = normailizePath(root["ip2location_database"].get<std::string>());

    if (root.contains("bootstraps")) {
        const auto bootstraps = root["bootstraps"];
        if (!bootstraps.is_array())
            throw std::invalid_argument("Config file error: bootstaps");

        for (const auto& bootstrap: bootstraps) {
            if (!bootstrap.contains("id"))
                throw std::invalid_argument("Config file error: bootstap node id");

            if (!bootstrap.contains("address"))
                throw std::invalid_argument("Config file error: bootstap node address");

            if (!bootstrap.contains("port"))
                throw std::invalid_argument("Config file error: bootstap node port");

            auto id = bootstrap["id"].get<std::string>();
            auto ip = bootstrap["address"].get<std::string>();
            auto port = bootstrap["port"].get<int>();

            auto node = std::make_shared<NodeInfo>(id, ip, port);
            bootstrapNodes.emplace_back(node);
        }
    }
}

}
}

