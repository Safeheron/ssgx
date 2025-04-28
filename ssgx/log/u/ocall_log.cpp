#include <stdio.h>
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

} // extern "C"
