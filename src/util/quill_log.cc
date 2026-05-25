

#include "util/quill_log.h"

std::shared_ptr<LoggerManager> LoggerManager::instance_log_;
quill::PatternFormatterOptions LoggerManager::formatter_;
std::mutex LoggerManager::instance_mutex_;
Show_where LoggerManager::show_ = Write_console;
bool LoggerManager::is_set_pattern = false;
std::string LoggerManager::path_;

LoggerManager::LoggerManager()
{
    struct quill::BackendOptions backopt;
    backopt.sink_min_flush_interval = std::chrono::milliseconds{2};

    std::vector<std::shared_ptr<quill::Sink>> sinks_;
    if (!is_set_pattern)
        set_pattern(true, true, MicroSeconds, true, true, true);
    if (show_ == Write_file && path_.empty())
        set_logger_path("logs", "quill.log");

    if (show_ == Both_console_file || show_ == Write_console)
    {
        auto console_sink = quill::FrontendImpl<CustomFrontendOptions>::create_or_get_sink<quill::ConsoleSink>("console");
        sinks_.emplace_back(std::move(console_sink));
    }
    if (show_ == Both_console_file || show_ == Write_file)
    {
        quill::RotatingFileSinkConfig rotating_config;
        rotating_config.set_rotation_max_file_size(1024 * 1024 * 8);
        rotating_config.set_max_backup_files(5);
        sinks_.emplace_back(std::make_shared<quill::RotatingFileSink>(path_, rotating_config));
    }

    logger_ = quill::FrontendImpl<CustomFrontendOptions>::create_or_get_logger("log", std::move(sinks_), formatter_);

    quill::Backend::start(backopt);
    logger_->set_log_level(quill::LogLevel::Debug);
}

LoggerManager& LoggerManager::instance()
{
    if (!instance_log_)
    {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        if (!instance_log_)
        {
            instance_log_ = std::shared_ptr<LoggerManager>(new LoggerManager());
        }
    }
    return *instance_log_;
}

void LoggerManager::set_pattern(bool data, bool time, enum Time_format time_format,
                    bool log_level, bool thread_id, bool filename_lineno)
{
    std::string logger_pattern_str, time_format_str;
    if (data)
        time_format_str += "%Y/%m/%d ";
    if (time)
    {
        time_format_str += "%H:%M:%S";
        switch (time_format)
        {
        case MilliSeconds:
            time_format_str += ".%Qms";
            break;
        case MicroSeconds:
            time_format_str += ".%Qus";
            break;
        case NanoSeconds:
            time_format_str += ".%Qns";
            break;
        case None:
        default:
            break;
        }
    }

    if (data || time)
        logger_pattern_str += "[%(time)] ";
    if (thread_id)
        logger_pattern_str += "[%(thread_id)] ";
    if (log_level)
        logger_pattern_str += "[%(log_level)] ";
    // if (filename_lineno)
        logger_pattern_str += "[%(full_path):%(line_number)] ";

    logger_pattern_str += "%(message)";

    // std::cout << "logger_pattern_str: " << logger_pattern_str << std::endl;
    // std::cout << "time_format_str: " << time_format_str << std::endl;
    formatter_ = quill::PatternFormatterOptions(std::move(logger_pattern_str), std::move(time_format_str), quill::Timezone::LocalTime);
    is_set_pattern = true;
}

void LoggerManager::set_write_into(Show_where show) { show_ = show; }

quill::LoggerImpl<CustomFrontendOptions> * LoggerManager::get_logger() { return logger_; }

void LoggerManager::set_logger_path(const std::string &directory, const std::string &filename)
{
    if (!std::filesystem::create_directory(directory))
    {
        std::cout << "create directory failed: " << directory << std::endl;
    }
    std::string path = directory + "/" + filename;
    std::ofstream ofs(path);
    path_ = path;
}

quill::LoggerImpl<CustomFrontendOptions> *get_quill_logger()
{
    return LoggerManager::instance().get_logger();
}

void Log_Control::set_pattern(bool data, bool time, enum Time_format time_format, bool log_level, bool thread_id, bool filename_lineno)
{
    LoggerManager::set_pattern(data, time, time_format, log_level, thread_id, filename_lineno);
}

void Log_Control::set_write_into_where(Show_where show_pos)
{
    LoggerManager::set_write_into(show_pos);
}

void Log_Control::set_path(const std::string &directory, const std::string &filename)
{
    LoggerManager::set_logger_path(directory, filename);
}

void Log_Control::set_log_level(quill::LogLevel level)
{
    get_quill_logger()->set_log_level(level);
}
