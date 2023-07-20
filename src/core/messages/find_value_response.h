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

#include "carrier/value.h"
#include "carrier/crypto_core.h"

#include "lookup_response.h"
#include "message.h"

namespace elastos {
namespace carrier {

class FindValueResponse : public LookupResponse {
public:
    FindValueResponse(int txid)
        : LookupResponse(Message::Method::FIND_VALUE, txid) {}

    FindValueResponse()
        : FindValueResponse(0) {}

    void setValue(const Value& value);

    bool hasValue() {
		return !value.empty();
	}

    Value getValue() const;

    int estimateSize() const override {
        return LookupResponse::estimateSize() + 195 + value.size();
    }

protected:
    void _serialize(nlohmann::json& object) const override;
    void _parse(const std::string& field, nlohmann::json& object) override;
    void _toString(std::stringstream& ss) const override;

private:
    Id publicKey {};
    Id recipient {};
    CryptoBox::Nonce nonce {};
    int sequenceNumber {-1};
	std::vector<uint8_t> signature {};
	std::vector<uint8_t> value;
};

}
}
