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

#include "carrier/id.h"
#include "utils/loading_cache.h"
#include "constants.h"
#include "crypto_context.h"

namespace elastos {
namespace carrier {

class CryptoCache : public LocadingCache<Id, CryptoContext> {
public:
    static const int EXPIRED_CHECK_INTERVAL = 60 * 1000;

    CryptoCache(CryptoBox::KeyPair _keypair)
        : keypair(_keypair), LocadingCache(Constants::KBUCKET_OLD_AND_STALE_TIME) {}

private:
    CryptoContext load(const Id& key) override {
        auto encryptedPublicKey = CryptoBox::PublicKey::fromSignatureKey(*key.toKey());
        return CryptoContext(encryptedPublicKey, keypair);
    };

    void onRemoval(const CryptoContext& val) override {
        //Don't need to close in native.
        // val.close();
    }

    CryptoBox::KeyPair keypair;
};

} // namespace carrier
} // namespace elastos
