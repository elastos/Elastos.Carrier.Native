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
    Sp<Value> putValue(const Sp<Value>& value, int expectedSeq) override;
    Sp<Value> putValue(const Sp<Value>& value) override;
    std::list<Id> listValueId() override;

    std::list<Sp<PeerInfo>> getPeer(const Id& peerId, int family, int maxPeers) override;
    Sp<PeerInfo> getPeer(const Id& peerId, int family, const Id& nodeId) override;
    void putPeer(const Id& peerId, const std::list<Sp<PeerInfo>>& peers) override;
    void putPeer(const Id& peerId, const Sp<PeerInfo>& peer) override;
    std::list<Id> listPeerId() override;

private:
    void init(const std::string& path, Scheduler& scheduler);
    void expire();
    int getUserVersion();

    sqlite3* sqlite_store {nullptr};
};

} // namespace carrier
} // namespace elastos
