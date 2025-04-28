#include <stdexcept>
#include <string>

#include "sgx_edger8r.h"

#include "ssgx_log_t.h"
#include "ssgx_log_t_t.h"

namespace ssgx {
namespace log_t {

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
    int ret;
    sgx_status_t status = ssgx_ocall_write_log(&ret, static_cast<int>(level_), log_buf.c_str());
    if (status != SGX_SUCCESS) {
        throw std::runtime_error(std::string("Failed in ssgx_ocall_write_log(level, msg) (error code: ") +
                                 std::to_string(status) + ")");
    }
    if (ret != 0) {
        throw std::runtime_error(std::string("SSGXLogger has not been initialized!"));
    }
}

} // namespace internal

} // namespace log_t
} // namespace ssgx
