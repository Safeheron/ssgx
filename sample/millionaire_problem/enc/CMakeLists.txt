set(enclave "${PROJECT_NAME}_enclave")


ssgx_add_enclave_library(${enclave}
        USE_SGXSSL OFF
        SRCS Enclave.cpp TrustedArbiter.cpp
        TRUSTED_LIBS
                ssgx::ssgx_utils_t
                ssgx::ssgx_config_t
                ssgx::ssgx_decimal_t
                ssgx::ssgx_log_t
                ssgx::ssgx_json_t
                ssgx::ssgx_attestation_t
        EDL Enclave.edl
        EDL_SEARCH_PATHS ../ ${ssgx_EDL_DIRS}
)

target_compile_features(${enclave} PRIVATE cxx_std_17)

ssgx_sign_enclave(${enclave}
        KEY Enclave_private_test.pem
        CONFIG Enclave.config.xml
)