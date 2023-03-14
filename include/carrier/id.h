/*
 * Copyright (c) 2022 - 2023 Elastos Foundation
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

#include <array>
#include <vector>
#include <stdexcept>

#include "def.h"
#include "types.h"
#include "blob.h"
#include "crypto_core.h"

#define ID_BYTES   32u
#define ID_BITS    256u

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC Id {
public:
    static Id MIN_ID;
    static Id MAX_ID;

    Id() noexcept = default;
    Id(const Id& id) = default;

    Id(const uint8_t* ptr, size_t len) {
        if (len != ID_BYTES || !ptr)
            throw std::invalid_argument("Binary id should be " + std::to_string(ID_BYTES) + " bytes long.");

        std::memcpy(bytes.data(), ptr, len);
    }

    Id(const Signature::PublicKey& key)
        : Id(key.bytes(), key.size()) {}

    explicit Id(const std::vector<uint8_t>& vector)
        : Id(vector.data(), vector.size()) {}

    explicit Id(const std::string& id) {
        id.find("0x") == 0 ? fromHexString(id) : fromBase58String(id);
    }

    constexpr const uint8_t* data() const noexcept {
        return bytes.data();
    }

    constexpr size_t size() const noexcept {
        return ID_BYTES;
    }

    constexpr const uint8_t* cbegin() const noexcept {
        return bytes.data();
    }

    constexpr const uint8_t* cend() const noexcept {
        return bytes.data() + ID_BYTES;
    }

    Blob asBlob() const noexcept {
        return static_cast<bool>(*this) ? Blob(data(), size()) : Blob();
    }

    Sp<Signature::PublicKey> toKey() const {
        return std::make_shared<Signature::PublicKey>(bytes.data(), ID_BYTES);
    }

    static Id ofHex(const std::string& hexId);
    static Id ofBase58(const std::string& base58Id);
    static Id random();
    static Id zero() {
        return Id();
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
        std::memcpy(bytes.data(), other.data(), ID_BYTES);
        return *this;
    }

    Id& operator=(Id&& other) noexcept {
        std::memcpy(bytes.data(), other.bytes.data(), ID_BYTES);
        std::memset(other.bytes.data(), 0, ID_BYTES);
        return *this;
    }

    operator std::string() const { return toBase58String(); }
    operator bool() const {
        return !std::all_of(bytes.cbegin(), bytes.cend(), [](uint8_t i){ return !i; });
    }

    friend std::ostream& operator<< (std::ostream& os, const Id& id);

    friend void to_json(nlohmann::json& json, const Id& id);
    friend void from_json(const nlohmann::json& json, Id& id);

protected:
    std::array<uint8_t, ID_BYTES>& getData() { return bytes; }

private:
    void fromBase58String(const std::string&);
    void fromHexString(const std::string&);

    // Counts the number of leading 0's in this Id
    int getLeadingZeros();

    std::array<uint8_t, ID_BYTES> bytes {0};
};

} // namespace carrier
} // namespace elastos
