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

static int VERSION = 3;
static std::string SET_USER_VERSION = "PRAGMA user_version = " + std::to_string(VERSION);
static std::string GET_USER_VERSION = "PRAGMA user_version";

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

static std::string CREATE_PEERS_TABLE = "CREATE TABLE IF NOT EXISTS peers (\
    id BLOB NOT NULL, \
    privateKey BLOB, \
    nodeId BLOB NOT NULL ,\
    origin BLOB NOT NULL, \
    port INTEGER NOT NULL, \
    alternativeURL VARCHAR(512), \
    signature BLOB NOT NULL ,\
    timestamp INTEGER NOT NULL, \
    PRIMARY KEY(id, family, nodeId)\
) WITHOUT ROWID";

static std::string CREATE_PEERS_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_peers_timpstamp ON peers (timestamp)";

static std::string CREATE_PEERS_ID_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_peers_id ON peers(id)";

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
    WHERE id = ? and timestamp >= ? \
    ORDER BY RANDOM() LIMIT ?";

static std::string SELECT_PEER_WITH_NODEID = "SELECT * from peers \
    WHERE id = ? and origin = ? and timestamp >= ?";

static std::string UPSERT_PEER = "INSERT INTO peers (\
    id, privateKey, nodeId, origin, port, alternativeURL, signature, timestamp) \
    VALUES(?, ?, ?, ?, ?, ?, ?, ?) ON CONFLICT(id, nodeId, origin) DO UPDATE SET \
    privateKey=excluded.privateKey, port=excluded.port, alternativeURL=excluded.alternativeURL, \
    signature=excluded.signature, timestamp=excluded.timestamp";

static std::string LIST_PEER_ID = "SELECT DISTINCT id from peers ORDER BY id";

SqliteStorage::~SqliteStorage() {
    if (sqlite_store) {
        sqlite3_close(sqlite_store);
        sqlite_store = NULL;
    }
}

void SqliteStorage::expire() {
    std::string delPeers = "DELETE FROM peers WHERE timestamp < ?";
    const char *sqls[2] = { "DELETE FROM valores WHERE timestamp < ?",  delPeers.c_str()};
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

    // if we change the schema,
    // we should check the user version, do the schema update,
    // then increase the user_version;
    int userVersion = getUserVersion();
    if (userVersion == 1) {
        if (sqlite3_exec(sqlite_store, "DROP TABLE IF EXISTS peers", 0, 0, 0) != 0
            || sqlite3_exec(sqlite_store, "DROP INDEX IF EXISTS idx_peers_timpstamp", 0, 0, 0) != 0
            || sqlite3_exec(sqlite_store, "DROP INDEX IF EXISTS idx_peers_id", 0, 0, 0) != 0) {
            throw std::runtime_error("Failed to drop peers table.");
        }
    }

    if (sqlite3_exec(sqlite_store, SET_USER_VERSION.c_str(), 0, 0, 0) != 0 ||
        sqlite3_exec(sqlite_store, CREATE_VALUES_TABLE.c_str(), 0, 0, 0) != 0 ||
        sqlite3_exec(sqlite_store, CREATE_VALUES_INDEX.c_str(), 0, 0, 0) != 0 ||
        sqlite3_exec(sqlite_store, CREATE_PEERS_TABLE.c_str(), 0, 0, 0) != 0 ||
        sqlite3_exec(sqlite_store, CREATE_PEERS_INDEX.c_str(), 0, 0, 0) != 0 ||
        sqlite3_exec(sqlite_store, CREATE_PEERS_ID_INDEX.c_str(), 0, 0, 0) != 0) {
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

int SqliteStorage::getUserVersion() {
    int userVersion = 0;
    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, GET_USER_VERSION.c_str(), strlen(GET_USER_VERSION.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    if (sqlite3_step(pStmt) == SQLITE_ROW)
        userVersion = sqlite3_column_int(pStmt, 0);

    sqlite3_finalize(pStmt);
    return userVersion;
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

    if (sqlite3_step(pStmt) == SQLITE_ROW) {
        Id publicKey {};
        Blob privateKey {};
        Id recipient {};
        Blob nonce {};
        int sequenceNumber {0};
        std::vector<uint8_t>  signature {};
        std::vector<uint8_t>  data {};

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
                publicKey = Id(Blob(ptr, len));
            } else if (std::strcmp(name, "privateKey") == 0 && len > 0) {
                privateKey = Blob(ptr, len);
            } else if (strcmp(name, "recipient") == 0 && len > 0) {
                recipient = Id(Blob(ptr, len));
            } else if (strcmp(name, "nonce") == 0 && len > 0 && len == CryptoBox::Nonce::BYTES) {
                nonce = Blob(ptr, len);
            } else if (strcmp(name, "signature") == 0 && len > 0) {
                signature = {Blob(ptr, len)};
            } else if (strcmp(name, "sequenceNumber") == 0 && cType == SQLITE_INTEGER) {
                sequenceNumber = sqlite3_column_int(pStmt, i);
            } else if (strcmp(name, "data") == 0 && len > 0) {
                data = {Blob(ptr, len)};
            }
        }

        sqlite3_finalize(pStmt);
        auto value = Value::of(publicKey, privateKey, recipient, nonce, sequenceNumber, signature, data);
        return std::make_shared<Value>(value);
    }

    sqlite3_finalize(pStmt);
    return nullptr;
}

Sp<Value> SqliteStorage::putValue(const Value& value, int expectedSeq) {
    sqlite3_stmt *pStmt;

    if (value.isMutable() && !value.isValid())
        throw std::invalid_argument("Value signature validation failed");

    auto id = value.getId();
    auto old = getValue(id);
    if (old != nullptr && old->isMutable()) {
        if(!value.isMutable())
            throw std::invalid_argument("Can not replace mutable value with immutable is not supported");
        if (old->hasPrivateKey() && !value.hasPrivateKey())
            throw std::invalid_argument("Not the owner of value");
        if(value.getSequenceNumber() < old->getSequenceNumber())
            throw std::invalid_argument("Sequence number less than current");
        if(expectedSeq >= 0 && old->getSequenceNumber() >= 0 && old->getSequenceNumber() != expectedSeq)
            throw std::invalid_argument("CAS failure");
    }

    if (sqlite3_prepare_v2(sqlite_store, UPSERT_VALUE.c_str(), strlen(UPSERT_VALUE.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    sqlite3_bind_blob(pStmt, 1, id.data(), id.size(), SQLITE_STATIC);

    auto signer = value.getPublicKey().blob();
    sqlite3_bind_blob(pStmt, 2, signer.ptr(), signer.size(), SQLITE_STATIC);

    auto sk = value.getPrivateKey();
    sqlite3_bind_blob(pStmt, 3, sk.bytes(), sk.size(), SQLITE_STATIC);

    auto recipient = value.getRecipient().blob();
    sqlite3_bind_blob(pStmt, 4, recipient.ptr(), recipient.size(), SQLITE_STATIC);

    auto nonce = value.getNonce();
    sqlite3_bind_blob(pStmt, 5, nonce.ptr(), nonce.size(), SQLITE_STATIC);

    auto sig = Blob(value.getSignature());
    sqlite3_bind_blob(pStmt, 6, sig.ptr(), sig.size(), SQLITE_STATIC);

    sqlite3_bind_int(pStmt, 7, value.getSequenceNumber());

    auto data = Blob(value.getData());
    sqlite3_bind_blob(pStmt, 8, data.ptr(), data.size(), SQLITE_STATIC);

    sqlite3_bind_int64(pStmt, 9, currentTimeMillis());

    sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    return old;
}

Sp<Value> SqliteStorage::putValue(const Value& value) {
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
            size_t len = 0;
            const uint8_t* ptr = NULL;

            if (cType == SQLITE_BLOB) {
                len = sqlite3_column_bytes(pStmt, i);
                ptr = (const uint8_t*)sqlite3_column_blob(pStmt, i);

                auto id = Id({ptr, len});
                ids.emplace_back(id);
            }
        }
    }

    sqlite3_finalize(pStmt);
    return ids;
}

std::list<PeerInfo> SqliteStorage::getPeer(const Id& peerId, int maxPeers) {
    if (maxPeers <=0)
        maxPeers = 0x7fffffff;

    std::list<PeerInfo> peers = {};
    sqlite3_stmt *pStmt;
    if(sqlite3_prepare_v2(sqlite_store, SELECT_PEER.c_str(), strlen(SELECT_PEER.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");
    }

    uint64_t when = currentTimeMillis() - Constants::MAX_VALUE_AGE;
    sqlite3_bind_blob(pStmt, 1, peerId.data(), peerId.size(), SQLITE_STATIC);
    sqlite3_bind_int64(pStmt, 2, when);
    sqlite3_bind_int(pStmt, 3, maxPeers);

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        Blob privateKey {};
        Id nodeId {};
        Id origin {};
        uint16_t port {0};
        std::string alt {};
        std::vector<uint8_t>  signature {};

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

            if (std::strcmp(name, "privateKey") == 0 && len > 0) {
                privateKey = Blob(ptr, len);
            } else if (std::strcmp(name, "nodeId") == 0 && len > 0) {
                nodeId = Id(Blob(ptr, len));
            } else if (std::strcmp(name, "origin") == 0 && len > 0) {
                origin = Id(Blob(ptr, len));
            } else if (std::strcmp(name, "port") == 0) {
                port = sqlite3_column_int(pStmt, i);
            } else if (std::strcmp(name, "alternativeURL") == 0) {
                alt = (char *)sqlite3_column_text(pStmt, i);
            } else if (std::strcmp(name, "signature") == 0) {
                signature = {Blob(ptr, len)};
            }
        }

        PeerInfo peer = PeerInfo::of(peerId, privateKey, nodeId, origin, port, alt, signature);
        peers.push_back(peer);
    }
    sqlite3_finalize(pStmt);

    return peers;
}

Sp<PeerInfo> SqliteStorage::getPeer(const Id& peerId, const Id& origin) {
    sqlite3_stmt *pStmt {nullptr};
    if(sqlite3_prepare_v2(sqlite_store, SELECT_PEER_WITH_NODEID.c_str(), strlen(SELECT_PEER_WITH_NODEID.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");
    }

    const uint64_t when = currentTimeMillis() - Constants::MAX_VALUE_AGE;
    sqlite3_bind_blob(pStmt, 1, peerId.data(), peerId.size(), SQLITE_STATIC);
    sqlite3_bind_blob(pStmt, 2, origin.data(), origin.size(), SQLITE_STATIC);
    sqlite3_bind_int64(pStmt, 3, when);

    if (sqlite3_step(pStmt) == SQLITE_ROW) {
        Blob privateKey {};
        Id nodeId {};
        Id origin {};
        uint16_t port {0};
        std::string alt {};
        std::vector<uint8_t>  signature {};

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

			if (std::strcmp(name, "privateKey") == 0 && len > 0) {
                privateKey = Blob(ptr, len);
            } else if (std::strcmp(name, "nodeId") == 0 && len > 0) {
                nodeId = Id(Blob(ptr, len));
            } else if (std::strcmp(name, "port") == 0) {
                port = sqlite3_column_int(pStmt, i);
            } else if (std::strcmp(name, "alternativeURL") == 0) {
                alt = (char *)sqlite3_column_text(pStmt, i);
            } else if (std::strcmp(name, "signature") == 0) {
                signature = {Blob(ptr, len)};
            }
        }

        sqlite3_finalize(pStmt);
        auto peer = PeerInfo::of(peerId, privateKey, nodeId, origin, port, alt, signature);
        return std::make_shared<PeerInfo>(peer);
    }

    sqlite3_finalize(pStmt);
    return nullptr;
}

void SqliteStorage::putPeer(const std::list<PeerInfo>& peers) {
    if (sqlite3_exec(sqlite_store, "BEGIN", 0, 0, 0) != 0)
        throw std::runtime_error("Open auto commit mode failed.");

    sqlite3_stmt *pStmt {nullptr};
    if(sqlite3_prepare_v2(sqlite_store, UPSERT_PEER.c_str(), strlen(UPSERT_PEER.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");
    }

    uint64_t now = currentTimeMillis();
    for (const auto& peer : peers) {
        sqlite3_bind_blob(pStmt, 1, peer.getId().data(), peer.getId().size(), SQLITE_STATIC);
        sqlite3_bind_int(pStmt, 2, peer.getPrivateKey());
        sqlite3_bind_blob(pStmt, 3, peer.getNodeId().data(), peer.getNodeId().size(), SQLITE_STATIC);
        sqlite3_bind_blob(pStmt, 4, peer.getOrigin().data(), peer.getOrigin().size(), SQLITE_STATIC);
        sqlite3_bind_int(pStmt, 5, peer.getPort());
        const char* alt = peer.getAlternativeURL().c_str();
        sqlite3_bind_text(pStmt, 6, alt, strlen(alt), NULL);
        sqlite3_bind_blob(pStmt, 7, peer.getSignature().data(), peer.getSignature().size(), SQLITE_STATIC);
        sqlite3_bind_int64(pStmt, 8, now);

        if (sqlite3_step(pStmt) != SQLITE_DONE) {
            sqlite3_finalize(pStmt);
            throw std::runtime_error("Step sqlite failed.");
        }

        sqlite3_reset(pStmt);
    }

    sqlite3_finalize(pStmt);
    sqlite3_exec(sqlite_store, "COMMIT", 0, 0, 0);
}

void SqliteStorage::putPeer(const PeerInfo& peer) {
    putPeer(std::list<PeerInfo>{peer});
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
            size_t len = 0;
            const void *ptr = NULL;

            if (cType == SQLITE_BLOB) {
                len = sqlite3_column_bytes(pStmt, i);
                ptr = sqlite3_column_blob(pStmt, i);

                auto id = Id(Blob{ptr, len});
                ids.emplace_back(id);
            }
        }
    }

    sqlite3_finalize(pStmt);
    return ids;
}

}
}
