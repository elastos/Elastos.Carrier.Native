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

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/syslog_sink.h"
#include "spdlog_setup/conf.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "carrier/log.h"

using namespace spdlog;

namespace elastos {
namespace carrier {

void Logger::initialize(const std::string& config) {
    spdlog_setup::from_file(config);
}

std::shared_ptr<Logger> Logger::get(const std::string& name) {
    auto spd_logger = spdlog::get(name);
    if (!spd_logger) {
        spd_logger = spdlog::stdout_color_mt(name);
    }
    return std::make_shared<Logger>(spd_logger);
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
