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

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "command.h"

class StoreValueCommand : public Command {
public:
    StoreValueCommand() : Command("storevalue", "Store a value to the DHT.") {};

protected:
    void setupOptions() override {
        add_flag("-p, --persistent", persistent, "Persistent value, default is false.");
        add_flag("-m, --mutable", bMutable, "Mutable value, default is immutable value, no effect on update mode.");
        add_option("-u, --update-value", target, "Existing value id to be update.");
        add_option("-r, --recipient", recipient, "The recipient id, no effect on immutable values or update mode");
        add_option("VALUES", text, "The value text.");
        require_option(1, 5);
    };

    void execute() override {
        Value value {};

        if (!recipient.empty())
            bMutable = true;

        std::vector<uint8_t> data(text.begin(), text.end());
        if (target.empty()) {
            if (bMutable) {
                if (recipient.empty()) {
                    value = Value::createSignedValue(data);
                } else {
                    auto recipientId = Id(recipient);
                    value = Value::createEncryptedValue(recipientId, data);
                }
            } else {
                value = Value::createValue(data);
            }
        } else {
            auto id = Id(target);
            value = *(node->getValue(id));
            value = value.update(data);
        }

        auto future = node->storeValue(value, persistent);
        future.get();
        std::cout << "Value " << value.getId().toBase58String() << " stored." << std::endl;
    };

private:
    bool persistent {false};
    bool bMutable {false};
    std::string recipient {};
    std::string target {};
    std::string text {};
};
