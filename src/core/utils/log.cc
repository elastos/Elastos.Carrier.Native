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

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wunqualified-std-cast-call"
#endif

#include <iostream>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog_setup/conf.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <iostream>
#include "carrier/log.h"

using namespace spdlog;

namespace elastos {
namespace carrier {

struct UserSettings {
    level::level_enum defalutLevel = level::info;
    Sp<sinks::basic_file_sink_mt> fileSink = nullptr;
    std::string pattern {};
};

static UserSettings userSettings {};

void Logger::initialize(const std::string& config) {
    spdlog_setup::from_file(config);
}

std::shared_ptr<Logger> Logger::get(const std::string& name) {
    auto spd_logger = spdlog::get(name);
    if (!spd_logger) {
        if (userSettings.fileSink == nullptr) {
            spd_logger = spdlog::stdout_color_mt(name);
        }
        else {
            spd_logger = std::make_shared<spdlog::logger>(name, userSettings.fileSink);
            spdlog::register_logger(spd_logger);
        }
    }

    spd_logger->set_level(userSettings.defalutLevel);
    if (!userSettings.pattern.empty())
        spd_logger->set_pattern(userSettings.pattern, pattern_time_type::local);

    return std::make_shared<Logger>(spd_logger);
}

void Logger::setDefaultSettings(std::any value) {

    if (value.type() != typeid(std::map<std::string, std::any>)) {
        std::cout << "Logger : invalid configure! " << std::endl;
    }

    auto settings = std::any_cast<std::map<std::string, std::any>>(value);

    if (settings.count("level")) {
        std::string level = std::any_cast<std::string>(settings.at("level"));
        setDefaultLevel(level);
    }

    if (settings.count("logFile")) {
        std::string logFile = std::any_cast<std::string>(settings.at("logFile"));
        setLogFile(logFile);
    }

    if (settings.count("pattern")) {
        userSettings.pattern = std::any_cast<std::string>(settings.at("pattern"));
    }
}

void Logger::setLogFile(std::string filename) {
    if (!filename.empty())
        userSettings.fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
}

void Logger::setDefaultLevel(std::string level) {
    static std::unordered_map<std::string, level::level_enum> levelMap = {
        { "trace", level::trace },
        { "debug", level::debug },
        { "info", level::info },
        { "warn", level::warn },
        { "err", level::err },
        { "critical", level::critical },
        { "off", level::off },
    };

    if (levelMap.count(level))
        userSettings.defalutLevel = levelMap[level];
}

void Logger::setDefaultPattern(std::string pattern) {
    userSettings.pattern = pattern;
}

void Logger::setLevel(Level level) {
    auto log_level =  spdlog::level::level_enum(level);
    spd_logger->set_level(log_level);
}

bool Logger::isEnabled(Level level) {
    auto log_level =  spdlog::level::level_enum(level);
    return log_level == spd_logger->level();
}

const std::string& Logger::getName() {
    return spd_logger->name();
}

void Logger::setPattern(const std::string& pattern, PatternTimeType time_type) const {
    auto spd_time_type = spdlog::pattern_time_type(time_type);
    spd_logger->set_pattern(pattern, spd_time_type);
}

}
}
