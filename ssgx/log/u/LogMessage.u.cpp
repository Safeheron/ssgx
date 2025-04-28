#include "ssgx_log_u.h"

#include "LogHelper.h"

namespace ssgx {
namespace log_u {

#include "../share/LogMessage.cpp"

namespace internal {

void LogMessage::Finish() {
    // format log
    // example: [main.cpp(42)]PrintLog():
    std::string log_buf;
    log_buf.reserve(500);
    log_buf.append("[");
    log_buf.append(filename_);
    log_buf.append("(");
    log_buf.append(std::to_string(line_));
    log_buf.append(")");
    log_buf.append("]");
    log_buf.append(func_name_);
    log_buf.append(":");
    log_buf.append(message_);

    // output log message
    ssgx::log_u::SSGXLogger::GetInstance().WriteLog(level_, log_buf.c_str());
}

} // namespace internal

} // namespace log_u
} // namespace ssgx
