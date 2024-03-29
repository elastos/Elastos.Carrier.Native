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
#include <map>
#include <any>
#include <future>

#include "node.h"

namespace elastos {
namespace carrier {

class CARRIER_PUBLIC Addon {
public:
    virtual std::future<void> initialize(Sp<Node> node, const std::map<std::string, std::any>& config) = 0;
    virtual std::future<void> deinitialize() = 0;
    virtual bool isInitialized() = 0;
};

CARRIER_PUBLIC void loadAddons(Sp<Node> node, std::map<std::string, std::any>& addons);
CARRIER_PUBLIC void unloadAddons();
CARRIER_PUBLIC std::map<std::string, std::shared_ptr<Addon>>& getAddons();

} // namespace carrier
} // namespace elastos
