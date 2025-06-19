#include "LogHelper.h"

#include "log4cplus/consoleappender.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/layout.h"
#include "log4cplus/mdc.h"

namespace ssgx {
namespace log_u {

// Get the singleton instance
LogHelper& LogHelper::GetInstance() {
    static LogHelper instance;
    return instance;
}

// Constructor
LogHelper::LogHelper() {
    log4cplus::initialize();
}

// Destructor
LogHelper::~LogHelper() {
    log4cplus::Logger::shutdown();
}

// Initialize the logging system
void LogHelper::SetLogger(const std::string& logger_name, const std::string& log_file, log4cplus::LogLevel log_level,
                          bool append_console) {
    auto logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(logger_name));
    logger.removeAllAppenders();

    log4cplus::tstring pattern = LOG4CPLUS_TEXT("%D{%Y-%m-%d %H:%M:%S.%q}[%c{2}][%-4p][%X{TraceId}][%t]%m%n");

    // Automatically create a new log file daily, with a maximum of 30 files.
    log4cplus::SharedAppenderPtr file_appender(
        new log4cplus::DailyRollingFileAppender(LOG4CPLUS_TEXT(log_file), log4cplus::DAILY, true, 30, false, false));
    file_appender->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)));

    logger.addAppender(file_appender);
    logger.setLogLevel(log_level);

    // Configure console logging if needed
    if (append_console) {
        log4cplus::SharedAppenderPtr console_appender(new log4cplus::ConsoleAppender());
        console_appender->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)));

        logger.addAppender(console_appender);
    }
}

// Get a logger for a specific module (lazy initialization)
log4cplus::Logger LogHelper::GetLogger(const std::string& logger_name) {
    return log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(logger_name));
}

// Set the log level for a specific logger
void LogHelper::SetLogLevel(const std::string& logger_name, log4cplus::LogLevel log_level) {
    auto logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(logger_name));
    logger.setLogLevel(log_level);
}

// Set the TraceID in the MDC
void LogHelper::SetTraceId(const std::string& trace_id) {
    log4cplus::getMDC().put(LOG4CPLUS_TEXT("TraceId"), LOG4CPLUS_TEXT(trace_id));
}

// Remove the TraceID from the MDC
void LogHelper::ClearTraceId() {
    log4cplus::getMDC().remove(LOG4CPLUS_TEXT("TraceId"));
}

} // namespace log_u
} // namespace ssgx