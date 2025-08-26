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

static inline sgx_ql_qv_result_t FromQvResult(QvResult val) {
    switch (val) {
    case QvResult::Ok:
        return SGX_QL_QV_RESULT_OK;
    case QvResult::ConfigNeeded:
        return SGX_QL_QV_RESULT_CONFIG_NEEDED;
    case QvResult::OutOfDate:
        return SGX_QL_QV_RESULT_OUT_OF_DATE;
    case QvResult::OutOfDateConfigNeeded:
        return SGX_QL_QV_RESULT_OUT_OF_DATE_CONFIG_NEEDED;
    case QvResult::InvalidSignature:
        return SGX_QL_QV_RESULT_INVALID_SIGNATURE;
    case QvResult::Revoked:
        return SGX_QL_QV_RESULT_REVOKED;
    case QvResult::Unspecified:
        return SGX_QL_QV_RESULT_UNSPECIFIED;
    case QvResult::SwHardeningNeeded:
        return SGX_QL_QV_RESULT_SW_HARDENING_NEEDED;
    case QvResult::ConfigAndSwHardeningNeeded:
        return SGX_QL_QV_RESULT_CONFIG_AND_SW_HARDENING_NEEDED;
    case QvResult::TDRelaunchAdvised:
        return SGX_QL_QV_RESULT_TD_RELAUNCH_ADVISED;
    case QvResult::TDRelaunchAdvisedConfigNeeded:
        return SGX_QL_QV_RESULT_TD_RELAUNCH_ADVISED_CONFIG_NEEDED;
    }
    throw std::runtime_error("Invalid QvResult value");
}

static ErrorCode VerifyRawQuote(const std::string& quote_report, std::string& out_mrenclave_hex,
                                std::optional<uint32_t>& out_qv_result,
                                uint32_t& out_collateral_expiration_status, std::string& out_err_msg);

void RemoteAttestor::SetAcceptableResults(std::initializer_list<QvResult> accepted_results) {
    accepted_qv_results_.clear();

    for (const auto& result : accepted_results) {
        accepted_qv_results_.insert(FromQvResult(result));
    }

    // If empty set is passed, fallback to strict default
    if (accepted_qv_results_.empty()) { accepted_qv_results_.insert(static_cast<uint32_t>(QvResult::Ok)); }
}

bool RemoteAttestor::VerifyReport(const uint8_t user_data[64], const std::string& report, std::string& mrenclave_hex) {
    int ret = 0;
    std::string internal_error;
    std::string quote_report;
    std::string mrenclave_hex_in_report;
    sgx_quote3_t* p_quote = nullptr;

    qv_result_ = std::nullopt;

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

    uint32_t collateral_expiration_status = 1;
    auto vry_err_code = VerifyRawQuote(quote_report, mrenclave_hex_in_report, qv_result_,
                                       collateral_expiration_status, internal_error);
    if (vry_err_code != ErrorCode::Success) {
        error_code_ = vry_err_code;
        error_msg_ = std::string("Failed to call VerifyRawQuoteWithQvE()! Detail: ") + internal_error;
        return false;
    } else if (!qv_result_.has_value()) {
        error_code_ = ErrorCode::Unknown;
        error_msg_ = "Failed to call VerifyRawQuoteWithQvE()! Detail: unknown error";
        return false;
    } else if (accepted_qv_results_.count(qv_result_.value()) == 0) {
        // If qv_result_ is not accepted
        error_code_ = ErrorCode::VerifyQuoteFailed;
        error_msg_ =
            std::string("Failed to call VerifyRawQuoteWithQvE()! Detail: rejected QvResult, ") + std::to_string( qv_result_.value() );
        return false;
    }

    // check verification collateral expiration status
    // this value should be considered in your own attestation/verification policy
    if (collateral_expiration_status != 0) {
        error_code_ = ErrorCode::CollateralExpired;
        error_msg_ =
            "Verification completed, but collateral is out of date based on 'expiration_check_date' you provided.";
        return false;
    }

    // Verify report_body.report_data.d
    p_quote = (sgx_quote3_t*)quote_report.c_str();
    if (memcmp(p_quote->report_body.report_data.d, user_data, 64) != 0) {
        error_code_ = ErrorCode::VerifyUserDataFailed;
        error_msg_ = "Failed to validate user data in report";
        return false;
    }

    mrenclave_hex = mrenclave_hex_in_report;
    error_code_ = ErrorCode::Success;

    return true;
}

bool RemoteAttestor::VerifyReport(const std::string& user_info, const std::string& report, std::string& mrenclave_hex) {
    qv_result_ = std::nullopt;

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

    return VerifyReport(user_data, report, mrenclave_hex);
}

bool RemoteAttestor::VerifyReport(const std::string& user_info, uint64_t timestamp, uint64_t validity_seconds,
                                  const std::string& report, std::string& mrenclave_hex) {
    qv_result_ = std::nullopt;

    // user_info cannot be null
    if (user_info.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = "Parameter user_info must be provided.";
        return false;
    }

    // append timestamp to user_info and use the new one to verify report
    std::string time_str = std::to_string(timestamp);
    std::string new_user_info = user_info + "&time=" + time_str;
    if (VerifyReport(new_user_info, report, mrenclave_hex)) {
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

ErrorCode VerifyRawQuote(const std::string& quote_report, std::string& out_mrenclave_hex,
                         std::optional<uint32_t>& out_qv_result,
                         uint32_t& out_collateral_expiration_status, std::string& out_err_msg) {
    time_t current_time = 0;
    sgx_ql_qv_result_t quote_verification_result = SGX_QL_QV_RESULT_UNSPECIFIED;
    quote3_error_t dcap_ret = SGX_QL_ERROR_UNEXPECTED;
    tee_supp_data_descriptor_t supp_data = {0};
    supp_ver_t latest_ver = {0};
    sgx_attributes_t attributes = {0};
    sgx_quote3_t* p_quote = nullptr;

    if (quote_report.empty()) {
        out_err_msg = "Parameter quote_report is null!";
        return ErrorCode::InvalidParameter;
    }

    // call DCAP quote verify library to get supplemental data size
    dcap_ret = tee_get_supplemental_data_version_and_size((uint8_t*)quote_report.c_str(), quote_report.length(),
                                                          &latest_ver.version, &supp_data.data_size);
    if (dcap_ret != SGX_QL_SUCCESS) {
        out_err_msg = "Failed to call tee_get_supplemental_data_version_and_size()! ret: " + std::to_string(dcap_ret);
        return ErrorCode::GetSupplementSizeCallException;
    }
    if (supp_data.data_size != sizeof(sgx_ql_qv_supplemental_t)) {
        out_err_msg = "supp_data.data_size is wrong, supp_data.data_size: " + std::to_string(supp_data.data_size);
        return ErrorCode::GetSupplementSizeFailed;
    }

    supp_data.p_data = static_cast<uint8_t*>(malloc(supp_data.data_size));
    if (supp_data.p_data == nullptr) {
        out_err_msg = "Failed to malloc supp_data.p_data, supp_data.data_size: " + std::to_string(supp_data.data_size);
        return ErrorCode::MallocFailed;
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
                                &out_collateral_expiration_status, &quote_verification_result, nullptr, &supp_data);

    // Free buffer
    free(supp_data.p_data);

    // check result
    if (dcap_ret != SGX_QL_SUCCESS) {
        out_err_msg = "Failed to call tee_verify_quote()! ret: " + std::to_string(dcap_ret);
        return ErrorCode::QvEVerificationCallFailed;
    }

    // Is a debug report?
    p_quote = (sgx_quote3_t*)quote_report.c_str();
    attributes = p_quote->report_body.attributes;
    if (attributes.flags & SGX_FLAGS_DEBUG) {
        out_err_msg = "The enclave that generate this report is in Debug Mode.";
        return ErrorCode::EnclaveInDebugMode;
    }

    out_qv_result = quote_verification_result;
    out_mrenclave_hex = hex::EncodeToHex(p_quote->report_body.mr_enclave.m, 32);
    return ErrorCode::Success;
}

} // namespace attestation_u
} // namespace ssgx
