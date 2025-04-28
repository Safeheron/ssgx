set(enclave "${PROJECT_NAME}_enclave")
find_package(SafeheronCryptoSuitesSgx REQUIRED)

ssgx_add_enclave_library(${enclave}
        USE_SGXSSL OFF
        SRCS Enclave.cpp
                ../cases/ssgx_utils_t_test.cpp
                ../cases/ssgx_utils_t_time_test.cpp
                ../cases/ssgx_utils_t_uuid_test.cpp
                ../cases/ssgx_utils_t_seal_handler_test.cpp
                ../cases/ssgx_utils_t_fmt_test.cpp
                ../cases/ssgx_utils_t_mem_test.cpp
                ../cases/ssgx_testframework_t_test.cpp
                ../cases/ssgx_filesystem_t_test.cpp
                ../cases/ssgx_config_t_test.cpp
                ../cases/ssgx_decimal_t_test.cpp
                ../cases/ssgx_log_t_test.cpp
                ../cases/ssgx_json_t_test.cpp
                ../cases/ssgx_http_t_client_test.cpp
                ../cases/ssgx_attestation_t_test.cpp
        TRUSTED_LIBS
                ssgx::ssgx_utils_t
                ssgx::ssgx_config_t
                ssgx::ssgx_decimal_t
                ssgx::ssgx_log_t
                ssgx::ssgx_json_t
                ssgx::ssgx_http_t
                ssgx::ssgx_filesystem_t
                ssgx::ssgx_attestation_t
                SafeheronCryptoSuitesSgx
        EDL Enclave.edl
        EDL_SEARCH_PATHS ../ ${ssgx_EDL_DIRS}
)

target_compile_features(${enclave} PRIVATE cxx_std_17)

ssgx_sign_enclave(${enclave}
        KEY Enclave_private_test.pem
        CONFIG Enclave.config.xml
)