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

#include <iostream>
#include <cstdlib>
#include <thread>

#include <IP2Location.h>

#include "setting.h"

using namespace elastos::carrier;

class Crawler {
public:
    Crawler() {
    }

    ~Crawler() {
        ip2location_cleanup();
    }

    void init(std::string path);

    void start();
    void stop();

    /*
    * Sends a getnodes request to up to setting.requests_per_interval nodes in the nodes list that have not been queried.
    * Returns the number of requests sent.
    */
    void sendNodeRequests();

    /* Returns true if the crawler is unable to find new nodes in the DHT or the exit flag has been triggered */
    bool finished();

    // /* Dumps crawler nodes list to file. */
    // void dumpNodes();

private:
    inline time_t now(void) {
        return time(NULL);
    }

    inline bool timedout(time_t timestamp, time_t timeout) {
        return timestamp + timeout <= now();
    }

    void pingNode(Sp<NodeInfo> ni);
    bool crawled(Sp<NodeInfo> node);
    bool checkPersistence(const std::string& path);
    void openStorageFile();

    void ip2location_init(std::string database);

    void ip2location_cleanup(void);

    std::string ip2location(std::string ip);

    void initCarrierNode();

    std::vector<Sp<NodeInfo>> nodes_list {};
    uint32_t     sendedNo {0};    /* index of the oldest node that we haven't sent a getnodes request to */
    time_t       last_new_node_stamp {0};   /* Last time we found an unknown node */
    time_t       last_getnodes_request_stamp {0};

    IP2Location *ip2LocationObj = nullptr;
    Sp<Node> node = nullptr;

 private:
    CrawlerSetting setting {};
    pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;
    Sp<std::ofstream> storageFile = nullptr;
    Sp<Logger> log = Logger::get("crawler");
};
