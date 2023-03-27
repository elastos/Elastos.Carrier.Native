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

#include <CLI/CLI.hpp>
#include <carrier.h>

using namespace elastos::carrier;
using namespace CLI;

extern std::shared_ptr<Node> node;

class Command {
public:
    Command(std::string name, std::string description) {
        cliApp = std::make_shared<App>(description, name);
    };

    Command(std::string description = "") : Command("", description) {};

    virtual ~Command() = default;

    void addSubCommand(Command& command) {
        command.prepare();
        cliApp->add_subcommand(command.cliApp);
    };

    void prepare() {
        setupOptions();

        cliApp->callback([this]() {
            this->execute();
        });
    }

    void run(int argc, char* argv[]) {
        try {
            cliApp->parse(argc, argv);
        } catch(const CLI::Error &e) {
            int rc = cliApp->exit(e);
            if (!cliApp->get_parent())
                exit(rc);
        }
    };

    void run(std::string cmdline) {
        try {
            cliApp->parse(cmdline, false);
        } catch(const CLI::Error &e) {
            cliApp->exit(e);
        }
    };

protected:
    virtual void setupOptions() {};
    virtual void execute() {};

    std::shared_ptr<App>& getApp() {
        return cliApp;
    }

private:
    std::shared_ptr<App> cliApp;
};
