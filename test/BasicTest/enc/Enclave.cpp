#include "ssgx_testframework_t.h"

#include "Enclave_t.h"

int ecall_run_test() {
    return ssgx::testframework_t::TestManager::RunAllSuites();
}
