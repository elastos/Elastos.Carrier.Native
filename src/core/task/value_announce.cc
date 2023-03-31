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

#include "carrier/id.h"
#include "carrier/value.h"
#include "messages/store_value_request.h"
#include "candidate_node.h"
#include "value_announce.h"

namespace elastos {
namespace carrier {

ValueAnnounce::ValueAnnounce(DHT* dht, const ClosestSet& closestSet, Sp<Value> _value)
        :Task(dht, "ValueAnnounce"), value(_value) {

    for (const auto& entry: closestSet.getEntries()) {
        todo.emplace_back(entry);
    }
}

void ValueAnnounce::update() {
    while (!todo.empty() && canDoRequest()) {
        auto candidateNode = todo.front();
        auto request = std::make_shared<StoreValueRequest>(value);

        request->setToken(candidateNode->getToken());
        try {
            sendCall(candidateNode, request, [=](Sp<RPCCall>&) {
                todo.pop_front();
            });
        } catch (const std::exception& e) {
            log->error("Error on sending 'storeValue' request: " + std::string(e.what()));
        }
    }
}

} /* namespace carrier */
} /* namespace elastos */
