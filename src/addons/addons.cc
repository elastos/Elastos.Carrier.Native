/*
 *  Copyright (c) 2022 - 2023 trinity-tech.io
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

#include "activeproxy.h"
#include <iostream>

namespace elastos {
namespace carrier {

std::map<std::string, std::shared_ptr<Addon>> g_addons {};

bool loadAddons(Sp<Node> node, std::map<std::string, std::any>& addons) {
    if (addons.empty())
        return true;

    for (auto& [name, value] : addons) {
        if (value.type() != typeid(std::map<std::string, std::any>)) {
            std::cout << "Addon '" << name << "': invalid configure! " << std::endl;
            return false;
        }

        std::shared_ptr<Addon> addon = nullptr;
        if (name == "ActiveProxy") {
            addon = std::make_shared<activeproxy::ActiveProxy>();
        }

        if (addon) {
            auto configure = std::any_cast<std::map<std::string, std::any>>(value);
            g_addons[name] = addon;
            try {
                auto future = addon->initialize(node, configure);
                future.get();
            } catch(...) {
                return false;
            }
        }
    }

    return true;
}

void unloadAddons() {
    auto it = g_addons.begin();
    while (it != g_addons.end()) {
        auto addon = it->second;
        it = g_addons.erase(it);
        if (addon->isInitialized()) {
            auto future = addon->deinitialize();
            future.get();
        }
    }
}

std::map<std::string, std::shared_ptr<Addon>>& getAddons() {
    return g_addons;
}

} // namespace carrier
} // namespace elastos
