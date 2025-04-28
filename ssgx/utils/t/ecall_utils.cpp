#include <cstdint>
#include <cstring>

#include "sgx_eid.h"
#include "sgx_report.h"
#include "sgx_utils.h"

#include "ssgx_utils_t.h"

using ssgx::utils_t::EnclaveInfo;

extern "C" void ssgx_ecall_get_enclave_id(uint8_t enclave_id[32]) {
    sgx_target_info_t self_info;
    sgx_self_target(&self_info);
    memcpy(enclave_id, self_info.mr_enclave.m, SGX_HASH_SIZE);
}

extern "C" void ssgx_ecall_register_enclave_eid(sgx_enclave_id_t eid) {
    EnclaveInfo::GetCurrentEnclave().SetEnclaveEid(eid);
}
