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

#include "constants.h"

namespace elastos {
namespace carrier {

const int Constants::DEFAULT_DHT_PORT                       = 39001;

const int Constants::RPC_SERVER_REACHABILITY_TIMEOUT        = 60 * 1000;
const int Constants::MAX_ACTIVE_CALLS                       = 256;
const int Constants::RPC_CALL_TIMEOUT_MAX                   = 10 * 1000;
const int Constants::RPC_CALL_TIMEOUT_BASELINE_MIN          = 100; // ms
const int Constants::RECEIVE_BUFFER_SIZE                    = 5 * 1024;

const int Constants::MAX_CONCURRENT_TASK_REQUESTS           = 10;
const int Constants::MAX_ACTIVE_TASKS                       = 16;

const int Constants::DHT_UPDATE_INTERVAL                    = 1000;
const int Constants::BOOTSTRAP_MIN_INTERVAL                 = 4 * 60 * 1000;
const int Constants::BOOTSTRAP_IF_LESS_THAN_X_PEERS         = 30;
const int Constants::SELF_LOOKUP_INTERVAL                   = 30 * 60 * 1000;   // 30 minutes
const int Constants::RANDOM_LOOKUP_INTERVAL                 = 10 * 60 * 1000;   // 10 minutes
const int Constants::RANDOM_PING_INTERVAL                   = 10 * 1000;        // 10 seconds
const int Constants::ROUTING_TABLE_PERSIST_INTERVAL         = 10 * 60 * 1000;   // 10 minutes

const int Constants::MAX_ENTRIES_PER_BUCKET                 = 8;
const int Constants::BUCKET_REFRESH_INTERVAL                = 15 * 60 * 1000;
const int Constants::ROUTING_TABLE_MAINTENANCE_INTERVAL     = 4 * 60 * 1000;
const int Constants::KBUCKET_MAX_TIMEOUTS                   = 5;
const int Constants::KBUCKET_OLD_AND_STALE_TIMEOUTS         = 2;

const int Constants::KBUCKET_OLD_AND_STALE_TIME             = 15 * 60 * 1000;
const int Constants::KBUCKET_PING_BACKOFF_BASE_INTERVAL     = 60 * 1000;
const int Constants::BUCKET_CACHE_PING_MIN_INTERVAL         = 30 * 1000;

const int Constants::STORAGE_EXPIRE_INTERVAL                = 5 * 60 * 1000;
const int Constants::TOKEN_TIMEOUT                          = 5 * 60 * 1000;
const int Constants::MAX_PEER_AGE                           = 120 * 60 * 1000;
const int Constants::MAX_VALUE_AGE                          = 120 * 60 * 1000;
const int Constants::RE_ANNOUNCE_INTERVAL                    = 5 * 60 * 1000;

const std::string Constants::NODE_NAME                      = "Meerkat";
const int Constants::NODE_VERSION                           = 1;
const std::string Constants::ENVIRONMENT_PROPERTY           = "elastos.carrier.enviroment";

}
}
