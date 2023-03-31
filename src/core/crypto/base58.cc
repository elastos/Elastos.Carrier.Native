// Copyright (c) 2014-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>
#include <cassert>

#include "base58.h"

/** All alphanumeric characters except for "0", "I", "O", and "l" */
static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const int8_t mapBase58[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
    -1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
    22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
    -1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
    47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
};

inline std::vector<uint8_t> sub(std::vector<uint8_t> &input, std::vector<uint8_t>::size_type offset)
{
    return std::vector<uint8_t>(input.begin() + offset, input.end());
}

constexpr inline bool is_space(char c) noexcept {
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

/**
 * Check if a string does not contain any embedded NUL (\0) characters
 */
[[nodiscard]] inline bool valid_as_cstring(const std::string& str) noexcept
{
    return str.size() == strlen(str.c_str());
}

std::vector<uint8_t> base58_decode(const char* psz)
{
    // Skip leading spaces.
    while (*psz && is_space(*psz))
        psz++;
    // Skip and count leading '1's.
    int zeroes = 0;
    int length = 0;
    while (*psz == '1') {
        zeroes++;
        psz++;
    }
    // Allocate enough space in big-endian base256 representation.
    int size = strlen(psz) * 733 /1000 + 1; // log(58) / log(256), rounded up.
    std::vector<uint8_t> b256(size);
    // Process the characters.
    static_assert(sizeof(mapBase58) == 256, "mapBase58.size() should be 256"); // guarantee not out of range
    while (*psz && !is_space(*psz)) {
        // Decode base58 character
        int carry = mapBase58[(uint8_t)*psz];
        if (carry == -1)  // Invalid b58 character
            return {} ;
        int i = 0;
        for (std::vector<uint8_t>::reverse_iterator it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        assert(carry == 0);
        length = i;
        psz++;
    }
    // Skip trailing spaces.
    while (is_space(*psz))
        psz++;
    if (*psz != 0)
        return {};
    // Skip leading zeroes in b256.
    std::vector<uint8_t>::iterator it = b256.begin() + (size - length);
    // Copy result into output vector.
    std::vector<uint8_t> vch;
    vch.reserve(zeroes + (b256.end() - it));
    vch.assign(zeroes, 0x00);
    while (it != b256.end())
        vch.push_back(*(it++));
    return vch;
}

std::string base58_encode(std::vector<uint8_t> &input)
{
    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    while (input.size() > 0 && input[0] == 0) {
        input = sub(input, 1);
        zeroes++;
    }
    // Allocate enough space in big-endian base58 representation.
    int size = input.size() * 138 / 100 + 1; // log(256) / log(58), rounded up.
    std::vector<uint8_t> b58(size);
    // Process the bytes.
    while (input.size() > 0) {
        int carry = input[0];
        int i = 0;
        // Apply "b58 = b58 * 256 + ch".
        for (std::vector<uint8_t>::reverse_iterator it = b58.rbegin(); (carry != 0 || i < length) && (it != b58.rend()); it++, i++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }

        assert(carry == 0);
        length = i;
        input = sub(input, 1);
    }
    // Skip leading zeroes in base58 result.
    std::vector<uint8_t>::iterator it = b58.begin() + (size - length);
    while (it != b58.end() && *it == 0)
        it++;
    // Translate the result into a string.
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, '1');
    while (it != b58.end())
        str += pszBase58[*(it++)];
    return str;
}

std::vector<uint8_t> base58_decode(const std::string& str)
{
    if (!valid_as_cstring(str)) {
        return {};
    }
    return base58_decode(str.c_str());
}
