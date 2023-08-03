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
#include "crypto/hex.h"
#include "constants.h"
#include "scheduler.h"
#include "sqlite_storage.h"

namespace elastos {
namespace carrier {

static int VERSION = 4;
static std::string SET_USER_VERSION = "PRAGMA user_version = " + std::to_string(VERSION);
static std::string GET_USER_VERSION = "PRAGMA user_version";

static std::string CREATE_VALUES_TABLE = "CREATE TABLE IF NOT EXISTS valores(\
        id BLOB NOT NULL PRIMARY KEY, \
        persistent BOOLEAN NOT NULL DEFAULT FALSE, \
        publicKey BLOB, \
        privateKey BLOB, \
        recipient BLOB, \
        nonce BLOB, \
        signature BLOB, \
        sequenceNumber INTEGER, \
        data BLOB, \
        timestamp INTEGER NOT NULL, \
        announced INTEGER NOT NULL DEFAULT 0\
        ) WITHOUT ROWID";

static std::string CREATE_VALUES_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_valores_timpstamp ON valores(timestamp)";

static std::string CREATE_PEERS_TABLE = "CREATE TABLE IF NOT EXISTS peers( \
        id BLOB NOT NULL, \
        nodeId BLOB NOT NULL, \
        origin BLOB NOT NULL, \
        persistent BOOLEAN NOT NULL DEFAULT FALSE, \
        privateKey BLOB, \
        port INTEGER NOT NULL, \
        alternativeURL VARCHAR(512), \
        signature BLOB NOT NULL, \
        timestamp INTEGER NOT NULL, \
        announced INTEGER NOT NULL DEFAULT 0, \
        PRIMARY KEY(id, nodeId, origin)\
        ) WITHOUT ROWID";


static std::string CREATE_PEERS_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_peers_timpstamp ON peers (timestamp)";

static std::string CREATE_PEERS_ID_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_peers_id ON peers(id)";

static std::string UPSERT_VALUE = "INSERT INTO valores(\
        id, persistent, publicKey, privateKey, recipient, nonce, signature, sequenceNumber, data, timestamp, announced) \
        VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) ON CONFLICT(id) DO UPDATE SET \
        publicKey=excluded.publicKey, privateKey=excluded.privateKey, \
        recipient=excluded.recipient, nonce=excluded.nonce, \
        signature=excluded.signature, sequenceNumber=excluded.sequenceNumber, \
        data=excluded.data, timestamp=excluded.timestamp";

static std::string SELECT_VALUE = "SELECT * from valores \
        WHERE id = ? and timestamp >= ?";

static std::string UPDATE_VALUE_LAST_ANNOUNCE = "UPDATE valores \
        SET timestamp=?, announced = ? WHERE id = ?";

static std::string GET_VALUES = "SELECT id from valores WHERE timestamp >= ? ORDER BY id";

static std::string GET_PERSISTENT_VALUES = "SELECT * FROM valores WHERE persistent = true AND announced <= ?";

static std::string REMOVE_VALUE = "DELETE FROM valores WHERE id = ?";

static std::string UPSERT_PEER = "INSERT INTO peers(\
        id, nodeId, origin, persistent, privateKey, port, alternativeURL, signature, timestamp, announced) \
        VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?) ON CONFLICT(id, nodeId, origin) DO UPDATE SET \
        persistent=excluded.persistent, privateKey=excluded.privateKey, \
        port=excluded.port, alternativeURL=excluded.alternativeURL, \
        signature=excluded.signature, timestamp=excluded.timestamp, \
		announced=excluded.announced";

static std::string SELECT_PEER = "SELECT * from peers \
        WHERE id = ? and timestamp >= ? \
        ORDER BY RANDOM() LIMIT ?";

static std::string SELECT_PEER_WITH_SRC = "SELECT * from peers \
        WHERE id = ? and origin = ? and timestamp >= ?";

static std::string UPDATE_PEER_LAST_ANNOUNCE = "UPDATE peers \
        SET timestamp=?, announced = ? WHERE id = ? and origin = ?";

static std::string GET_PEERS = "SELECT DISTINCT id from peers WHERE timestamp >= ? ORDER BY id";

static std::string GET_PERSISTENT_PEERS = "SELECT * FROM peers WHERE persistent = true AND announced <= ?";

static std::string REMOVE_PEER = "DELETE FROM peers WHERE id = ? and origin = ?";

SqliteStorage::~SqliteStorage() {
    if (sqlite_store) {
        sqlite3_close(sqlite_store);
        sqlite_store = NULL;
    }
}

void SqliteStorage::expire() {
    const char *sqls[2] = { "DELETE FROM valores WHERE persistent != TRUE and timestamp < ?",  "DELETE FROM peers WHERE persistent != TRUE and timestamp < ?"};
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
    if (userVersion < 4) {
        if (sqlite3_exec(sqlite_store, "DROP INDEX IF EXISTS idx_valores_timpstamp", 0, 0, 0) != 0
            || sqlite3_exec(sqlite_store, "DROP TABLE IF EXISTS valores", 0, 0, 0) != 0
            || sqlite3_exec(sqlite_store, "DROP INDEX IF EXISTS idx_peers_timpstamp", 0, 0, 0) != 0
            || sqlite3_exec(sqlite_store, "DROP INDEX IF EXISTS idx_peers_id", 0, 0, 0) != 0
            || sqlite3_exec(sqlite_store, "DROP INDEX IF EXISTS peers", 0, 0, 0) != 0) {

            throw std::runtime_error("Failed to update tables.");
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
        Blob publicKey {};
        Blob privateKey {};
        Blob recipient {};
        Blob nonce {};
        Blob signature {};
        Blob data {};
        int sequenceNumber {0};

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
                publicKey = Blob(ptr, len);
            } else if (std::strcmp(name, "privateKey") == 0 && len > 0) {
                privateKey = Blob(ptr, len);
            } else if (strcmp(name, "recipient") == 0 && len > 0) {
                recipient = Blob(ptr, len);
            } else if (strcmp(name, "nonce") == 0 && len > 0 && len == CryptoBox::Nonce::BYTES) {
                nonce = Blob(ptr, len);
            } else if (strcmp(name, "signature") == 0 && len > 0) {
                signature = Blob(ptr, len);
            } else if (strcmp(name, "sequenceNumber") == 0 && cType == SQLITE_INTEGER) {
                sequenceNumber = sqlite3_column_int(pStmt, i);
            } else if (strcmp(name, "data") == 0 && len > 0) {
                data = Blob(ptr, len);
            }
        }

        auto value = Value::of(publicKey, privateKey, recipient, nonce, sequenceNumber, signature, data);
        sqlite3_finalize(pStmt);
        return std::make_shared<Value>(value);
    }

    sqlite3_finalize(pStmt);
    return nullptr;
}

Sp<Value> SqliteStorage::putValue(const Value& value, int expectedSeq, bool persistent, bool updateLastAnnounce) {
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
    sqlite3_bind_int(pStmt, 2, persistent);

    if (value.isMutable()) {
        auto signer = value.getPublicKey().blob();
        sqlite3_bind_blob(pStmt, 3, signer.ptr(), signer.size(), SQLITE_STATIC);
    } else {
        sqlite3_bind_null(pStmt, 3);
    }

    if (value.hasPrivateKey()) {
        auto sk = value.getPrivateKey().blob();
        sqlite3_bind_blob(pStmt, 4, sk.ptr(), sk.size(), SQLITE_STATIC);
    } else {
        sqlite3_bind_null(pStmt, 4);
    }

    if (value.isEncrypted()) {
        auto recipient = value.getRecipient().blob();
        sqlite3_bind_blob(pStmt, 5, recipient.ptr(), recipient.size(), SQLITE_STATIC);
    } else {
        sqlite3_bind_null(pStmt, 5);
    }

    if (value.isMutable()) {
        auto nonce = value.getNonce().blob();
        sqlite3_bind_blob(pStmt, 6, nonce.ptr(), nonce.size(), SQLITE_STATIC);
    } else {
        sqlite3_bind_null(pStmt, 6);
    }

    if (value.isSigned()) {
        auto sig = Blob(value.getSignature());
        sqlite3_bind_blob(pStmt, 7, sig.ptr(), sig.size(), SQLITE_STATIC);
    } else {
        sqlite3_bind_null(pStmt, 7);
    }

    sqlite3_bind_int(pStmt, 8, value.getSequenceNumber());

    auto data = Blob(value.getData());
    sqlite3_bind_blob(pStmt, 9, data.ptr(), data.size(), SQLITE_STATIC);

    auto now = currentTimeMillis();
    sqlite3_bind_int64(pStmt, 10, currentTimeMillis());
    sqlite3_bind_int64(pStmt, 11, updateLastAnnounce ? now : 0);

    sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    return old;
}

std::list<Id> SqliteStorage::getAllValues() {
    std::list<Id> ids {};

    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, GET_VALUES.c_str(), strlen(GET_VALUES.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    const uint64_t when = currentTimeMillis() - Constants::MAX_VALUE_AGE;
    sqlite3_bind_int64(pStmt, 1, when);

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

void SqliteStorage::updateValueLastAnnounce(const Id& valueId) {
    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, UPDATE_VALUE_LAST_ANNOUNCE.c_str(), strlen(UPDATE_VALUE_LAST_ANNOUNCE.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    auto now = currentTimeMillis();
    sqlite3_bind_int64(pStmt, 1, now);
    sqlite3_bind_int64(pStmt, 2, now);
    sqlite3_bind_blob(pStmt, 3, valueId.data(), valueId.size(), SQLITE_STATIC);

    sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
}

std::list<Value> SqliteStorage::getPersistentValues(uint64_t lastAnnounceBefore) {
    std::list<Value> values {};
    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, GET_PERSISTENT_VALUES.c_str(), strlen(GET_PERSISTENT_VALUES.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    sqlite3_bind_int64(pStmt, 1, lastAnnounceBefore);

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        Blob publicKey {};
        Blob privateKey {};
        Blob recipient {};
        Blob nonce {};
        Blob  signature {};
        Blob data {};
        int sequenceNumber {0};

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
                publicKey = Blob(ptr, len);
            } else if (std::strcmp(name, "privateKey") == 0 && len > 0) {
                privateKey = Blob(ptr, len);
            } else if (strcmp(name, "recipient") == 0 && len > 0) {
                recipient = Blob(ptr, len);
            } else if (strcmp(name, "nonce") == 0 && len > 0 && len == CryptoBox::Nonce::BYTES) {
                nonce = Blob(ptr, len);
            } else if (strcmp(name, "signature") == 0 && len > 0) {
                signature = Blob(ptr, len);
            } else if (strcmp(name, "sequenceNumber") == 0 && cType == SQLITE_INTEGER) {
                sequenceNumber = sqlite3_column_int(pStmt, i);
            } else if (strcmp(name, "data") == 0 && len > 0) {
                data = Blob(ptr, len);
            }
        }

        auto value = Value::of(publicKey, privateKey, recipient, nonce, sequenceNumber, signature, data);
        values.emplace_back(value);
    }

    sqlite3_finalize(pStmt);
    return values;
}

bool SqliteStorage::removeValue(const Id& valueId) {
    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, REMOVE_VALUE.c_str(), strlen(REMOVE_VALUE.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    sqlite3_bind_blob(pStmt, 1, valueId.data(), valueId.size(), SQLITE_STATIC);

    bool ret = false;
    if (sqlite3_step(pStmt) == SQLITE_DONE) {
        if (sqlite3_changes(sqlite_store) > 0)
            ret = true;
    }

    sqlite3_finalize(pStmt);
    return ret;
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

    uint64_t when = currentTimeMillis() - Constants::MAX_PEER_AGE;
    sqlite3_bind_blob(pStmt, 1, peerId.data(), peerId.size(), SQLITE_STATIC);
    sqlite3_bind_int64(pStmt, 2, when);
    sqlite3_bind_int(pStmt, 3, maxPeers);

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        Blob privateKey {};
        Blob nodeId {};
        Blob origin {};
        uint16_t port {0};
        std::string alt {};
        Blob signature {};

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
                nodeId = Blob(ptr, len);
            } else if (std::strcmp(name, "origin") == 0 && len > 0) {
                origin = Blob(ptr, len);
            } else if (std::strcmp(name, "port") == 0) {
                port = sqlite3_column_int(pStmt, i);
            } else if (std::strcmp(name, "alternativeURL") == 0) {
                auto c = (char *)sqlite3_column_text(pStmt, i);
                alt = c ? c : "";
            } else if (std::strcmp(name, "signature") == 0) {
                signature = Blob(ptr, len);
            }
        }

        PeerInfo peer = PeerInfo::of(peerId.blob(), privateKey, nodeId, origin, port, alt, signature);
        peers.push_back(peer);
    }
    sqlite3_finalize(pStmt);

    return peers;
}

Sp<PeerInfo> SqliteStorage::getPeer(const Id& peerId, const Id& origin) {
    sqlite3_stmt *pStmt {nullptr};
    if(sqlite3_prepare_v2(sqlite_store, SELECT_PEER_WITH_SRC.c_str(), strlen(SELECT_PEER_WITH_SRC.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");
    }

    const uint64_t when = currentTimeMillis() - Constants::MAX_PEER_AGE;
    sqlite3_bind_blob(pStmt, 1, peerId.data(), peerId.size(), SQLITE_STATIC);
    sqlite3_bind_blob(pStmt, 2, origin.data(), origin.size(), SQLITE_STATIC);
    sqlite3_bind_int64(pStmt, 3, when);

    if (sqlite3_step(pStmt) == SQLITE_ROW) {
        Blob privateKey {};
        Blob nodeId {};
        Blob origin {};
        uint16_t port {0};
        std::string alt {};
        Blob  signature {};

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
                nodeId = Blob(ptr, len);
            } else if (std::strcmp(name, "origin") == 0 && len > 0) {
                origin = Blob(ptr, len);
            } else if (std::strcmp(name, "port") == 0) {
                port = sqlite3_column_int(pStmt, i);
            } else if (std::strcmp(name, "alternativeURL") == 0) {
                auto c = (char *)sqlite3_column_text(pStmt, i);
                alt = c ? c : "";
            } else if (std::strcmp(name, "signature") == 0) {
                signature = Blob(ptr, len);
            }
        }

        auto peer = PeerInfo::of(peerId.blob(), privateKey, nodeId, origin, port, alt, signature);
        sqlite3_finalize(pStmt);
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
        sqlite3_bind_blob(pStmt, 2, peer.getNodeId().data(), peer.getNodeId().size(), SQLITE_STATIC);
        sqlite3_bind_blob(pStmt, 3, peer.getOrigin().data(), peer.getOrigin().size(), SQLITE_STATIC);
        sqlite3_bind_int(pStmt, 4, false);
        if (peer.hasPrivateKey())
            sqlite3_bind_blob(pStmt, 5, peer.getPrivateKey().bytes(), peer.getPrivateKey().size(), SQLITE_STATIC);
        else
            sqlite3_bind_null(pStmt, 5);
        sqlite3_bind_int(pStmt, 6, peer.getPort());
        if (peer.hasAlternativeURL()) {
            const char* alt = peer.getAlternativeURL().c_str();
            sqlite3_bind_text(pStmt, 7, alt, strlen(alt), SQLITE_STATIC);
        } else {
            sqlite3_bind_null(pStmt, 7);
        }
        sqlite3_bind_blob(pStmt, 8, peer.getSignature().data(), peer.getSignature().size(), SQLITE_STATIC);
        sqlite3_bind_int64(pStmt, 9, now);
        sqlite3_bind_int64(pStmt, 10, 0);

        if (sqlite3_step(pStmt) != SQLITE_DONE) {
            sqlite3_finalize(pStmt);
            throw std::runtime_error("Step sqlite failed.");
        }

        sqlite3_reset(pStmt);
    }

    sqlite3_finalize(pStmt);
    sqlite3_exec(sqlite_store, "COMMIT", 0, 0, 0);
}

void SqliteStorage::putPeer(const PeerInfo& peer, bool persistent, bool updateLastAnnounce) {
    sqlite3_stmt *pStmt {nullptr};
    if(sqlite3_prepare_v2(sqlite_store, UPSERT_PEER.c_str(), strlen(UPSERT_PEER.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");
    }

    sqlite3_bind_blob(pStmt, 1, peer.getId().data(), peer.getId().size(), SQLITE_STATIC);
    sqlite3_bind_blob(pStmt, 2, peer.getNodeId().data(), peer.getNodeId().size(), SQLITE_STATIC);
    sqlite3_bind_blob(pStmt, 3, peer.getOrigin().data(), peer.getOrigin().size(), SQLITE_STATIC);
    sqlite3_bind_int(pStmt, 4, persistent);
    if (peer.hasPrivateKey())
        sqlite3_bind_blob(pStmt, 5, peer.getPrivateKey().bytes(), peer.getPrivateKey().size(), SQLITE_STATIC);
    else
        sqlite3_bind_null(pStmt, 5);
    sqlite3_bind_int(pStmt, 6, peer.getPort());
    if (peer.hasAlternativeURL()) {
        const char *alt = peer.getAlternativeURL().c_str();
        sqlite3_bind_text(pStmt, 7, alt, strlen(alt), SQLITE_STATIC);
    } else {
        sqlite3_bind_null(pStmt, 7);
    }
    sqlite3_bind_blob(pStmt, 8, peer.getSignature().data(), peer.getSignature().size(), SQLITE_STATIC);
    auto now = currentTimeMillis();
    sqlite3_bind_int64(pStmt, 9, currentTimeMillis());
    sqlite3_bind_int64(pStmt, 10, updateLastAnnounce ? now : 0);

    sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
}

std::list<Id> SqliteStorage::getAllPeers() {
    std::list<Id> ids {};

    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, GET_PEERS.c_str(), strlen(GET_PEERS.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    uint64_t when = currentTimeMillis() - Constants::MAX_PEER_AGE;
    sqlite3_bind_int64(pStmt, 1, when);

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

void SqliteStorage::updatePeerLastAnnounce(const Id& peerId, const Id& origin) {
    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, UPDATE_PEER_LAST_ANNOUNCE.c_str(), strlen(UPDATE_PEER_LAST_ANNOUNCE.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    auto now = currentTimeMillis();
    sqlite3_bind_int64(pStmt, 1, now);
    sqlite3_bind_int64(pStmt, 2, now);
    sqlite3_bind_blob(pStmt, 3, peerId.data(), peerId.size(), SQLITE_STATIC);
    sqlite3_bind_blob(pStmt, 4, origin.data(), origin.size(), SQLITE_STATIC);

    sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
}

std::list<PeerInfo> SqliteStorage::getPersistentPeers(uint64_t lastAnnounceBefore) {
    std::list<PeerInfo> peers {};
    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, GET_PERSISTENT_PEERS.c_str(), strlen(GET_PERSISTENT_PEERS.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    sqlite3_bind_int64(pStmt, 1, lastAnnounceBefore);

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        Blob peerId {};
        Blob privateKey {};
        Blob nodeId {};
        Blob origin {};
        uint16_t port {0};
        std::string alt {};
        Blob signature {};

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

            if (std::strcmp(name, "id") == 0 && len > 0) {
                peerId = Blob(ptr, len);
            } else if (std::strcmp(name, "privateKey") == 0 && len > 0) {
                privateKey = Blob(ptr, len);
            } else if (std::strcmp(name, "nodeId") == 0 && len > 0) {
                nodeId = Blob(ptr, len);
            } else if (std::strcmp(name, "port") == 0) {
                port = sqlite3_column_int(pStmt, i);
            } else if (std::strcmp(name, "alternativeURL") == 0) {
                auto c = (char *)sqlite3_column_text(pStmt, i);
                alt = c ? c : "";
            } else if (std::strcmp(name, "signature") == 0) {
                signature = Blob(ptr, len);
            }
        }

        auto peer = PeerInfo::of(peerId, privateKey, nodeId, origin, port, alt, signature);
        peers.emplace_back(peer);
    }

    sqlite3_finalize(pStmt);
    return peers;
}

bool SqliteStorage::removePeer(const Id& peerId, const Id& origin) {
    sqlite3_stmt* pStmt {nullptr};
    if (sqlite3_prepare_v2(sqlite_store, REMOVE_PEER.c_str(), strlen(REMOVE_PEER.c_str()), &pStmt, 0) != SQLITE_OK) {
        sqlite3_finalize(pStmt);
        throw std::runtime_error("Prepare sqlite failed.");;
    }

    sqlite3_bind_blob(pStmt, 1, peerId.data(), peerId.size(), SQLITE_STATIC);
    sqlite3_bind_blob(pStmt, 2, origin.data(), origin.size(), SQLITE_STATIC);

    bool ret = false;
    if (sqlite3_step(pStmt) == SQLITE_DONE) {
        if (sqlite3_changes(sqlite_store) > 0)
            ret = true;
    }

    sqlite3_finalize(pStmt);
    return ret;
}

}
}
