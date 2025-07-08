#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <csignal>

#include "sgx_urts.h"

#include "ssgx_log_u.h"
#include "ssgx_http_u.h"

#include "Enclave_u.h"

sgx_enclave_id_t test_enclave_id = 0;

#include <atomic>

std::atomic<bool> g_should_stop{false};

void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        ssgx::http_u::StopHttpServers();
        std::cout << "Stopping server..." << std::endl;
    }
}

int SGX_CDECL main(int argc, char* argv[]) {
    int ret = 0;
    sgx_status_t sgx_status;
    uint8_t enclave_id[32] = {0};
    bool ok = true;

    signal(SIGINT, handle_signal);   // Ctrl+C
    signal(SIGTERM, handle_signal);  // kill

    printf("Try to create testing enclave ...\n");
    sgx_status = sgx_create_enclave((const char*)argv[1], 0, nullptr, nullptr, &test_enclave_id, nullptr);
    if (sgx_status != SGX_SUCCESS) {
        printf("--->Initialize enclave failed! enclave file: %s, sgx message: %s\n", argv[1],
               strerror((int)sgx_status));
        ret = -1;
        goto _exit;
    }
    printf("Enclave (eid = %ld) is created!\n\n", test_enclave_id);

    ok = ssgx::http_u::HttpCallbackManager::GetInstance().RegisterCallbacks(test_enclave_id, ssgx_ecall_register_enclave_eid, ssgx_ecall_http_on_message);
    if(!ok){
        printf("\nFailed to register callback function for HTTP module.\n");
        goto _exit;
    }

    // Initialize SSGXLogger
    ssgx::log_u::SSGXLogger::GetInstance().Init("/opt/logs/tee-log/log-safeheron-mpc-engine", "PROJECT_NAME",
                                                ssgx::log_u::LogLevel::INFO, true);

    printf("Try to run ecall_run_http_server() ...\n\n");
    sgx_status = ecall_run_http_server(test_enclave_id, &ret);
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        printf("--->ecall_run_http_server() failed, error code: %d, error message: %s\n", ret, strerror((int)sgx_status));
        goto _exit;
    }
    printf("\nExit from function ecall_run_http_server()!\n");

_exit:
    printf("Destroy enclave!\n\n");
    sgx_destroy_enclave(test_enclave_id);

    return ret;
}
