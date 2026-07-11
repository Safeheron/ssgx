#include <stdio.h>
#include <exception>
#include <unistd.h>

#include "ssgx_log_t_u.h"
#include "ssgx_log_u.h"

#include "log4cplus/loggingmacros.h"
#include "LogHelper.h"

extern "C" {

int ssgx_ocall_write_log(int32_t level, const char* msg) {
    if (level < 0 || level > 6)
        return -2;
    auto log_level = static_cast<ssgx::log_u::LogLevel>(level);
    return ssgx::log_u::SSGXLogger::GetInstance().WriteLog(log_level, msg);
}

void ssgx_ocall_set_trace_id(const char* trace_id) {
    ssgx::log_u::LogHelper::GetInstance().SetTraceId(trace_id);
}

int ssgx_ocall_init_logger(const char* logger_name, const char* log_file,
                           int32_t log_level, int32_t append_console) {
    if (!logger_name || logger_name[0] == '\0' ||
        !log_file    || log_file[0]    == '\0' ||
        log_level < 0 || log_level > 6) return -1;
    try {
        auto level = static_cast<ssgx::log_u::LogLevel>(log_level);
        ssgx::log_u::SSGXLogger::GetInstance().Init(logger_name, log_file, level, append_console != 0);
        return 0;
    } catch (const std::exception&) {
        // Init failed with a standard exception (e.g. log file not writable).
        return -2;
    } catch (...) {
        // Non-standard exception. Use a distinct code so it is
        // distinguishable from the std::exception case above.
        return -3;
    }
}

} // extern "C"
