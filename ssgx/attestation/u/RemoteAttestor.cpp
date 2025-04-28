#include <stdexcept>
#include <string.h>

#include "sgx_dcap_quoteverify.h"

#include "ssgx_attestation_u.h"

#include "crypto-suites/crypto-encode/base64.h"
#include "crypto-suites/crypto-encode/hex.h"
#include "crypto-suites/crypto-hash/sha256.h"
#include "auxiliary.h"

using namespace safeheron;
using namespace safeheron::hash;
using namespace safeheron::encode;

namespace ssgx {
namespace attestation_u {

static bool VerifyQuoteReport(const std::string& quote_report, std::string& enclave_id, std::string& err_msg);

bool RemoteAttestor::VerifyReport(const uint8_t user_data[64], const std::string& report, std::string& enclave_id) {
    int ret = 0;
    std::string internal_error;
    std::string quote_report;
    std::string enclave_id_in_report;
    sgx_quote3_t* p_quote = nullptr;

    error_msg_.clear();

    // Check parameters
    if (report.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = "Parameter report must be provided.";
        return false;
    }

    // Try to decode report from base64 string
    try {
        quote_report = base64::DecodeFromBase64(report);
    } catch (...) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = "Parameter report is an invalid Base64 string.";
        return false;
    }

    // Verify quote report by Intel DCAP service
    if (!VerifyQuoteReport(quote_report, enclave_id_in_report, internal_error)) {
        error_code_ = ErrorCode::VerifyQuoteFailed;
        error_msg_ = "Failed to call VerifyQuoteReport()!! Detail: " + internal_error;
        return false;
    }

    // Verify report_body.report_data.d
    p_quote = (sgx_quote3_t*)quote_report.c_str();
    if (memcmp(p_quote->report_body.report_data.d, user_data, 64) != 0) {
        error_code_ = ErrorCode::VerifyUserDataFailed;
        error_msg_ = "Failed to validate user data in report";
        return false;
    }

    enclave_id = enclave_id_in_report;
    error_code_ = ErrorCode::Success;

    return true;
}

bool RemoteAttestor::VerifyReport(const std::string& user_info, const std::string& report, std::string& enclave_id) {
    // Check parameters
    if (user_info.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = "Parameter user_info must be provided.";
        return false;
    }
    if (report.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = "Parameter report must be provided.";
        return false;
    }

    // Take the SHA256 digest value of user_info as user_data to verify report
    CSHA256 sha;
    uint8_t user_data[64] = {0};
    sha.Write((uint8_t*)user_info.c_str(), user_info.length());
    sha.Finalize(user_data);

    return VerifyReport(user_data, report, enclave_id);
}

bool RemoteAttestor::VerifyReport(const std::string& user_info, uint64_t timestamp, uint64_t validity_seconds,
                                  const std::string& report, std::string& enclave_id) {
    // user_info cannot be null
    if (user_info.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = "Parameter user_info must be provided.";
        return false;
    }

    // append timestamp to user_info and use the new one to verify report
    std::string time_str = std::to_string(timestamp);
    std::string new_user_info = user_info + "&time=" + time_str;
    if (VerifyReport(new_user_info, report, enclave_id)) {
        uint64_t time_now = time(nullptr);
        if (time_now - timestamp > validity_seconds) {
            error_msg_ = "Report is expired!";
            error_code_ = ErrorCode::VerifyTimeStampFailed;
            return false;
        }
        return true;
    }
    return false;
}

bool VerifyQuoteReport(const std::string& quote_report, std::string& enclave_id, std::string& err_msg) {
    bool success = false;
    time_t current_time = 0;
    uint32_t collateral_expiration_status = 1;
    sgx_ql_qv_result_t quote_verification_result = SGX_QL_QV_RESULT_UNSPECIFIED;
    quote3_error_t dcap_ret = SGX_QL_ERROR_UNEXPECTED;
    tee_supp_data_descriptor_t supp_data = {0};
    supp_ver_t latest_ver = {0};
    sgx_attributes_t attributes = {0};
    sgx_quote3_t* p_quote = nullptr;

    if (quote_report.empty()) {
        err_msg = "Parameter quote_report is null!";
        return false;
    }

    // call DCAP quote verify library to get supplemental data size
    dcap_ret = tee_get_supplemental_data_version_and_size((uint8_t*)quote_report.c_str(), quote_report.length(),
                                                          &latest_ver.version, &supp_data.data_size);
    if (dcap_ret != SGX_QL_SUCCESS) {
        err_msg = "Failed to call tee_get_supplemental_data_version_and_size()! ret: " + std::to_string(dcap_ret);
        return false;
    }
    if (supp_data.data_size != sizeof(sgx_ql_qv_supplemental_t)) {
        err_msg = "supp_data.data_size is wrong, supp_data.data_size: " + std::to_string(supp_data.data_size);
        return false;
    }

    supp_data.p_data = static_cast<uint8_t*>(malloc(supp_data.data_size));
    if (supp_data.p_data == nullptr) {
        err_msg = "Failed to malloc supp_data.p_data, supp_data.data_size: " + std::to_string(supp_data.data_size);
        return false;
    }
    memset(supp_data.p_data, 0, supp_data.data_size);

    // set current time. This is only for sample use, please use trusted time in product.
    current_time = time(nullptr);

    // call DCAP quote verify library for quote verification
    // here you can choose 'trusted' or 'untrusted' quote verification by specifying parameter '&qve_report_info'
    // if '&qve_report_info' is NOT NULL, this API will call Intel QvE to verify quote
    // if '&qve_report_info' is NULL, this API will call 'untrusted quote verify lib' to verify quote, this mode doesn't
    // rely on SGX capable system, but the results can not be cryptographically authenticated
    dcap_ret = tee_verify_quote((uint8_t*)quote_report.c_str(), quote_report.length(), nullptr, current_time,
                                &collateral_expiration_status, &quote_verification_result, nullptr, &supp_data);

    // Free buffer
    free(supp_data.p_data);

    // check result
    if (dcap_ret != SGX_QL_SUCCESS) {
        err_msg = "Failed to call tee_verify_quote()! ret: " + std::to_string(dcap_ret);
        return false;
    }

    // Is a debug report?
    p_quote = (sgx_quote3_t*)quote_report.c_str();
    attributes = p_quote->report_body.attributes;
    if (attributes.flags & SGX_FLAGS_DEBUG) {
        err_msg = "The enclave that generate this report is in Debug Mode.";
        return false;
    }

    // Check verification result
    switch (quote_verification_result) {
    case SGX_QL_QV_RESULT_OK:
        // check verification collateral expiration status
        // this value should be considered in your own attestation/verification policy
        //
        if (collateral_expiration_status == 0) {
            // Verification completed successfully.
            enclave_id = hex::EncodeToHex(p_quote->report_body.mr_enclave.m, 32);
            success = true;
        } else {
            err_msg =
                "Verification completed, but collateral is out of date based on 'expiration_check_date' you provided.";
            success = false;
        }
        break;
    case SGX_QL_QV_RESULT_CONFIG_NEEDED:
    case SGX_QL_QV_RESULT_OUT_OF_DATE:
    case SGX_QL_QV_RESULT_OUT_OF_DATE_CONFIG_NEEDED:
    case SGX_QL_QV_RESULT_SW_HARDENING_NEEDED:
    case SGX_QL_QV_RESULT_CONFIG_AND_SW_HARDENING_NEEDED:
        err_msg = "Verification completed with Non-terminal ret: " + std::to_string(quote_verification_result);
        success = false;
        break;
    case SGX_QL_QV_RESULT_INVALID_SIGNATURE:
    case SGX_QL_QV_RESULT_REVOKED:
    case SGX_QL_QV_RESULT_UNSPECIFIED:
    default:
        err_msg = "Verification completed with Terminal ret: " + std::to_string(quote_verification_result);
        success = false;
        break;
    }

    return success;
}

} // namespace attestation_u
} // namespace ssgx
