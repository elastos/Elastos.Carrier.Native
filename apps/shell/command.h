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
#include <memory>
#include <cstdlib>

#include <CLI/CLI.hpp>
#include <carrier.h>

using namespace elastos::carrier;
using namespace CLI;

template<class T>
struct null_deleter
{
    void operator() (T*) const {}
};

class Command : protected App, public std::enable_shared_from_this<App> {
public:
    Command(std::string name, std::string description) : App(description, name) {};

    Command(std::string description = "") : Command("", description) {};

    virtual ~Command() = default;

    void addSubCommand(Command& command) {
        command.prepare();
        this->add_subcommand(command.shared_from_this());
    };

    void prepare() {
        if (prepaired)
            return;

        setupOptions();

        this->callback([this]() {
            this->execute();
        });

        prepaired = true;
    }

    void process(int argc, char* argv[]) {
        prepare();

        try {
            parse(argc, argv);
        } catch(const CLI::Error &e) {
            int rc = this->exit(e);
            if (!this->get_parent())
                std::exit(rc);
        }
    };

    void process(std::string cmdline) {
        prepare();

        try {
            this->parse(cmdline, false);
        } catch(const CLI::Error &e) {
            this->exit(e);
        }
    };

    std::shared_ptr<App> shared_from_this() {
        try {
            return std::enable_shared_from_this<App>::shared_from_this();
        } catch (std::bad_weak_ptr &) {
            return std::shared_ptr<App>(dynamic_cast<App*>(this), null_deleter<App>());
        }
    }

    std::shared_ptr<const App> shared_from_this() const {
        try {
            return std::enable_shared_from_this<App>::shared_from_this();
        } catch (std::bad_weak_ptr &) {
            return std::shared_ptr<const App>(dynamic_cast<const App*>(this), null_deleter<const App>());
        }
    }

protected:
    virtual void setupOptions() {};
    virtual void execute() {};

    static std::shared_ptr<Node> node;

/*
    std::shared_ptr<App> getApp() {
        return shared_from_this();
    }
*/
private:
    bool prepaired {false};
};
