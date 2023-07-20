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

#include <string>

namespace elastos {
namespace carrier {

struct Constants {
    ///////////////////////////////////////////////////////////////////////////
    // Default DHT port
    ///////////////////////////////////////////////////////////////////////////
    // IANA - https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml
    // 38866-39062, Unassigned
    static const int        DEFAULT_DHT_PORT;

    ///////////////////////////////////////////////////////////////////////////
    // RPC server constants
    ///////////////////////////////////////////////////////////////////////////
    // enter survival mode if we don't see new packets after this time
    static const int        RPC_SERVER_REACHABILITY_TIMEOUT;
    static const int        MAX_ACTIVE_CALLS;
    static const int        RPC_CALL_TIMEOUT_MAX;
    static const int        RPC_CALL_TIMEOUT_BASELINE_MIN;
    static const int        RECEIVE_BUFFER_SIZE;

    ///////////////////////////////////////////////////////////////////////////
    // Task & Lookup constants
    ///////////////////////////////////////////////////////////////////////////
    static const int        MAX_CONCURRENT_TASK_REQUESTS;
    static const int        MAX_ACTIVE_TASKS;

    ///////////////////////////////////////////////////////////////////////////
    // DHT maintenance constants
    ///////////////////////////////////////////////////////////////////////////
    static const int        DHT_UPDATE_INTERVAL;
    static const int        BOOTSTRAP_MIN_INTERVAL;
    static const int        BOOTSTRAP_IF_LESS_THAN_X_PEERS;
    static const int        SELF_LOOKUP_INTERVAL;
    static const int        RANDOM_LOOKUP_INTERVAL;
    static const int        RANDOM_PING_INTERVAL;
    static const int        ROUTING_TABLE_PERSIST_INTERVAL;

    ///////////////////////////////////////////////////////////////////////////
    // Routing table and KBucket constants
    ///////////////////////////////////////////////////////////////////////////
    static const int        MAX_ENTRIES_PER_BUCKET;
    static const int        BUCKET_REFRESH_INTERVAL;
    static const int        ROUTING_TABLE_MAINTENANCE_INTERVAL;
    // 5 timeouts, used for exponential back-off as per Kademlia paper
    static const int        KBUCKET_MAX_TIMEOUTS;
    static const int        KBUCKET_OLD_AND_STALE_TIMEOUTS;
    // haven't seen it for a long time + timeout == evict sooner than pure timeout
    // based threshold. e.g. for old entries that we haven't touched for a long time
    static const int        KBUCKET_OLD_AND_STALE_TIME;
    static const int        KBUCKET_PING_BACKOFF_BASE_INTERVAL;
    static const int        BUCKET_CACHE_PING_MIN_INTERVAL;

    ///////////////////////////////////////////////////////////////////////////
    // Tokens and data storage constants
    ///////////////////////////////////////////////////////////////////////////
    static const int        STORAGE_EXPIRE_INTERVAL;
    static const int        TOKEN_TIMEOUT;
    static const int        MAX_PEER_AGE;
    static const int        MAX_VALUE_AGE;
    static const int        RE_ANNOUNCE_INTERVAL;

    ///////////////////////////////////////////////////////////////////////////
    // Node software name and version
    ///////////////////////////////////////////////////////////////////////////
    static const std::string    NODE_NAME;
    static const int            NODE_VERSION;
    static const int            VERSION					= 1;

    ///////////////////////////////////////////////////////////////////////////
    // Development environment
    ///////////////////////////////////////////////////////////////////////////
    static const std::string    ENVIRONMENT_PROPERTY;
};

} /* namespace carrier */
} /* namespace elastos */
