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

#include "carrier/id.h"
#include "carrier/value.h"
#include "carrier/peer_info.h"

namespace elastos {
namespace carrier {


class DataStorage {
public:
    virtual Sp<Value> getValue(const Id& valueId) = 0;
    virtual bool removeValue(const Id& valueId) = 0;
    virtual Sp<Value> putValue(const Value& value, int expectedSeq = -1, bool persistent = false, bool updateLastAnnounce = false) = 0;
    Sp<Value> putValue(const Value& value, bool persistent) {
        return putValue(value, -1, persistent, true);
    }
    virtual void updateValueLastAnnounce(const Id& valueId) = 0;
    virtual std::list<Value> getPersistentValues(uint64_t lastAnnounceBefore) = 0;
    virtual std::list<Id> getAllValues() = 0;

    virtual std::list<PeerInfo> getPeer(const Id& peerId, int maxPeers) = 0;
    virtual Sp<PeerInfo> getPeer(const Id& peerId, const Id& origin) = 0;
    virtual bool removePeer(const Id& peerId, const Id& origin) = 0;
    virtual void putPeer(const std::list<PeerInfo>& peers) = 0;
    virtual void putPeer(const PeerInfo& peer, bool persistent, bool updateLastAnnounce) = 0;
    void putPeer(const PeerInfo& peer, bool persistent) {
        return putPeer(peer, persistent, true);
    }
    void putPeer(const PeerInfo& peer) {
        return putPeer(peer, false, false);
    }
    virtual void updatePeerLastAnnounce(const Id& peerId, const Id& origin) = 0;
    virtual std::list<PeerInfo> getPersistentPeers(uint64_t lastAnnounceBefore) = 0;
    virtual std::list<Id> getAllPeers() = 0;

    virtual void close() = 0;
};

} // namespace carrier
} // namespace elastos
