#include <iomanip>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sgx_urts.h"

#include "ssgx_attestation_u.h"

#include "Enclave_u.h"

using namespace ssgx::attestation_u;

#define PRINT_REMOTE_ATTESTOR_STATUS(info, attestor)                                                                         \
    do {                                                                                                               \
        auto qv = (attestor).GetRawQvResult();                                                                         \
        auto err_code = (attestor).GetLastErrorCode();                                                                 \
        auto err_msg = (attestor).GetLastErrorMsg();                                                                   \
        if (qv.has_value()) {                                                                                          \
            printf("[RemoteAttestor] %s:%d %s()\n"                                                      \
                   "  Info         : %s\n"                                                          \
                   "  QvResult     : 0x%04X\n"                                                          \
                   "  ErrorCode    : %d\n"                                                              \
                   "  ErrorMessage : %s\n",                                                             \
                   __FILE__, __LINE__, __func__, (info), static_cast<uint32_t>(*qv),                            \
                   static_cast<int>(err_code), err_msg.c_str());                                     \
        } else {                                                                                                       \
            printf("[RemoteAttestor] %s:%d %s()\n"                                                      \
                   "  Info         : %s\n"                                                          \
                   "  QvResult     : (not available)\n"                                                 \
                   "  ErrorCode    : %d\n"                                                              \
                   "  ErrorMessage : %s\n",                                                             \
                   __FILE__, __LINE__, __func__, (info), static_cast<int>(err_code), err_msg.c_str());          \
        }                                                                                                              \
    } while (0)


extern "C" int ocall_symlink_file(const char* path, const char* link_path) {
    return symlink(path, link_path) == 0 ? 0 : -1;
}

extern "C" int ocall_verify_quote_untrusted(const uint8_t* quote, int quote_size, uint64_t time_stamp, uint64_t validity_seconds,
                                 const char* user_info) {
    int ret;
    RemoteAttestor attestor;
    std::string quote_report((char*)quote, quote_size);
    std::string mrenclave_hex_in_report;
    if (time_stamp == 0) {
        attestor.SetAcceptableResults({
            QvResult::Ok,
            QvResult::ConfigNeeded,
            QvResult::OutOfDate,
            QvResult::OutOfDateConfigNeeded,
            QvResult::SwHardeningNeeded,
            QvResult::ConfigAndSwHardeningNeeded
        });
        bool pass = attestor.VerifyReport(user_info, quote_report, mrenclave_hex_in_report) && mrenclave_hex_in_report.size() == 64;
        ret = pass ? 0 : -1;
        PRINT_REMOTE_ATTESTOR_STATUS(pass? "U_ASSERT_TRUE" : "U_ASSERT_FALSE", attestor);
        if (pass) {
            printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
        }
        return ret;
    } else {
        attestor.SetAcceptableResults({
            QvResult::Ok,
            QvResult::ConfigNeeded,
            QvResult::OutOfDate,
            QvResult::OutOfDateConfigNeeded,
            QvResult::SwHardeningNeeded,
            QvResult::ConfigAndSwHardeningNeeded
        });
        bool pass = attestor.VerifyReport(user_info, time_stamp, validity_seconds, quote_report, mrenclave_hex_in_report) && mrenclave_hex_in_report.size() == 64;
        ret = pass ? 0 : -1;
        PRINT_REMOTE_ATTESTOR_STATUS(pass? "U_ASSERT_TRUE" : "U_ASSERT_FALSE", attestor);
        if (pass) {
            printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
        }
        return ret;
    }
}

extern "C" int ocall_verify_quote_untrusted_original(const uint8_t* quote, int quote_size, const uint8_t user_info[64]) {
    RemoteAttestor attestor;
    std::string quote_report((char*)quote, quote_size);
    std::string mrenclave_hex_in_report;
    attestor.SetAcceptableResults({
        QvResult::Ok,
        QvResult::ConfigNeeded,
        QvResult::OutOfDate,
        QvResult::OutOfDateConfigNeeded,
        QvResult::SwHardeningNeeded,
        QvResult::ConfigAndSwHardeningNeeded
    });
    bool pass = attestor.VerifyReport(user_info, quote_report, mrenclave_hex_in_report);
    int ret = pass ? 0 : -1;
    PRINT_REMOTE_ATTESTOR_STATUS(pass? "U_ASSERT_TRUE" : "U_ASSERT_FALSE", attestor);
    if (pass) {
        printf("MRENCLAVE in report: %s\n", mrenclave_hex_in_report.c_str());
    }
    return ret;
}
