#include "ssgx_log_t_logger.h"
#include "ssgx_log_t_t.h"

namespace ssgx {
namespace log_t {

SSGXLogger& SSGXLogger::GetInstance() {
    static SSGXLogger instance;
    return instance;
}

void SSGXLogger::Init(const std::string& logger_name, const std::string& log_file,
                      LogLevel log_level, bool append_console) {
    if (logger_name.empty() || log_file.empty()) {
        throw std::runtime_error("logger_name and log_file must not be empty");
    }
    int ret;
    sgx_status_t status = ssgx_ocall_init_logger(
        &ret, logger_name.c_str(), log_file.c_str(),
        static_cast<int32_t>(log_level),
        append_console ? 1 : 0);
    if (status != SGX_SUCCESS) {
        throw std::runtime_error(std::string("Failed in ssgx_ocall_init_logger (error code: ") +
                                 std::to_string(status) + ")");
    }
    if (ret != 0) {
        throw std::runtime_error(std::string("ssgx_ocall_init_logger returned error: ") +
                                 std::to_string(ret));
    }
}

void SSGXLogger::SetTraceId(const std::string& trace_id) {
    sgx_status_t status = ssgx_ocall_set_trace_id(trace_id.c_str());
    if (status != SGX_SUCCESS) {
        throw std::runtime_error(std::string("Failed in ssgx_ocall_set_trace_id(trace_id) (error code: ") +
                                 std::to_string(status) + ")");
    }
}

SSGXLogger::SSGXLogger() = default;

SSGXLogger::~SSGXLogger() = default;

} // namespace log_t
} // namespace ssgx
