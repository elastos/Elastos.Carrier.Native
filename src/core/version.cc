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

#include <vector>
#include <cstring>
#include <algorithm>
#include <map>

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "(unknown version)"
#endif

#include "carrier/version.h"

namespace elastos {
namespace carrier {

const char* version() {
    return PACKAGE_VERSION;
}

std::map<std::string, std::string> names {{"MK", "Meerkat"}};

int Version::build(std::string& name, int version) {
    std::transform(name.cbegin(), name.cend(), name.begin(), // write to the same location
        [](unsigned char c) { return std::toupper(c); });

    std::vector<uint8_t> nameBytes;
    nameBytes.reserve(name.size());
    std::memcpy(nameBytes.data(), name.data(), name.size());

    return (int)nameBytes[0] << 24 | (int)nameBytes[1] << 16 |
            (version & 0x0000FFFF);
}

const std::string Version::toString(int version) {
    if (version == 0)
        return "N/A";

    char code[3];
    code[0] = version >> 24;
    code[1] = (version & 0x00ff0000) >> 16;
    code[2] = 0;

    std::string n = std::string(code);
    std::string v = std::to_string(version & 0x0000ffff);

    auto result = names.find(n);
    return result != names.end() ? (result->second + "/" + v) : (n + "/" + v);
}

}
}
