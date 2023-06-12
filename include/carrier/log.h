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

#include "spdlog/spdlog.h"
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

/**
 * @brief 记录Carrier行为日志
 *
 */
class CARRIER_PUBLIC Logger {
public:
    /**
     * @brief 创建空Logger
     *
     */
    Logger() = default;
    /**
     * @brief 根据spdlog内容创建Logger
     *
     * @param spd_logger
     */
    Logger(const Sp<spdlog::logger> spd_logger) : spd_logger(spd_logger) {}

    //---- Init -----
    /**
     * @brief 初始化Logger
     *
     * @param config 日志配置文件路径
     */
    static void initialize(const std::string& config);
    /**
     * @brief 根据模块名创建对应的Logger，主要可以用于模块的Logger
     *
     * @param name 模块名
     * @return Sp<Logger> 返回对应Logger对象指针
     */
    static Sp<Logger> get(const std::string& name);
    /**
     * @brief 设置日志的默认配置
     *
     * @param settings 配置参数
     */
    static void setDefaultSettings(std::any settings);
    /**
     * @brief 设置日志文件
     *
     * @param file 日志文件路径
     */
    static void setLogFile(std::string file);
    /**
     * @brief 设置日志等级
     *
     * @param level 等级字符串
     */
    static void setLogLevel(std::string level);
    /**
     * @brief 设置日志模式
     *
     * @param pattern 模式字符串
     */
    static void setLogPattern(std::string pattern);

    //---- Print -----
    /**
     * @brief 输出日志
     *
     * @tparam Args 输出数据的类型
     * @param level 日志等级
     * @param fmt 输书数据字符串形式
     * @param args 输出参数
     */
    template<typename... Args>
    inline void log(Level level, format_string_t<Args...> fmt, Args &&... args) const {
        spdlog::level::level_enum log_level =  spdlog::level::level_enum(level);
        spd_logger->log(log_level, fmt, std::forward<Args>(args)...);
    }
    /**
     * @brief 输出指定文件日志
     *
     * @tparam Args 输出数据的类型
     * @param filename_in 需要输出的文件
     * @param line_in 需要输出的文件行
     * @param funcname_in 需要输出的函数名
     * @param level 日志等级
     * @param fmt 输书数据字符串形式
     * @param args 输出参数
     */
    template<typename... Args>
    void source_log(const char *filename_in, int line_in, const char *funcname_in, Level level, format_string_t<Args...> fmt, Args &&... args) const {
        spdlog::level::level_enum log_level =  spdlog::level::level_enum(level);
        spd_logger->log(spdlog::source_loc{filename_in, line_in, funcname_in}, log_level, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief 以Level::Trace等级输出日志
     *
     * @tparam Args 输出数据的类型
     * @param args 输出数据
     */
    template<typename... Args>
    inline void trace(Args &&... args) const {
        log(Level::Trace, std::forward<Args>(args)...);
    }
    /**
     * @brief 以Level::Debug等级输出日志
     *
     * @tparam Args 输出数据的类型
     * @param args 输出数据
     */
    template<typename... Args>
    inline void debug(Args &&... args) const {
        log(Level::Debug, std::forward<Args>(args)...);
    }
    /**
     * @brief 以Level::Warn等级输出日志
     *
     * @tparam Args 输出数据的类型
     * @param args 输出数据
     */
    template<typename... Args>
    inline void warn(Args &&... args) const {
        log(Level::Warn, std::forward<Args>(args)...);
    }
    /**
     * @brief 以Level::Info等级输出日志
     *
     * @tparam Args 输出数据的类型
     * @param args 输出数据
     */
    template<typename... Args>
    inline void info(Args &&... args) const {
        log(Level::Info, std::forward<Args>(args)...);
    }
    /**
     * @brief 以Level::Error等级输出日志
     *
     * @tparam Args 输出数据的类型
     * @param args 输出数据
     */
    template<typename... Args>
    inline void error(Args &&... args) const {
        log(Level::Error, std::forward<Args>(args)...);
    }
    /**
     * @brief 以Level::Critical等级输出日志
     *
     * @tparam Args 输出数据的类型
     * @param args 输出数据
     */
    template<typename... Args>
    inline void critical(Args &&... args) const {
        log(Level::Critical, std::forward<Args>(args)...);
    }
    /**
     * @brief 设置日志等级
     *
     * @param level 日志等级
     */
    void setLevel(Level level);
    /**
     * @brief 是否使能level日志等级
     *
     * @param level 日志等级
     * @return true level等级可用
     * @return false level等级不可用
     */
    bool isEnabled(Level level);
    /**
     * @brief 是否使能日志等级Level::Trace
     *
     * @return true Trace等级可用
     * @return false Trace等级不可用
     */
    bool isTraceEnabled() {
        return isEnabled(Level::Trace);
    }
    /**
     * @brief 是否使能日志等级Level::Debug
     *
     * @return true Debug等级可用
     * @return false Debug等级不可用
     */
    bool isDebugEnabled() {
        return isEnabled(Level::Debug);
    }
    /**
     * @brief 是否使能日志等级Level::Info
     *
     * @return true Info等级可用
     * @return false Info等级不可用
     */
    bool isInfoEnabled() {
        return isEnabled(Level::Info);
    }
    /**
     * @brief 是否使能日志等级Level::Warn
     *
     * @return true Warn等级可用
     * @return false Warn等级不可用
     */
    bool isWarnEnabled() {
        return isEnabled(Level::Warn);
    }
    /**
     * @brief 是否使能日志等级Level::Error
     *
     * @return true Error等级可用
     * @return false Error等级不可用
     */
    bool isErrorEnabled() {
        return isEnabled(Level::Error);
    }
    /**
     * @brief 是否使能日志等级Level::Critical
     *
     * @return true Critical等级可用
     * @return false Critical等级不可用
     */
    bool isCriticalEnabled() {
        return isEnabled(Level::Critical);
    }
    /**
     * @brief 获取Logger名
     *
     * @return const std::string& 返回名字字符串
     */
    const std::string& getName();
    /**
     * @brief 设置Logger格式
     *
     * @param pattern 日志格式
     * @param time_type 日志时间格式类型
     */
    void setPattern(const std::string& pattern, PatternTimeType time_type = PatternTimeType::Local) const;

private:
    Sp<spdlog::logger> spd_logger;
};

} /* carrier  */
} /* elastos */
