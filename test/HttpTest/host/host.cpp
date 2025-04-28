#include <cstring>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <thread>


#include "sgx_urts.h"

#include "ssgx_log_u.h"
#include "ssgx_http_u.h"

#include "Enclave_u.h"

sgx_enclave_id_t test_enclave_id = 0;

void* server_thread_func(void* arg) {
    int ret = 0;
    sgx_status_t sgx_status = SGX_SUCCESS;
    const int server_alive_time_sec = 10;

    printf("Try to run ecall_run_server() ...\n\n");
    sgx_status = ecall_run_server(test_enclave_id, &ret, server_alive_time_sec);
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        printf("--->ecall_run_server() failed, error code: %d, error message: %s\n", ret, strerror((int)sgx_status));
        return nullptr;
        ;
    }

    printf("\nExit from function ecall_run_server()!\n");
    return nullptr;
}

int SGX_CDECL main(int argc, char* argv[]) {
    int ret = 0;
    sgx_status_t sgx_status = SGX_SUCCESS;
    bool ok = true;

    printf("Try to create testing enclave ...\n");
    sgx_status = sgx_create_enclave((const char*)argv[1], 0, nullptr, nullptr, &test_enclave_id, nullptr);
    if (sgx_status != SGX_SUCCESS) {
        printf("--->Initialize enclave failed! enclave file: %s, sgx message: %s\n", argv[1],
               strerror((int)sgx_status));
        return -1;
    }
    printf("Enclave is created!\n\n");

    ok = ssgx::http_u::HttpCallbackManager::GetInstance().RegisterCallbacks(test_enclave_id, ssgx_ecall_register_enclave_eid, ssgx_ecall_http_on_message);
    if(!ok){
        printf("\nFailed to register callback function for HTTP module.\n");
        goto _exit;
    }

    // Initialize SSGXLogger
    ssgx::log_u::SSGXLogger::GetInstance().Init("/opt/logs/tee-log/log-safeheron-mpc-engine", "PROJECT_NAME",
                                                ssgx::log_u::LogLevel::INFO, true);

    // Run server in child thread
    pthread_t pthread;
    pthread_create(&pthread, nullptr, server_thread_func, nullptr);

    sleep(2);

    // Run client
    printf("Try to run ecall_run_client() ...\n\n");
    sgx_status = ecall_run_client(test_enclave_id, &ret);
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        printf("--->ecall_run_client() failed, error code: %d, error message: %s\n", ret, strerror((int)sgx_status));
        goto _exit;
    }
    printf("\nExit from function ecall_run_client()!\n");

_exit:
    // Waiting for server exit
    pthread_join(pthread, nullptr);
    sgx_destroy_enclave(test_enclave_id);
    printf("End!\n\n");

    return ret;
}