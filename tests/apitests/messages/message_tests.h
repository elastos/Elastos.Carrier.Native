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

#include "utils/hex.h"
#include "messages/message.h"

using namespace elastos::carrier;

namespace test {

class MessageTests {
public:
	int VERSION = 0x68690001;
    std::string VERSION_STR = "hi/1";

protected:
    void printMessage(Message msg, std::vector<uint8_t> bin) {
		std::cout << "======== " << msg.getTypeString() << ":" << msg.getMethodString() << std::endl;
		std::cout << "String: " << static_cast<std::string>(msg);
		std::cout << "   Hex: " << bin.size() << Utils::PATH_SEP << msg.estimateSize() << " : " << Hex::encode(bin);
        nlohmann::json j = nlohmann::json::from_cbor(bin);
		std::cout << "  JSON: " << j.dump();
	}
};

}  // namespace test
