#ifndef SSGX_LOG_U_LOGHELPER_H
#define SSGX_LOG_U_LOGHELPER_H

#include <mutex>
#include <string>

#include "ssgx_log_u.h"

#include "log4cplus/logger.h"
#include "log4cplus/loglevel.h"

namespace ssgx {
namespace log_u {

class LogHelper {
  public:
    static LogHelper& GetInstance();

    void SetLogger(const std::string& logger_name, const std::string& log_file, log4cplus::LogLevel log_level,
                   bool append_console = false);

    log4cplus::Logger GetLogger(const std::string& logger_name);

    void SetLogLevel(const std::string& logger_name, log4cplus::LogLevel log_level);

    void SetTraceId(const std::string& trace_id);

    void ClearTraceId();

  private:
    LogHelper();

    ~LogHelper();

    std::mutex mutex_; // Protects global initialization and shared resources
};

} // namespace log_u
} // namespace ssgx

#endif // SSGX_LOG_U_LOGHELPER_H