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

#include "find_value_request.h"

namespace elastos {
namespace carrier {

void FindValueRequest::_serialize(nlohmann::json& object) const {
    if (sequenceNumber >= 0)
        object[Message::KEY_RES_SEQ] = sequenceNumber;
}

void FindValueRequest::_parse(const std::string& fieldName, nlohmann::json& object) {
    if (fieldName != Message::KEY_RES_SEQ)
        throw std::invalid_argument(std::string("Unknown field: ") + fieldName);

    object.get_to(sequenceNumber);
}

void FindValueRequest::_toString(std::stringstream& ss) const {
    if (sequenceNumber >= 0)
        ss << ",seq:" << std::to_string(sequenceNumber);
}

} // namespace carrier
} // namespace elastos
