/*
* Copyright (c) 2022 trinity-tech.io
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

#include <vector>

#include <nlohmann/json.hpp>

#include "serializers.h"
#include "kbucket_entry.h"

namespace elastos {
namespace carrier {

long KBucketEntry::backoffWindowEnd() const {
    if (failedRequests == 0 || lastSend <= 0)
        return -1;

    int backoff = Constants::KBUCKET_PING_BACKOFF_BASE_INTERVAL <<
            std::min(Constants::KBUCKET_MAX_TIMEOUTS, std::max<int>(0, failedRequests - 1));

    return lastSend + backoff;
}

void KBucketEntry::merge(Sp<KBucketEntry> other) {
    assert(other);

    if (!this->equals(*other) || (this == other.get()))
        return;

    created  = std::min<uint64_t>(created,  other->getCreationTime());
    lastSeen = std::max<uint64_t>(lastSeen, other->getLastSeen());
    lastSend = std::max<uint64_t>(lastSend, other->getLastSend());

    if (other->isReachable())
        setReachable(true);

    if (other->getFailedRequests() > 0)
        failedRequests = std::min(failedRequests, other->getFailedRequests());
}

bool KBucketEntry::withinBackoffWindow(uint64_t now) const {
    int backoff = Constants::KBUCKET_PING_BACKOFF_BASE_INTERVAL <<
            std::max(Constants::KBUCKET_MAX_TIMEOUTS, std::max<int>(0, failedRequests - 1));

    return failedRequests != 0 && now - lastSend < backoff;
}

Sp<KBucketEntry> KBucketEntry::fromJson(nlohmann::json& root) {
    auto id = root.at("id").get<Id>();
    Blob ip = root.at("addr").get_binary();
    uint16_t port = root.at("port").get<uint16_t>();
    SocketAddress addr{ip, port};

    auto entry = std::make_shared<KBucketEntry>(id, addr, root.at("version").get<int>());
    entry->created = root.at("created");
    entry->lastSeen = root.at("lastSeen");
    entry->lastSend = root.at("lastSend");
    entry->failedRequests = root.at("failedRequests");
    entry->reachable = root.at("reachable");

    return entry;
}

nlohmann::json KBucketEntry::toJson() const {
    nlohmann::json root = nlohmann::json::object();

    std::vector<uint8_t> addr(getAddress().inaddrLength());
    memcpy(addr.data(), getAddress().inaddr(), addr.size());

    root["id"] = getId();
    root["addr"] = nlohmann::json::binary_t(addr);
    root["port"] = getAddress().port();
    root["created"] = created;
    root["lastSeen"] = lastSeen;
    root["lastSend"] = lastSend;
    root["failedRequests"] = failedRequests;
    root["reachable"] = isReachable();
    root["version"] = getVersion();

    return root;
}

std::string KBucketEntry::toString() const {
    auto now = currentTimeMillis();
    std::string str {};
    str.append(static_cast<std::string>(getId())).append(1, '@').append(getAddress().host());
    str.append(";seen:").append(std::to_string(now - lastSeen));
    str.append(";age:").append(std::to_string(now - created));

    if (lastSend > 0)
        str.append(";sent:" + std::to_string(now - lastSend));
    if (failedRequests != 0)
        str.append(";fail:" + std::to_string(failedRequests));
    if (reachable)
        str.append(";reachable");

    if (getVersion() != 0)
        str.append(";ver:").append(std::to_string(getVersion()));

    return str;
}

}
}
