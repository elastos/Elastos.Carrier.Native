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

#include "carrier/id.h"
#include "carrier/peer_info.h"
#include "utils/hex.h"
#include "constants.h"
#include "scheduler.h"
#include "sqlite_storage.h"

namespace elastos {
namespace carrier {

static std::string SET_USER_VERSION = "PRAGMA user_version = 1";

static std::string CREATE_VALUES_TABLE = "CREATE TABLE IF NOT EXISTS valores(\
    id BLOB NOT NULL PRIMARY KEY, \
    publicKey BLOB, \
    privateKey BLOB, \
    recipient BLOB, \
    nonce BLOB, \
    signature BLOB, \
    sequenceNumber INTEGER, \
    data BLOB, \
    timestamp BIGINT NOT NULL\
) WITHOUT ROWID";

static std::string CREATE_VALUES_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_valores_timpstamp ON valores(timestamp)";

static std::string CREATE_PEERS_TABLE = "CREATE TABLE IF NOT EXISTS peers(\
    id BLOB NOT NULL, \
    family INTEGER NOT NULL, \
    nodeId BLOB NOT NULL ,\
    ip BLOB NOT NULL, \
    port INTEGER NOT NULL, \
    timestamp BIGINT NOT NULL, \
    PRIMARY KEY(id, family, nodeId)\
) WITHOUT ROWID";

static std::string CREATE_PEERS_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_peers_timpstamp ON peers(timestamp)";

static std::string SELECT_VALUE = "SELECT * from valores \
    WHERE id = ? and timestamp >= ?";

static std::string LIST_VALUE_ID = "SELECT DISTINCT id from valores ORDER BY id";

static std::string UPSERT_VALUE = "INSERT INTO valores(\
    id, publicKey, privateKey, recipient, nonce, signature, sequenceNumber, data, timestamp) \
    VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?) ON CONFLICT(id) DO UPDATE SET \
    publicKey=excluded.publicKey, privateKey=excluded.privateKey, \
    recipient=excluded.recipient, nonce=excluded.nonce, \
    signature=excluded.signature, sequenceNumber=excluded.sequenceNumber, \
    data=excluded.data, timestamp=excluded.timestamp";

static std::string SELECT_PEER = "SELECT * from peers \
    WHERE id = ? and family = ? and timestamp >= ?\
    ORDER BY RANDOM() LIMIT ?";

static std::string SELECT_PEER_WITH_NODEID = "SELECT * from peers \
    WHERE id = ? and family = ? and nodeId = ? and timestamp >= ?";

static std::string UPSERT_PEER = "INSERT INTO peers(\
    id, family, nodeId, ip, port, timestamp) \
    VALUES(?, ?, ?, ?, ?, ?) ON CONFLICT(id, family, nodeId) DO UPDATE SET \
    ip=excluded.ip, port=excluded.port, timestamp=excluded.timestamp";

static std::string LIST_PEER_ID = "SELECT DISTINCT id from peers ORDER BY id";

SqliteStorage::~SqliteStorage() {
    if (sqlite_store) {
        sqlite3_close(sqlite_store);
        sqlite_store = NULL;
    }
}

void SqliteStorage::expire() {
    const char *sqls[2] = { "DELETE FROM valores WHERE timestamp < ?", "DELETE FROM peers WHERE timestamp < ?" };
    uint64_t ts[2];
    ts[0] = currentTimeMillis() - Constants::MAX_VALUE_AGE;
    ts[1] = currentTimeMillis() - Constants::MAX_PEER_AGE;

    for (int i = 0; i < 2; i++) {
        sqlite3_stmt *pStmt;
        if (sqlite3_prepare_v2(sqlite_store, sqls[i], strlen(sqls[i]), &pStmt, 0) != SQLITE_OK) {
            sqlite3_finalize(pStmt);
            throw std::runtime_error("Prepare sqlite failed.");
        }

        sqlite3_bind_int64(pStmt, 1, ts[i]);
        sqlite3_step(pStmt);
        sqlite3_finalize(pStmt);
    }
}

void SqliteStorage::init(const std::string& path, Scheduler& scheduler) {
    int rc = sqlite3_open(path.c_str(), &sqlite_store);
    if (rc)
        throw std::runtime_error("Failed to open the SQLite storage.");

    if (sqlite3_exec(sqlite_store, SET_USER_VERSION.c_str(), 0, 0, 0) != 0 ||
        sqlite3_exec(sqlite_store, CREATE_VALUES_TABLE.c_str(), 0, 0, 0) != 0 ||
        sqlite3_exec(sqlite_store, CREATE_VALUES_INDEX.c_str(), 0, 0, 0) != 0 ||
        sqlite3_exec(sqlite_store, CREATE_PEERS_TABLE.c_str(), 0, 0, 0) != 0 ||
        sqlite3_exec(sqlite_store, CREATE_PEERS_INDEX.c_str(), 0, 0, 0) != 0) {
        throw std::runtime_error("Failed to update SQLite text.");
    }

    scheduler.add([=]() {
        expire();
    }, 0, Constants::STORAGE_EXPIRE_INTERVAL);
}

Sp<DataStorage> SqliteStorage::open(const std::string& path, Scheduler& scheduler) {
    Sp<SqliteStorage> storage = std::make_shared<SqliteStorage>();
    storage->init(path, scheduler);
    return std::static_pointer_cast<DataStorage>(storage);
}

void SqliteStorage::close() {
    if (sqlite_store) {
        sqlite3_close(sqlite_store);
        sqlite_store = NULL;
    }
}

Sp<Value> SqliteStorage::getValue(const Id& valueId) {
    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, SELECT_VALUE.c_str(), strlen(SELECT_VALUE.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    const uint64_t when = currentTimeMillis() - Constants::MAX_VALUE_AGE;
    sqlite3_bind_blob(pStmt, 1, valueId.data(), valueId.size(), SQLITE_STATIC);
    sqlite3_bind_int64(pStmt, 2, when);

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        auto value = std::make_shared<Value>();

        int cNum = sqlite3_column_count(pStmt);
        for (int i = 0; i < cNum; i++) {
            const char* name = sqlite3_column_name(pStmt, i);
            const int cType = sqlite3_column_type(pStmt, i);
            int len= 0;
            const void *ptr = NULL;

            if (cType == SQLITE_BLOB) {
                len = sqlite3_column_bytes(pStmt, i);
                ptr = sqlite3_column_blob(pStmt, i);
            }

            if (std::strcmp(name, "publicKey") == 0 && len > 0) {
                value->setPublicKey(Blob(ptr, len));
            } else if (std::strcmp(name, "privateKey") == 0 && len > 0) {
                value->setPrivateKey(Blob(ptr, len));
            } else if (strcmp(name, "recipient") == 0 && len > 0) {
                value->setRecipient(Blob(ptr, len));
            } else if (strcmp(name, "nonce") == 0 && len > 0 && len == CryptoBox::Nonce::BYTES) {
                value->setNonce(Blob(ptr, len));
            } else if (strcmp(name, "signature") == 0 && len > 0) {
                value->setSignature(Blob(ptr, len));
            } else if (strcmp(name, "sequenceNumber") == 0 && cType == SQLITE_INTEGER) {
                value->setSequenceNumber(sqlite3_column_int(pStmt, i));
            } else if (strcmp(name, "data") == 0 && len > 0) {
                value->setData(Blob(ptr, len));
            }
        }

        sqlite3_finalize(pStmt);
        return value;
    }

    sqlite3_finalize(pStmt);
    return nullptr;
}

Sp<Value> SqliteStorage::putValue(const Sp<Value>& value, int expectedSeq) {
    sqlite3_stmt *pStmt;

    if (value->isMutable() && !value->isValid())
        throw std::invalid_argument("Value signature validation failed");

    auto id = value->getId();
    auto old = getValue(id);
    if (old != nullptr && old->isMutable()) {
        if(!value->isMutable())
            throw std::invalid_argument("Can not replace mutable value with immutable is not supported");
        if (old->hasPrivateKey() && !value->hasPrivateKey())
            throw std::invalid_argument("Not the owner of value");
        if(value->getSequenceNumber() < old->getSequenceNumber())
            throw std::invalid_argument("Sequence number less than current");
        if(expectedSeq >= 0 && old->getSequenceNumber() >= 0 && old->getSequenceNumber() != expectedSeq)
            throw std::invalid_argument("CAS failure");
    }

    if (sqlite3_prepare_v2(sqlite_store, UPSERT_VALUE.c_str(), strlen(UPSERT_VALUE.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    sqlite3_bind_blob(pStmt, 1, id.data(), id.size(), SQLITE_STATIC);

    auto signer = value->getPublicKey().asBlob();
    sqlite3_bind_blob(pStmt, 2, signer.ptr(), signer.size(), SQLITE_STATIC);

    auto sk = value->getPrivateKey();
    sqlite3_bind_blob(pStmt, 3, sk.bytes(), sk.size(), SQLITE_STATIC);

    auto recipient = value->getRecipient().asBlob();
    sqlite3_bind_blob(pStmt, 4, recipient.ptr(), recipient.size(), SQLITE_STATIC);

    auto nonce = value->getNonce();
    sqlite3_bind_blob(pStmt, 5, nonce.bytes(), nonce.size(), SQLITE_STATIC);

    auto sig = Blob(value->getSignature());
    sqlite3_bind_blob(pStmt, 6, sig.ptr(), sig.size(), SQLITE_STATIC);

    sqlite3_bind_int(pStmt, 7, value->getSequenceNumber());

    auto data = Blob(value->getData());
    sqlite3_bind_blob(pStmt, 8, data.ptr(), data.size(), SQLITE_STATIC);

    sqlite3_bind_int64(pStmt, 9, currentTimeMillis());

    sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    return old;
}

Sp<Value> SqliteStorage::putValue(const Sp<Value>& value) {
    return putValue(value, -1);
}

std::list<Id> SqliteStorage::listValueId() {
    std::list<Id> ids {};

    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, LIST_VALUE_ID.c_str(), strlen(LIST_VALUE_ID.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        int cNum = sqlite3_column_count(pStmt);
        for (int i = 0; i < cNum; i++) {
            const int cType = sqlite3_column_type(pStmt, i);
            int len= 0;
            const void *ptr = NULL;

            if (cType == SQLITE_BLOB) {
                len = sqlite3_column_bytes(pStmt, i);
                ptr = sqlite3_column_blob(pStmt, i);

                auto id = Id((uint8_t*)ptr, len);
                ids.emplace_back(id);
            }
        }
    }

    sqlite3_finalize(pStmt);
    return ids;
}

std::list<Sp<PeerInfo>> SqliteStorage::getPeer(const Id& peerId, int family, int maxPeers) {
    std::list<int> families = {};

    if (family == 4) {
        families.push_back(4);
    } else if (family == 6) {
        families.push_back(6);
    } else if (family == 10) { // IPv4 + IPv6
        families.push_back(4);
        families.push_back(6);
    }

    if (maxPeers <=0)
        maxPeers = 0x7fffffff;

    std::list<Sp<PeerInfo>> peers = {};
    for (auto it = families.begin(); it != families.end(); it++) {
        sqlite3_stmt *pStmt;

        if(sqlite3_prepare_v2(sqlite_store, SELECT_PEER.c_str(), strlen(SELECT_PEER.c_str()), &pStmt, 0) != SQLITE_OK) {
            sqlite3_finalize(pStmt);
            throw std::runtime_error("Prepare sqlite failed.");
        }

        uint64_t when = currentTimeMillis() - Constants::MAX_VALUE_AGE;
        sqlite3_bind_blob(pStmt, 1, peerId.data(), peerId.size(), SQLITE_STATIC);
        sqlite3_bind_int(pStmt, 2, *it);
        sqlite3_bind_int64(pStmt, 3, when);
        sqlite3_bind_int(pStmt, 4, maxPeers);

        while (sqlite3_step(pStmt) == SQLITE_ROW) {
            auto peer = std::make_shared<PeerInfo>();
            std::vector<uint8_t> ip;
            int port;

            int cNum = sqlite3_column_count(pStmt);
            for (int i = 0; i < cNum; i++) {
                const char *name = sqlite3_column_name(pStmt, i);
                int cType = sqlite3_column_type(pStmt, i);
                int len = 0;
                const void *ptr = NULL;

                if (cType == SQLITE_BLOB) {
                    len = sqlite3_column_bytes(pStmt, i);
                    ptr = sqlite3_column_blob(pStmt, i);
                }

                if (std::strcmp(name, "nodeId") == 0 && len > 0) {
                    peer->setNodeId(Blob(ptr, len));
                } else if (std::strcmp(name, "ip") == 0 && len > 0) {
                    ip.resize(len);
                    std::memcpy(ip.data(), (uint8_t*)ptr, len);
                } else if (std::strcmp(name, "port") == 0) {
                    port = sqlite3_column_int(pStmt, i);
                }
            }

            peer->setSocketAddress(ip.data(), ip.size(), port);
            peers.push_back(peer);
        }
        sqlite3_finalize(pStmt);
    }

    return peers;
}

Sp<PeerInfo> SqliteStorage::getPeer(const Id& peerId, int family, const Id& nodeId) {
    sqlite3_stmt *pStmt {nullptr};
    if(sqlite3_prepare_v2(sqlite_store, SELECT_PEER_WITH_NODEID.c_str(), strlen(SELECT_PEER_WITH_NODEID.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");
    }

    const uint64_t when = currentTimeMillis() - Constants::MAX_VALUE_AGE;
    sqlite3_bind_blob(pStmt, 1, peerId.data(), peerId.size(), SQLITE_STATIC);
    sqlite3_bind_int(pStmt, 2, family);
    sqlite3_bind_blob(pStmt, 3, nodeId.data(), nodeId.size(), SQLITE_STATIC);
    sqlite3_bind_int64(pStmt, 4, when);

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        auto peer = std::make_shared<PeerInfo>();
        std::vector<std::uint8_t> ip;
        int port;

        const int cNum = sqlite3_column_count(pStmt);
        for (int i = 0; i < cNum; i++) {
            const char* name = sqlite3_column_name(pStmt, i);
            const int cType = sqlite3_column_type(pStmt, i);
            int len = 0;
            const void *ptr = NULL;

            if (cType == SQLITE_BLOB) {
                len = sqlite3_column_bytes(pStmt, i);
                ptr = sqlite3_column_blob(pStmt, i);
            }

            if (std::strcmp(name, "nodeId") == 0 && len > 0) {
                peer->setNodeId(Blob(ptr, len));
            } else if (std::strcmp(name, "ip") == 0 && len > 0) {
                ip.reserve(len);
                std::memcpy(ip.data(), ptr, len);
            } else if (std::strcmp(name, "port") == 0 && cType == SQLITE_INTEGER) {
                port = sqlite3_column_int(pStmt, i);
            }
        }
        sqlite3_finalize(pStmt);
        peer->setSocketAddress(ip.data(), ip.size(), port);
        return peer;
    }

    sqlite3_finalize(pStmt);
    return nullptr;
}

void SqliteStorage::putPeer(const Id& peerId, const std::list<Sp<PeerInfo>>& peers) {
    if (sqlite3_exec(sqlite_store, "BEGIN", 0, 0, 0) != 0)
        throw std::runtime_error("Open auto commit mode failed.");

    sqlite3_stmt *pStmt {nullptr};
    if(sqlite3_prepare_v2(sqlite_store, UPSERT_PEER.c_str(), strlen(UPSERT_PEER.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");
    }

    uint64_t now = currentTimeMillis();
    for (const auto& peer : peers) {
        sqlite3_bind_blob(pStmt, 1, peerId.data(), peerId.size(), SQLITE_STATIC);
        sqlite3_bind_int(pStmt, 2, peer->isIPv4() ? 4 : 6);
        sqlite3_bind_blob(pStmt, 3, peer->getNodeId().data(), peer->getNodeId().size(), SQLITE_STATIC);
        sqlite3_bind_blob(pStmt, 4, peer->getAddress().inaddr(), peer->getAddress().inaddrLength(), SQLITE_STATIC);
        sqlite3_bind_int(pStmt, 5, peer->getPort());
        sqlite3_bind_int64(pStmt, 6, now);

        if (sqlite3_step(pStmt) != SQLITE_DONE) {
            sqlite3_finalize(pStmt);
            throw std::runtime_error("Step sqlite failed.");
        }

        sqlite3_reset(pStmt);
    }

    sqlite3_finalize(pStmt);
    sqlite3_exec(sqlite_store, "COMMIT", 0, 0, 0);
}

void SqliteStorage::putPeer(const Id& peerId, const Sp<PeerInfo>& peer) {
    putPeer(peerId, std::list<Sp<PeerInfo>>{peer});
}

std::list<Id> SqliteStorage::listPeerId() {
    std::list<Id> ids {};

    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, LIST_PEER_ID.c_str(), strlen(LIST_PEER_ID.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        int cNum = sqlite3_column_count(pStmt);
        for (int i = 0; i < cNum; i++) {
            const int cType = sqlite3_column_type(pStmt, i);
            int len= 0;
            const void *ptr = NULL;

            if (cType == SQLITE_BLOB) {
                len = sqlite3_column_bytes(pStmt, i);
                ptr = sqlite3_column_blob(pStmt, i);

                auto id = Id((uint8_t*)ptr, len);
                ids.emplace_back(id);
            }
        }
    }

    sqlite3_finalize(pStmt);
    return ids;
}

}
}
