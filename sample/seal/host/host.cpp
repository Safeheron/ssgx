#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sgx_urts.h"

#include "Enclave_u.h"

int SGX_CDECL main(int argc, char* argv[]) {
    int ret = 0;
    sgx_status_t sgx_status = SGX_SUCCESS;
    sgx_enclave_id_t enclave_id;

    printf("Try to create testing enclave ...\n");
    sgx_status = sgx_create_enclave((const char*)argv[1], 0, nullptr, nullptr, &enclave_id, nullptr);
    if (sgx_status != SGX_SUCCESS) {
        printf("--->Initialize enclave failed! enclave file: %s, sgx message: %s\n", argv[1],
               strerror((int)sgx_status));
        return -1;
    }
    printf("Enclave is created!\n\n");

    sgx_status = ecall_run_test(enclave_id, &ret);
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        printf("--->ecall_run() failed, error code: %d, error message: %s\n", ret, strerror((int)sgx_status));
    }
    printf("\nExit from function ecall_run()!\n");
    return ret;
}