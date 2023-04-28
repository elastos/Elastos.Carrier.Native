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

#include <fstream>
#include <memory>
#include <sys/stat.h>

#include "utils/addr.h"
#include "utils/json_to_any.h"
#include "carrier/default_configuration.h"
#include "carrier/log.h"

#if defined(_WIN32) || defined(_WIN64)
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#endif

namespace elastos {
namespace carrier {

typedef DefaultConfiguration::Builder Builder;

std::string normailize(const std::string& path) {
    if (path.find("~") != std::string::npos) {
        return getenv("HOME") + path.substr(1);
    } else {
        return path;
    }
}

void Builder::load(const std::string& filePath) {
    auto path = normailize(filePath);
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

    bool enabled {false};

    enabled = root.contains("ipv4") ? root["ipv4"].get<bool>() : true;
    setAutoIPv4Address(enabled);
    if (enabled && root.contains("address4"))
        setIPv4Address(root["address4"].get<std::string>());

    enabled = root.contains("ipv6") ? root["ipv6"].get<bool>() : false;
    setAutoIPv6Address(enabled);
    if (enabled && root.contains("address6"))
        setIPv6Address(root["address6"].get<std::string>());

    if (root.contains("port"))
        setListeningPort(root["port"].get<int>());

    if (root.contains("dataDir"))
        setStoragePath(root["dataDir"].get<std::string>());

    if (root.contains("logger")) {
        auto logSettings = root["logger"].get<nlohmann::json>();
        Logger::setDefaultSettings(jsonToAny(logSettings));
    }

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

    if (root.contains("services")) {
        const auto _services = root["services"];
        if (!_services.is_array())
            throw std::invalid_argument("Config file error: services");

        for (const auto& service: _services) {
            if (!service.contains("name"))
                throw std::invalid_argument("Config file error: service name");

            if (!service.contains("configuration"))
                throw std::invalid_argument("Config file error: service configuration");

            auto name = service["name"].get<std::string>();
            auto configuration = service["configuration"].get<nlohmann::json>();
            services[name] = jsonToAny(configuration);
        }

    }
}

void Builder::reset() {
    autoAddr4 = true;
    autoAddr6 = false;
    ip4 = {};
    ip6 = {};
    port = 39001;
    storagePath = {};
    bootstrapNodes.clear();
    services.clear();
}

Sp<Configuration> Builder::build() {
    if (autoAddr4 && ip4.empty())
        ip4 = getLocalIpAddresses();

    if (autoAddr6 && ip6.empty())
        ip6 = getLocalIpAddresses(false);

    auto dataStorage = std::make_shared<DefaultConfiguration>(ip4, ip6,  port, normailize(storagePath), bootstrapNodes, services);
    return std::static_pointer_cast<Configuration>(dataStorage);
}

}
}
