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


#include "crawler.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#if defined(_WIN32) || defined(_WIN64)
    static const std::string PATH_SEP = "\\";
    #define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#else
    static const std::string PATH_SEP = "/";
#endif

static const std::string PATH_CWD = ".";

static uint32_t node_limit = 4;

namespace fs = std::filesystem;

void Crawler::init(std::string path) {
    setting.load(path);

    initCarrierNode();

    if (!setting.ip2location_database.empty())
        ip2location_init(setting.ip2location_database.c_str());
    else
        log->warn("IP2Location - no database configured, will disable location lookup.");

    openStorageFile();
}

void Crawler::start() {
    try {
        node->start();
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    for (auto ni: setting.bootstrapNodes) {
        pingNode(ni);
    }

    last_new_node_stamp = now();
}

void Crawler::stop() {
    node->stop();
    if (storageFile != nullptr) {
        storageFile->close();
    }
}

void Crawler::pingNode(Sp<NodeInfo> ni) {
    auto completeHandler = [=](Sp<NodeInfo> ni) {
        if (ni != NULL) {
            if (crawled(ni))
                return;

            nodes_list.emplace_back(ni);

            auto ip_str = ni->getAddress().toString();

            auto loc_str = ip2location(ip_str);
            auto str = "Id: " + ni->getId().toBase58String() + ", Ip: " + ip_str +
                        ", Version: " + ni->getReadableVersion() + ", " + loc_str;
            if (storageFile != nullptr)
                (*storageFile) << str << '\n';
            log->info(str);
            last_new_node_stamp = now();
        }
    };

    node->ping(ni, completeHandler);
    pinged_list.emplace_back(ni);
}

/*
* Sends a getnodes request to up to setting.requests_per_interval nodes in the nodes list that have not been queried.
* Returns the number of requests sent.
*/
void Crawler::sendNodeRequests() {
    size_t count = 0;
    uint32_t i = 0;

    if (!timedout(last_getnodes_request_stamp, setting.request_interval))
        return;

    auto completeHandler = [=](std::list<Sp<NodeInfo>> nis) {
        for(auto ni: nis) {
            if (pinged(ni))
                continue;

            pingNode(ni);
            last_new_node_stamp = now();
        }
    };

    for (i = sendedNo; count < setting.requests_per_interval && i < nodes_list.size(); ++i) {
        size_t j = 0;

        std::array<uint8_t, ID_BYTES> bytes {0};

        node->getNodes(Id(bytes), nodes_list[i], completeHandler);

        for (int j = 0; j < ID_BYTES; j++) {
            for (int k = 7; k >= 0; k--) {
                bytes[j] = 0xFF << k;
                Id id(bytes);
                auto str = id.toBinaryString();
                auto strh = id.toHexString();
                node->getNodes(id, nodes_list[i], completeHandler);
            }
        }

        for (int j = 0; j < ID_BYTES; j++) {
            for (int k = 1; k <= 8; k++) {
                bytes[j] = 0xFF >> k;
                Id id(bytes);
                auto str = id.toBinaryString();
                auto strh = id.toHexString();
                node->getNodes(id, nodes_list[i], completeHandler);
            }
        }

        for (int j = 0; j < setting.random_requests; ++j) {
            auto randId = Id::random();
            node->getNodes(randId, nodes_list[i], completeHandler);
        }
    }

    sendedNo = i;
    last_getnodes_request_stamp = now();
}

bool Crawler::pinged(Sp<NodeInfo> node) {
    auto id = node->getId();
    for (auto ni : pinged_list) {
        if (ni->getId() == id) {
            return true;
        }
    }

    return false;
}

bool Crawler::crawled(Sp<NodeInfo> node) {
    auto id = node->getId();
    for (auto ni : nodes_list) {
        if (ni->getId() == id) {
            return true;
        }
    }

    return false;
}

/* Returns true if the crawler is unable to find new nodes in the DHT or the exit flag has been triggered */
bool Crawler::finished() {
    auto num_nodes = nodes_list.size();
    if ((sendedNo == num_nodes &&
            timedout(last_new_node_stamp, setting.timeout))) {
        return true;
    }

    if (num_nodes >= setting.node_limit) {
        return true;
    }

    return false;
}


void Crawler::ip2location_init(std::string database)
{
    ip2LocationObj = IP2Location_open((char *)database.c_str());
    if (!ip2LocationObj) {
        throw std::invalid_argument("IP2Location - open database failed, check config file!");
    }

    if (IP2Location_open_mem(ip2LocationObj, IP2LOCATION_SHARED_MEMORY) == -1) {
        throw "IP2Location - open shared memory failed.";
    }
}

void Crawler::ip2location_cleanup(void) {
    IP2Location_close(ip2LocationObj);
    IP2Location_delete_shm();
}

std::string Crawler::ip2location(std::string ip) {
    IP2LocationRecord *record;
    std::string result;

    if (ip2LocationObj) {
        auto pos = ip.find(":");
        if (pos != -1) {
            ip = ip.substr(0, pos);
        }

        pthread_mutex_lock(&db_lock);
        record = IP2Location_get_all(ip2LocationObj, (char *)ip.c_str());
        pthread_mutex_unlock(&db_lock);

        if (record) {
            std::stringstream ss;
            ss << "Country: " << record->country_long << ", Region: " << record->region << ", City: " << record->city;
            result = ss.str();
            IP2Location_free_record(record);
        }
    }

    return result;
}

void Crawler::initCarrierNode() {
    auto builder = DefaultConfiguration::Builder{};

    if (setting.port > 0)
        builder.setListeningPort(setting.port);

    builder.setAutoIPv4Address(true);

    for (auto ni: setting.bootstrapNodes)
        builder.addBootstrap(ni);

    auto config = builder.build();

    node = std::make_shared<Node>(config);
}

bool Crawler::checkPersistence(const std::string& path) {
    if (path.empty()) {
        log->info("Storage path disabled, DHT node will not try to persist");
        return false;
    }

    if (path == PATH_CWD)
        return true;

    struct stat _stat;
    if (stat(path.c_str(), &_stat) != 0) {
        if (errno != ENOENT) {
            log->warn("Failed to access storage path {}. DHT node will not be able to persist state", path);
            return false;
        }
        if (!fs::create_directories(path)) {
            log->warn("Creating storage path {} failed. DHT node will not be able to persist state", path);
            return false;
        }
        return true;
    }

    if (!S_ISDIR(_stat.st_mode)) {
        log->warn("Storage path {} is not a directory. DHT node will not be able to persist state", path);
        return false;
    }
    return true;
}


void Crawler::openStorageFile() {
    auto storagePath = setting.data_dir.empty() ? PATH_CWD: setting.data_dir;

    char tmstr[32];
    time_t stamp = now();
    strftime(tmstr, sizeof(tmstr), "%Y-%m-%d", localtime(&stamp));
    storagePath += PATH_SEP + tmstr;

    if (!checkPersistence(storagePath))
        return;

    strftime(tmstr, sizeof(tmstr), "%H%M%S", localtime(&stamp));
    auto path =  storagePath + PATH_SEP + tmstr + ".lst";

    struct stat _stat {};
    auto rc = stat(path.c_str(), &_stat);
    if (rc != -1) {
        if (S_ISDIR(_stat.st_mode)) {
            log->warn("list file path {} is an existing directory. Crawler will not be able to persist list", path);
        }
        else {
            log->warn("list file path {} is existing. Crawler will not be able to persist list", path);
        }
        return;
    }

    storageFile = std::make_shared<std::ofstream>(path);
    if (!storageFile->is_open())
        throw std::runtime_error("Failed to open file for writing.");
}


