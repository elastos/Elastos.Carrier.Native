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

#include <sstream>
#include "carrier/node_info.h"
#include "carrier/version.h"

namespace elastos {
namespace carrier {

std::string NodeInfo::toString() const {
    std::stringstream ss;
    ss.str().reserve(80);

    ss << "<" << nodeId.toString()
        << "," << sockaddr.host()
        << "," << std::to_string(sockaddr.port())
        << ">";

    return ss.str();
}

std::ostream& operator<< (std::ostream& os, const NodeInfo& ni) {
    os << ni.toString();
    return os;
}

std::string NodeInfo::getReadableVersion() const {
    return Version::toString(version);
}

}
}
