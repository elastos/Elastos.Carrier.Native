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

#include <list>
#include <string>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <carrier.h>

#include "utils.h"
#include "utils/list.h"
#include "sqlite_storage.h"
#include "peerinfo_storage_tests.h"

using namespace elastos::carrier;

namespace test {
CPPUNIT_TEST_SUITE_REGISTRATION(PeerInfoStorageTests);

void PeerInfoStorageTests::setUp() {
    path = Utils::getPwdStorage("apitests.db");
}

void PeerInfoStorageTests::tearDown() {
    Utils::removeStorage(path);
}

void PeerInfoStorageTests::testPutAndGetPeer() {
    auto storage = SqliteStorage::open(path, scheduler);
    std::map<Id, std::list<PeerInfo>> allPeers {};

    std::list<Id> ids {};
    int basePort = 8000;

    std::cout << "Writing peers...";
    for (int i = 1; i <= 64; i++) {
        auto keypair = Signature::KeyPair::random();
        auto peerId = Id(keypair.publicKey());

        std::list<PeerInfo> peers = {};
        for (int j = 0; j < i; j++) {
            auto pi = PeerInfo::create(keypair, Id::random(), Id::random(), basePort + i);
            CPPUNIT_ASSERT(pi.getId() == peerId);
            peers.push_back(pi);

            pi = PeerInfo::create(keypair, Id::random(), Id::random(), basePort + i,  "alt:" + std::to_string(i));
            CPPUNIT_ASSERT(pi.getId() == peerId);
            peers.push_back(pi);
        }
        storage->putPeer(peers);
        allPeers.emplace(peerId, peers);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;
    }

    std::cout << std::endl << "Reading peers...";
    int total = 0;
    for (auto& entry : allPeers) {
        total++;
        auto peerId = entry.first;
        auto peers = entry.second;

        //all
        auto ps = storage->getPeer(peerId, peers.size() + 8);
        CPPUNIT_ASSERT_EQUAL(peers.size(), ps.size());

        auto c = [&](const PeerInfo& a, const PeerInfo& b) {
            int r = a.getNodeId().compareTo(b.getNodeId());
            if (r != 0)
                return r < 0;

            return a.getOrigin().compareTo(b.getOrigin()) < 0;
        };

        peers.sort(c);
        ps.sort(c);

        for (int i = 0; i < peers.size(); i++)
            CPPUNIT_ASSERT_EQUAL(list_get(peers, i), list_get(ps, i));

        // limited
        ps = storage->getPeer(peerId, 16);
        CPPUNIT_ASSERT_EQUAL(std::min<size_t>(16, peers.size()), ps.size());
        for (auto& peer : ps)
            CPPUNIT_ASSERT_EQUAL(list_get(peers, 0).getPort(), peer.getPort());

        for (auto& peer : peers) {
            auto pi = storage->getPeer(peer.getId(), peer.getOrigin());
            CPPUNIT_ASSERT(pi);
            CPPUNIT_ASSERT(*pi == peer);

            auto removed = storage->removePeer(peer.getId(), peer.getOrigin());
            CPPUNIT_ASSERT(removed);

            pi = storage->getPeer(peer.getId(), peer.getOrigin());
            CPPUNIT_ASSERT(!pi);

            removed = storage->removePeer(peer.getId(), peer.getOrigin());
            CPPUNIT_ASSERT(!removed);
        }

        std::cout << ".";
        if (total % 16 == 0)
            std::cout << std::endl;
    }

    CPPUNIT_ASSERT(total == 64);

    storage->close();
}

void PeerInfoStorageTests::testPutAndGetPersistentPeer() {
    auto storage = SqliteStorage::open(path, scheduler);

    std::list<Id> peerIds {};
    auto origin = Id::random();
    uint16_t basePort = 8000;
    std::vector<uint8_t> sig(64);

    std::cout << "Writing peers...";
    for (int i = 1; i <= 256; i++) {
        auto peer = PeerInfo::create(Id::random(), origin, basePort + i);

        peerIds.push_back(peer.getId());
        storage->putPeer(peer, i % 2 == 0);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;
    }

    auto ts = currentTimeMillis();
    std::list<PeerInfo> peers = storage->getPersistentPeers(ts);
    CPPUNIT_ASSERT(peers.size() == 128);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "\nUpdate the last announced for peers...";
    for (int i = 1; i <= 128; i++) {
        auto peerId = list_get(peers, i - 1).getId();

        if (i % 2 == 0)
            storage->updatePeerLastAnnounce(peerId, origin);
    }

    peers = storage->getPersistentPeers(ts);
    CPPUNIT_ASSERT(peers.size() == 64);

    std::cout << "\nReading values...";
    for (int i = 1; i <= 256; i++) {
        auto peerId = peerIds.front();
        auto peer = storage->getPeer(peerId, origin);
        CPPUNIT_ASSERT(peer);
        CPPUNIT_ASSERT(peer->getPort() == (uint16_t)(basePort + i));
        CPPUNIT_ASSERT(peer->getId() == peerId);

        auto removed = storage->removePeer(peerId, origin);
        CPPUNIT_ASSERT(removed);

        peer = storage->getPeer(peerId, origin);
        CPPUNIT_ASSERT(!peer);

        removed = storage->removePeer(peerId, origin);
        CPPUNIT_ASSERT(!removed);

        std::cout << ".";
        if (i % 16 == 0)
            std::cout << std::endl;

        peerIds.pop_front();
    }

    storage->close();
}

}  // namespace test
