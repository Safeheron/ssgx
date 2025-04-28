#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sgx_dcap_ql_wrapper.h"
#include "sgx_dcap_quoteverify.h"

#include "ssgx_attestation_t_u.h"
#include "ssgx_attestation_u.h"

#include "crypto-suites/crypto-hash/sha256.h"
#include "auxiliary.h"

using namespace safeheron::hash;
using namespace ssgx::attestation_u;

static bool s_quote_is_initialized = false;
static const char* SGX_AESM_ADDR = "SGX_AESM_ADDR";

extern "C" int ssgx_ocall_get_qe_target_info(sgx_target_info_t* p_qe3_target) {
    bool is_out_of_proc = false;
    quote3_error_t qe3_ret = SGX_QL_SUCCESS;

    if (!p_qe3_target) {
        return static_cast<int>(ErrorCode::InvalidParameter);
    }

    // Check AESM server, if it is available, set is_out_of_proc = true
    const char *out_of_proc = getenv(SGX_AESM_ADDR);
    if (!out_of_proc) {
        is_out_of_proc = true;
    }

    // Initialize QE libraries,
    // only need to initialize once in a process
    if (!s_quote_is_initialized) {
        if (initialize_qe_setting(is_out_of_proc) != 0) {
            return static_cast<int>(ErrorCode::InitializationFailed);
        }
        s_quote_is_initialized = true;
    }

    qe3_ret = sgx_qe_get_target_info(p_qe3_target);
    if (SGX_QL_SUCCESS != qe3_ret) {
        return  static_cast<int>(ErrorCode::GetTargetInfoFailed);
    }

    return static_cast<int>(ErrorCode::Success);
}

extern "C" int ssgx_ocall_create_quote_data(const sgx_report_t* report, uint8_t** p_quote, uint32_t* quote_size) {
    int ret = 0;
    uint32_t quote_buffer_size = 0;
    uint8_t* quote_buffer = nullptr;
    quote3_error_t qe3_ret = SGX_QL_SUCCESS;

    if (!report || !quote_size) {
        return static_cast<int>(ErrorCode::InvalidParameter);
    }

    // Get quote data size
    qe3_ret = sgx_qe_get_quote_size(&quote_buffer_size);
    if (SGX_QL_SUCCESS != qe3_ret) {
        return static_cast<int>(ErrorCode::GetQuoteSizeFailed);
    }
    quote_buffer = static_cast<uint8_t*>(malloc(quote_buffer_size));
    if (quote_buffer == nullptr) {
        return static_cast<int>(ErrorCode::MallocFailed);
    }
    memset(quote_buffer, 0, quote_buffer_size);

    // Get the quote data
    qe3_ret = sgx_qe_get_quote(report, quote_buffer_size, quote_buffer);
    if (SGX_QL_SUCCESS != qe3_ret) {
        free(quote_buffer);
        return static_cast<int>(ErrorCode::GetQuoteDataFailed);
    }

    // Return
    *p_quote = quote_buffer;
    *quote_size = quote_buffer_size;
    return static_cast<int>(ErrorCode::Success);
}

extern "C" int ssgx_ocall_verify_quote_data(const uint8_t* quote_report, uint64_t quote_report_size,
                                 sgx_ql_qe_report_info_t* p_qve_report_info, time_t* expiration_check_date,
                                 uint32_t* collateral_expiration_status, sgx_ql_qv_result_t* quote_verification_result,
                                 uint8_t** p_supplemental_data, uint32_t* supplemental_data_size) {
    int ret = 0;
    time_t current_time = 0;
    quote3_error_t dcap_ret = SGX_QL_ERROR_UNEXPECTED;
    sgx_ql_qe_report_info_t qve_report_info = {0};
    tee_supp_data_descriptor_t supp_data = {0};
    supp_ver_t latest_ver;

    // Checking
    if (!quote_report || quote_report_size <= 0) {
        return static_cast<int>(ErrorCode::InvalidParameter);
    }
    if (!p_qve_report_info || !expiration_check_date || !collateral_expiration_status ||
        !quote_verification_result || !supplemental_data_size) {
        return static_cast<int>(ErrorCode::InvalidParameter);
    }
    memcpy(&qve_report_info, p_qve_report_info, sizeof(sgx_ql_qe_report_info_t));

    // Get supplement data size
    memset(&supp_data, 0, sizeof(tee_supp_data_descriptor_t));
    dcap_ret = tee_get_supplemental_data_version_and_size(quote_report, quote_report_size, &latest_ver.version,
                                                          &supp_data.data_size);
    if (dcap_ret != SGX_QL_SUCCESS) {
        return static_cast<int>(ErrorCode::GetSupplementSizeFailed);
    }
    if (supp_data.data_size != sizeof(sgx_ql_qv_supplemental_t)) {
        return static_cast<int>(ErrorCode::SupplementSizeIsWrong);
    }

    supp_data.p_data = (uint8_t*)malloc(supp_data.data_size);
    if (!supp_data.p_data) {
        return static_cast<int>(ErrorCode::MallocFailed);
    }
    memset(supp_data.p_data, 0, supp_data.data_size);

    // set current time. This is only for sample use, please use trusted time in product.
    //
    current_time = time(nullptr);

    // call DCAP quote verify library for quote verification
    // here you can choose 'trusted' or 'untrusted' quote verification by specifying parameter '&qve_report_info'
    // if '&qve_report_info' is NOT NULL, this API will call Intel QvE to verify quote
    // if '&qve_report_info' is NULL, this API will call 'untrusted quote verify lib' to verify quote, this mode doesn't
    // rely on SGX capable system, but the results can not be cryptographically authenticated
    dcap_ret = tee_verify_quote((uint8_t*)quote_report, (uint32_t)quote_report_size, nullptr, current_time,
                                collateral_expiration_status, quote_verification_result, &qve_report_info, &supp_data);
    if (dcap_ret != SGX_QL_SUCCESS) {
        free(supp_data.p_data);
        return static_cast<int>(ErrorCode::VerifyQuoteFailed);
    }

    *expiration_check_date = current_time;
    *supplemental_data_size = supp_data.data_size;
    *p_supplemental_data = supp_data.p_data;
    memcpy(p_qve_report_info, &qve_report_info, sizeof(sgx_ql_qe_report_info_t));

    return static_cast<int>(ErrorCode::Success);
}