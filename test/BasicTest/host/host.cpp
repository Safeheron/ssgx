#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sgx_urts.h"

#include "ssgx_attestation_u.h"
#include "ssgx_log_u.h"

#include "Enclave_u.h"

using namespace ssgx::attestation_u;

sgx_enclave_id_t test_enclave_id = 0;

int SGX_CDECL main(int argc, char* argv[]) {
    int ret = 0;
    sgx_status_t sgx_status;
    uint8_t enclave_id[32] = {0};
    const std::string test_toml_file = "test.toml";

    printf("Try to create testing enclave ...\n");
    sgx_status = sgx_create_enclave((const char*)argv[1], 0, nullptr, nullptr, &test_enclave_id, nullptr);
    if (sgx_status != SGX_SUCCESS) {
        printf("--->Initialize enclave failed! enclave file: %s, sgx message: %s\n", argv[1],
               strerror((int)sgx_status));
        ret = -1;
        goto _exit;
    }
    printf("Enclave is created!\n\n");

    // Initialize SSGXLogger
    ssgx::log_u::SSGXLogger::GetInstance().Init("PROJECT_NAME","/tmp/tee-log",
                                                ssgx::log_u::LogLevel::INFO, true);

    printf("Try to run ecall_run_test() ...\n\n");
    sgx_status = ecall_run_test(test_enclave_id, &ret);
    if (sgx_status != SGX_SUCCESS) {
        printf("--->ecall_run_test() failed, error code: %d, error message: %s\n", ret, strerror((int)sgx_status));
        goto _exit;
    }
    printf("\nExit from function ecall_run_test()!\n");

_exit:
    printf("Destroy enclave!\n\n");
    sgx_destroy_enclave(test_enclave_id);

    return ret;
}