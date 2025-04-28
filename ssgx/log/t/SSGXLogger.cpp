#include "ssgx_log_t_logger.h"
#include "ssgx_log_t_t.h"

namespace ssgx {
namespace log_t {

SSGXLogger& SSGXLogger::GetInstance() {
    static SSGXLogger instance;
    return instance;
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
