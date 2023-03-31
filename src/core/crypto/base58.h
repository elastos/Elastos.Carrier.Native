// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Why base-58 instead of standard base-64 encoding?
 * - Don't want 0OIl characters that look the same in some fonts and
 *      could be used to create visually identical looking data.
 * - A string with non-alphanumeric characters is not as easily accepted as input.
 * - E-mail usually won't line-break if there's no punctuation to break at.
 * - Double-clicking selects the whole string as one word if it's all alphanumeric.
 */
#ifndef BASE58_H
#define BASE58_H

#include <string>
#include <vector>

/**
 * Encode a byte span as a base58-encoded string
 */
std::string base58_encode(std::vector<uint8_t> &input);

std::string inline base58_encode(uint8_t *input, size_t size) {
    std::vector<uint8_t> d{input, input + size};
    return base58_encode(d);
}

/**
 * Decode a base58-encoded string (str) into a byte vector (vchRet).
 */
std::vector<uint8_t> base58_decode(const std::string& str);

std::vector<uint8_t> base58_decode(const char* str);
#endif // BASE58_H
