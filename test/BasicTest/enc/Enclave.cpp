#include "ssgx_testframework_t.h"
#include "ssgx_log_t.h"
#include "ssgx_utils_t.h"

#include "Enclave_t.h"

int ecall_run_test() {
    // Initialize logging from inside the enclave (via OCALL) instead of the
    // host side, demonstrating the trusted-side log Init() interface.
    try {
        ssgx::log_t::SSGXLogger::GetInstance().Init(
            "PROJECT_NAME", "/tmp/tee-log",
            ssgx::log_t::LogLevel::INFO, /*append_console=*/true);
    } catch (const std::exception& e) {
        // No logger available yet — fall back to Printf so the operator sees why
        ssgx::utils_t::Printf("[FATAL] log init from enclave failed: %s\n", e.what());
        return -1;
    }

    return ssgx::testframework_t::TestManager::RunAllSuites();
}
