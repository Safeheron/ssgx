set(app "${PROJECT_NAME}_app")

ssgx_add_untrusted_executable(${app}
		SRCS host.cpp
		EDL Enclave.edl
		EDL_SEARCH_PATHS ../ ${ssgx_EDL_DIRS}
		UNTRUSTED_LIBS
			ssgx::ssgx_http_u
			ssgx::ssgx_log_u
			ssgx::ssgx_config_u
			ssgx::ssgx_utils_u
			ssgx::ssgx_filesystem_u
			ssgx::ssgx_attestation_u
)

target_compile_features(${app} PRIVATE cxx_std_11)

