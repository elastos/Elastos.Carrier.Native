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
#include <any>
#include <spdlog/spdlog.h>

#include "types.h"
#include "def.h"

using namespace spdlog;
namespace elastos {
namespace carrier {

#define CARRIER_LOG_LEVEL_TRACE     0
#define CARRIER_LOG_LEVEL_DEBUG     1
#define CARRIER_LOG_LEVEL_INFO      2
#define CARRIER_LOG_LEVEL_WARN      3
#define CARRIER_LOG_LEVEL_ERROR     4
#define CARRIER_LOG_LEVEL_CRITICAL  5
#define CARRIER_LOG_LEVEL_OFF       6

enum Level {
    Trace = CARRIER_LOG_LEVEL_TRACE,
    Debug = CARRIER_LOG_LEVEL_DEBUG,
    Info = CARRIER_LOG_LEVEL_INFO,
    Warn = CARRIER_LOG_LEVEL_WARN,
    Error = CARRIER_LOG_LEVEL_ERROR,
    Critical = CARRIER_LOG_LEVEL_CRITICAL,
    Off = CARRIER_LOG_LEVEL_OFF,
};

//
// Pattern time - specific time getting to use for pattern_formatter.
// local time by default
//
enum PatternTimeType {
    Local, // localtime
    UTC    // utc time.
};

/*
 * If use Detail to print file lines and function name, need use such as: CARRIER_LOGGER_INFO,
 * and if use CARRIER_LOGGER_DEBUG or CARRIER_LOGGER_TRACE need modify the CARRIER_LOG_ACTIVE_LEVEL to support
*/

#ifndef CARRIER_FUNCTION
#    define CARRIER_FUNCTION static_cast<const char *>(__FUNCTION__)
#endif

#if !defined(CARRIER_LOG_ACTIVE_LEVEL)
#    define CARRIER_LOG_ACTIVE_LEVEL CARRIER_LOG_LEVEL_INFO
#endif

#define CARRIER_LOGGER_CALL(logger, level, ...) (logger)->source_log(__FILE__, __LINE__, CARRIER_FUNCTION, level, __VA_ARGS__);

#if CARRIER_LOG_ACTIVE_LEVEL <= CARRIER_LOG_LEVEL_TRACE
#    define CARRIER_LOGGER_TRACE(logger, ...) CARRIER_LOGGER_CALL(logger, Level::Trace, __VA_ARGS__)
#else
#    define CARRIER_LOGGER_TRACE(logger, ...) (void)0
#endif

#if CARRIER_LOG_ACTIVE_LEVEL <= CARRIER_LOG_LEVEL_DEBUG
#    define CARRIER_LOGGER_DEBUG(logger, ...) CARRIER_LOGGER_CALL(logger, Level::Debug, __VA_ARGS__)
#else
#    define CARRIER_LOGGER_DEBUG(logger, ...) (void)0
#endif

#if CARRIER_LOG_ACTIVE_LEVEL <= CARRIER_LOG_LEVEL_INFO
#    define CARRIER_LOGGER_INFO(logger, ...) CARRIER_LOGGER_CALL(logger, Level::Info, __VA_ARGS__)
#else
#    define CARRIER_LOGGER_INFO(logger, ...) (void)0
#endif

#if CARRIER_LOG_ACTIVE_LEVEL <= CARRIER_LOG_LEVEL_WARN
#    define CARRIER_LOGGER_WARN(logger, ...) CARRIER_LOGGER_CALL(logger, Level::Warn, __VA_ARGS__)
#else
#    define CARRIER_LOGGER_WARN(logger, ...) (void)0
#endif

#if CARRIER_LOG_ACTIVE_LEVEL <= CARRIER_LOG_LEVEL_ERROR
#    define CARRIER_LOGGER_ERROR(logger, ...) CARRIER_LOGGER_CALL(logger, Level::Error, __VA_ARGS__)
#else
#    define CARRIER_LOGGER_ERROR(logger, ...) (void)0
#endif

#if CARRIER_LOG_ACTIVE_LEVEL <= CARRIER_LOG_LEVEL_CRITICAL
#    define CARRIER_LOGGER_CRITICAL(logger, ...) CARRIER_LOGGER_CALL(logger, Level::Critical, __VA_ARGS__)
#else
#    define CARRIER_LOGGER_CRITICAL(logger, ...) (void)0
#endif

class CARRIER_PUBLIC Logger {
public:
    Logger() = default;
    Logger(const Sp<spdlog::logger> spd_logger) : spd_logger(spd_logger) {}

    //---- Init -----
    static void initialize(const std::string& config);

    static Sp<Logger> get(const std::string& name);

    static void setDefaultSettings(std::any settings);

    static void setLogFile(std::string file);

    static void setLogLevel(std::string level);

    static void setLogPattern(std::string pattern);

    //---- Print -----
    template<typename... Args>
    inline void log(Level level, format_string_t<Args...> fmt, Args &&... args) const {
        spdlog::level::level_enum log_level =  spdlog::level::level_enum(level);
        spd_logger->log(log_level, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void source_log(const char *filename_in, int line_in, const char *funcname_in, Level level, format_string_t<Args...> fmt, Args &&... args) const {
        spdlog::level::level_enum log_level =  spdlog::level::level_enum(level);
        spd_logger->log(spdlog::source_loc{filename_in, line_in, funcname_in}, log_level, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void trace(Args &&... args) const {
        log(Level::Trace, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void debug(Args &&... args) const {
        log(Level::Debug, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void warn(Args &&... args) const {
        log(Level::Warn, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void info(Args &&... args) const {
        log(Level::Info, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void error(Args &&... args) const {
        log(Level::Error, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void critical(Args &&... args) const {
        log(Level::Critical, std::forward<Args>(args)...);
    }

    void setLevel(Level level);

    bool isEnabled(Level level);

    bool isTraceEnabled() {
        return isEnabled(Level::Trace);
    }

    bool isDebugEnabled() {
        return isEnabled(Level::Debug);
    }

    bool isInfoEnabled() {
        return isEnabled(Level::Info);
    }

    bool isWarnEnabled() {
        return isEnabled(Level::Warn);
    }

    bool isErrorEnabled() {
        return isEnabled(Level::Error);
    }

    bool isCriticalEnabled() {
        return isEnabled(Level::Critical);
    }

    const std::string& getName();

    void setPattern(const std::string& pattern, PatternTimeType time_type = PatternTimeType::Local) const;

private:
    Sp<spdlog::logger> spd_logger;
};

} /* carrier  */
} /* elastos */
