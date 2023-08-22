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
#include <sqlite3.h>

#include "carrier/types.h"
#include "carrier/id.h"
#include "carrier/value.h"
#include "data_storage.h"
#include "scheduler.h"

namespace elastos {
namespace carrier {

class SqliteStorage final : public DataStorage {
public:
    SqliteStorage() {}
    ~SqliteStorage();

    static Sp<DataStorage> open(const std::string& path, Scheduler& scheduler);
    void close() override;

    Sp<Value> getValue(const Id& valueId) override;
    bool removeValue(const Id& valueId) override;
    Sp<Value> putValue(const Value& value, int expectedSeq = -1, bool persistent = false, bool updateLastAnnounce = false) override;
    void updateValueLastAnnounce(const Id& valueId) override;
    std::vector<Value> getPersistentValues(uint64_t lastAnnounceBefore) override;
    std::vector<Id> getAllValues() override;

    std::vector<PeerInfo> getPeer(const Id& peerId, int maxPeers) override;
    Sp<PeerInfo> getPeer(const Id& peerId, const Id& origin) override;
    bool removePeer(const Id& peerId, const Id& origin) override;
    void putPeer(const std::vector<PeerInfo>& peers) override;
    void putPeer(const PeerInfo& peer, bool persistent = false, bool updateLastAnnounce = false) override;
    void updatePeerLastAnnounce(const Id& peerId, const Id& origin) override;
    std::vector<PeerInfo> getPersistentPeers(uint64_t lastAnnounceBefore) override;
    std::vector<Id> getAllPeers() override;

private:
    void init(const std::string& path, Scheduler& scheduler);
    void expire();
    int getUserVersion();

    sqlite3* sqlite_store {nullptr};
};

} // namespace carrier
} // namespace elastos
