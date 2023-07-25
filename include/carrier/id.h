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

#include "def.h"
#include "blob.h"
#include "types.h"
#include "signature.h"
#include "crypto_box.h"

#define ID_BYTES   32u
#define ID_BITS    256u

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC Id {
public:
    static const uint32_t BYTES { 32 };

    static Id MIN_ID;
    static Id MAX_ID;

    Id() {
        bytes = new uint8_t[ID_BYTES];
        std::memset(bytes, 0, ID_BYTES);
    }

    Id(const Id& id) {
        bytes = new uint8_t[ID_BYTES];
        std::memcpy(bytes, id.bytes, ID_BYTES);
    }

    Id(Id&& id) noexcept {
        bytes = id.bytes;
        id.bytes = nullptr;
    };

    ~Id() {
        std::memset(bytes, 0, ID_BYTES);
        if (bytes)
            delete [] bytes;
    }

    Id(const Blob& id);

    explicit Id(const std::string& id) {
        id.find("0x") == 0 ? fromHexString(id) : fromBase58String(id);
    }

    constexpr const uint8_t* data() const noexcept {
        return const_cast<uint8_t*>(bytes);
    }

    constexpr size_t size() const noexcept {
        return ID_BYTES;
    }

    constexpr const uint8_t* cbegin() const noexcept {
        return const_cast<uint8_t*>(bytes);
    }

    constexpr const uint8_t* cend() const noexcept {
        return const_cast<uint8_t*>(bytes) + ID_BYTES;
    }

    const Blob blob() const noexcept {
        return Blob(bytes, ID_BYTES);
    }

    Sp<Signature::PublicKey> toKey() const {
        return std::make_shared<Signature::PublicKey>(blob());
    }

    static Id ofHex(const std::string& hexId);
    static Id ofBase58(const std::string& base58Id);
    static Id ofName(std::string name);
    static Id random();
    static Id zero() {
        return Id();
    }

    Signature::PublicKey toSignatureKey() const {
        return Signature::PublicKey(Blob(bytes, ID_BYTES));
    }

    CryptoBox::PublicKey toEncryptionKey() const {
        return CryptoBox::PublicKey::fromSignatureKey(toSignatureKey());
    }

    /**
     * Checks the distance between this and another Id
     *
     * @param other another to
     *
     * @return The distance of this NodeId from the given NodeId
     */
    Id distance(const Id& other) const;

    static Id distance(const Id& id1, const Id& id2);

    /**
     * Compares the distance of two keys relative to this using the XOR metric
     *
     * @return -1 if k1 is closer to this key, 0 if k1 and k2 are equal distant, 1 if
     *         k2 is closer
     */
    int threeWayCompare(const Id& id1, const Id& id2) const;

    static bool bitsEqual(const Id& id1, const Id& id2, int bits);
    static void bitsCopy(const Id& src, Id& dest, int depth);

    int compareTo(const Id& other) const {
        return std::memcmp(data(), other.data(), ID_BYTES);
    }

    const std::string toHexString() const;
    const std::string toBase58String() const;
    const std::string toBinaryString() const;

    bool operator==(const Id& other) const { return compareTo(other) == 0; }
    bool operator!=(const Id& other) const { return compareTo(other) != 0; }
    bool operator<(const Id& other) const;

    Id& operator=(const Id& other) noexcept {
        std::memcpy(bytes, other.bytes, ID_BYTES);
        return *this;
    }

    Id& operator=(Id&& other) noexcept {
        delete [] bytes;
        bytes = other.bytes;
        other.bytes = nullptr;
        return *this;
    }

    operator std::string() const {
        return toBase58String();
    }

    operator bool() const {
        return compareTo(MIN_ID) != 0;
    }

    friend std::ostream& operator<< (std::ostream& os, const Id& id);

protected:
    uint8_t* getData() { return bytes; }

private:
    void fromBase58String(const std::string&);
    void fromHexString(const std::string&);

    // Counts the number of leading 0's in this Id
    int getLeadingZeros();

    uint8_t *bytes { nullptr };
};

} // namespace carrier
} // namespace elastos
