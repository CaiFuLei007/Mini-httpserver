
#pragma once

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/Utility.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"
#include "quill/sinks/RotatingFileSink.h"
#include <filesystem>
#include <fstream>
#include <iostream>

struct CustomFrontendOptions
{
    static constexpr quill::QueueType queue_type = quill::QueueType::UnboundedBlocking;
    static constexpr size_t initial_queue_capacity = 8 * 1024 * 1024;
    static constexpr uint32_t blocking_queue_retry_interval_ns = 800;
    static constexpr size_t unbounded_queue_max_capacity = 2ull * 1024u * 1024u * 1024u;
    static constexpr quill::HugePagesPolicy huge_pages_policy = quill::HugePagesPolicy::Never;
};

enum Time_format
{
    None,
    MilliSeconds,
    MicroSeconds,
    NanoSeconds
};

enum Show_where
{
    Write_console,
    Write_file,
    Both_console_file
};


class LoggerManager
{
    LoggerManager();

public:
    static LoggerManager &instance();

    static void set_pattern(bool data, bool time, enum Time_format time_format,
                            bool log_level, bool thread_id, bool filename_lineno);

    static void set_write_into(Show_where show);

    quill::LoggerImpl<CustomFrontendOptions> *get_logger() ;

    static void set_logger_path(const std::string &directory, const std::string &filename);

private:
    // quill::Logger *logger_;
    quill::LoggerImpl<CustomFrontendOptions> *logger_;

    static std::shared_ptr<LoggerManager> instance_log_;
    static quill::PatternFormatterOptions formatter_;
    static std::mutex instance_mutex_;
    static Show_where show_;
    static bool is_set_pattern;
    static std::string path_;
};



quill::LoggerImpl<CustomFrontendOptions> *get_quill_logger();

class Log_Control
{
public:
    Log_Control() = delete;

    static void set_pattern(bool data, bool time, enum Time_format time_format, bool log_level, bool thread_id, bool filename_lineno);

    static void set_write_into_where(Show_where show_pos);

    static void set_path(const std::string &directory, const std::string &filename);

    static void set_log_level(quill::LogLevel level);
};


#define QLOG_INFO(fmt, ...)                                                                                         \
        QUILL_LOGGER_CALL(QUILL_LIKELY, (get_quill_logger()), nullptr, quill::LogLevel::Info, fmt, ##__VA_ARGS__); 

#define QLOG_DEBUG(fmt, ...)                                                                                   \
    do                                                                                                             \
    {                                                                                                              \
        QUILL_LOGGER_CALL(QUILL_LIKELY, (get_quill_logger()), nullptr, quill::LogLevel::Debug, fmt, ##__VA_ARGS__); \
        get_quill_logger()->flush_log();                                                                           \
    } while (0)

#define QLOG_ERROR(fmt, ...)                                                                                   \
    do                                                                                                             \
    {                                                                                                              \
        QUILL_LOGGER_CALL(QUILL_LIKELY, (get_quill_logger()), nullptr, quill::LogLevel::Error, fmt, ##__VA_ARGS__); \
        get_quill_logger()->flush_log();                                                                           \
    } while (0)

#define QLOG_INFO_FLUSH(fmt, ...)                                                                                   \
    do                                                                                                             \
    {                                                                                                              \
        QUILL_LOGGER_CALL(QUILL_LIKELY, (get_quill_logger()), nullptr, quill::LogLevel::Info, fmt, ##__VA_ARGS__); \
        get_quill_logger()->flush_log();                                                                           \
    } while (0)

#define _QLOG_DEBUG(fmt, ...) \
    QUILL_LOGGER_CALL(QUILL_LIKELY, (get_quill_logger()), nullptr, quill::LogLevel::Debug, fmt, ##__VA_ARGS__)

#define _QLOG_ERROR(fmt, ...) \
    QUILL_LOGGER_CALL(QUILL_LIKELY, (get_quill_logger()), nullptr, quill::LogLevel::Error, fmt, ##__VA_ARGS__)

#define QLOG_WARN(fmt, ...) \
    QUILL_LOGGER_CALL(QUILL_LIKELY, (get_quill_logger()), nullptr, quill::LogLevel::Warning, fmt, ##__VA_ARGS__)

#define QLOG_NOTICE(fmt, ...) \
    QUILL_LOGGER_CALL(QUILL_LIKELY, (get_quill_logger()), nullptr, quill::LogLevel::Notice, fmt, ##__VA_ARGS__)

#define QLOG_CRITICAL(fmt, ...) \
    QUILL_LOGGER_CALL(QUILL_LIKELY, (get_quill_logger()), nullptr, quill::LogLevel::Critical, fmt, ##__VA_ARGS__)
