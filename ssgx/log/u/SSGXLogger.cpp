#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "ssgx_log_u_logger.h"

#include "log4cplus/consoleappender.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/layout.h"
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
#include "log4cplus/mdc.h"
#include "LogHelper.h"

namespace ssgx {
namespace log_u {

SSGXLogger::SSGXLogger() {
    logger_name_ = "";
    initialized_ = false;
}

SSGXLogger& SSGXLogger::GetInstance() {
    static SSGXLogger instance;
    return instance;
}

void SSGXLogger::Init(const std::string& logger_name, const std::string& log_file, LogLevel log_level,
                      bool append_console) {
    ssgx::log_u::LogHelper::GetInstance().SetLogger(logger_name, log_file, MapLogLevel(log_level), append_console);
    ssgx::log_u::LogHelper::GetInstance().SetTraceId("main");
    logger_name_ = logger_name;
    initialized_ = true;
}

void SSGXLogger::SetTraceId(const std::string& trace_id) {
    ssgx::log_u::LogHelper::GetInstance().SetTraceId(trace_id);
}

// Helper to map custom log levels to log4cplus levels
int SSGXLogger::MapLogLevel(LogLevel log_level) const {
    switch (log_level) {
    case LogLevel::TRACE:
        return log4cplus::TRACE_LOG_LEVEL;
    case LogLevel::DEBUG:
        return log4cplus::DEBUG_LOG_LEVEL;
    case LogLevel::INFO:
        return log4cplus::INFO_LOG_LEVEL;
    case LogLevel::WARN:
        return log4cplus::WARN_LOG_LEVEL;
    case LogLevel::ERROR:
        return log4cplus::ERROR_LOG_LEVEL;
    case LogLevel::FATAL:
        return log4cplus::FATAL_LOG_LEVEL;
    case LogLevel::OFF:
        return log4cplus::OFF_LOG_LEVEL;
    default:
        return log4cplus::NOT_SET_LOG_LEVEL;
    }
}

bool SSGXLogger::IsInitialized() {
    return initialized_;
}

int SSGXLogger::WriteLog(ssgx::log_u::LogLevel log_level, const char* msg) {
    if (!GetInstance().IsInitialized())
        return -1;
    switch (log_level) {
    case ssgx::log_u::LogLevel::INFO:
        LOG4CPLUS_INFO_FMT(
            ssgx::log_u::LogHelper::GetInstance().GetLogger(ssgx::log_u::SSGXLogger::GetInstance().GetLoggerName()),
            "%s", msg);
        break;
    case ssgx::log_u::LogLevel::WARN:
        LOG4CPLUS_WARN_FMT(
            ssgx::log_u::LogHelper::GetInstance().GetLogger(ssgx::log_u::SSGXLogger::GetInstance().GetLoggerName()),
            "%s", msg);
        break;
    case ssgx::log_u::LogLevel::ERROR:
        LOG4CPLUS_ERROR_FMT(
            ssgx::log_u::LogHelper::GetInstance().GetLogger(ssgx::log_u::SSGXLogger::GetInstance().GetLoggerName()),
            "%s", msg);
        break;
    case ssgx::log_u::LogLevel::FATAL:
        LOG4CPLUS_FATAL_FMT(
            ssgx::log_u::LogHelper::GetInstance().GetLogger(ssgx::log_u::SSGXLogger::GetInstance().GetLoggerName()),
            "%s", msg);
        break;
    case ssgx::log_u::LogLevel::DEBUG:
        LOG4CPLUS_DEBUG_FMT(
            ssgx::log_u::LogHelper::GetInstance().GetLogger(ssgx::log_u::SSGXLogger::GetInstance().GetLoggerName()),
            "%s", msg);
        break;
    case ssgx::log_u::LogLevel::TRACE:
        LOG4CPLUS_TRACE_FMT(
            ssgx::log_u::LogHelper::GetInstance().GetLogger(ssgx::log_u::SSGXLogger::GetInstance().GetLoggerName()),
            "%s", msg);
        break;
    default:
        break;
    }
    return 0;
}

} // namespace log_u
} // namespace ssgx
