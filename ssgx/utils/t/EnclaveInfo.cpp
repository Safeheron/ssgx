#include "ssgx_utils_t.h"

namespace ssgx {
namespace utils_t {

EnclaveInfo::EnclaveInfo() = default;
EnclaveInfo::~EnclaveInfo() = default;

EnclaveInfo& EnclaveInfo::GetCurrentEnclave() {
    static EnclaveInfo instance;
    return instance;
}

void EnclaveInfo::SetEnclaveEid(sgx_enclave_id_t eid) {
    std::lock_guard<std::mutex> lock(mutex_);
    enclave_eid_ = eid;
}

sgx_enclave_id_t EnclaveInfo::GetEnclaveEid() {
    std::lock_guard<std::mutex> lock(mutex_);
    return enclave_eid_;
}

} // namespace utils_t
} // namespace ssgx
