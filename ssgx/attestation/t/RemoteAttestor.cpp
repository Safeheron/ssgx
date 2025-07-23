#include <sgx_lfence.h>
#include <stdexcept>
#include <string>

#include "sgx_ql_quote.h"
#include "sgx_trts.h"
#include "sgx_utils.h"

#include "ssgx_attestation_t.h"
#include "ssgx_attestation_t_t.h"
#include "ssgx_utils_t.h"

#include "crypto-suites/crypto-bn/rand.h"
#include "crypto-suites/crypto-encode/base64.h"
#include "crypto-suites/crypto-encode/hex.h"
#include "crypto-suites/crypto-hash/sha256.h"

using namespace safeheron;
using namespace safeheron::hash;
using namespace safeheron::encode;
using namespace ssgx::utils_t;

namespace ssgx {
namespace attestation_t {

static ErrorCode VerifyRawQuoteWithQvE(const std::string& quote_report, std::string& out_mrenclave_hex,
                                       std::optional<uint32_t>& out_qv_result,
                                       uint32_t& out_collateral_expiration_status, std::string& out_err_msg);

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

RemoteAttestor::RemoteAttestor()
    : error_code_(ErrorCode::Unknown), qv_result_(std::nullopt), accepted_qv_results_({SGX_QL_QV_RESULT_OK}) {
}

void RemoteAttestor::SetAcceptableResults(std::initializer_list<QvResult> accepted_results) {
    accepted_qv_results_.clear();

    for (const auto& result : accepted_results) {
        accepted_qv_results_.insert(FromQvResult(result));
    }

    // If empty set is passed, fallback to strict default
    if (accepted_qv_results_.empty()) { accepted_qv_results_.insert(static_cast<uint32_t>(QvResult::Ok)); }
}

bool RemoteAttestor::CreateReport(const uint8_t user_data[64], std::string& report) {
    int ret = 0;
    sgx_status_t status = SGX_SUCCESS;
    sgx_target_info_t qe_target_info = {0};
    sgx_report_data_t report_data = {0};
    sgx_report_t sgx_report = {0};
    uint8_t* p_quote_data = nullptr;
    uint32_t quote_data_size = 0;

    error_msg_.clear();

    // Get QE target information
    status = ssgx_ocall_get_qe_target_info(&ret, &qe_target_info);
    if (status != SGX_SUCCESS) {
        error_code_ = ErrorCode::OCallOperationFailed;
        error_msg_ = FormatStr("Failed to call ssgx_ocall_get_qe_target_info(), sgx_status: 0x%x", status);
        return false;
    }
    if (ret != 0) {
        error_code_ = ErrorCode::GetTargetInfoFailed;
        error_msg_ = FormatStr("Failed to call ssgx_ocall_get_qe_target_info(), ret: %d", ret);
        return false;
    }

    // Set report_body.report_data.d,
    // which is a 64 bytes data specified by user
    // report_body.report_data.d will be validated when verify report
    memcpy(report_data.d, user_data, 64);

    // Create self report
    status = sgx_create_report(&qe_target_info, &report_data, &sgx_report);
    if (status != SGX_SUCCESS) {
        error_code_ = ErrorCode::CreateReportFailed;
        error_msg_ = FormatStr("Failed to call sgx_create_report(), sgx_status: 0x%x", status);
        return false;
    }

    // Create QE quote data
    status = ssgx_ocall_create_quote_data(&ret, &sgx_report, &p_quote_data, &quote_data_size);
    if (status != SGX_SUCCESS) {
        error_code_ = ErrorCode::OCallOperationException;
        error_msg_ = FormatStr("Failed to call ssgx_ocall_create_quote_data(), sgx_status: 0x%x", status);
        return false;
    }
    if (ret != 0) {
        error_code_ = ErrorCode::OCallOperationFailed;
        error_msg_ = FormatStr("Failed to call ssgx_ocall_create_quote_data(), ret: %d", ret);
        return false;
    }

    // Validate p_quote_data is outside of enclave or not
    if (sgx_is_outside_enclave(p_quote_data, quote_data_size) == 0) {
        // Don't call FreeOutside() to release p_quote_data in this case, due to this is an exception case.
        error_code_ = ErrorCode::BufferValidateFailed;
        error_msg_ = "Failed to validate buffer p_quote_data, it is not outside of enclave.";
        return false;
    }

    sgx_lfence();

    // Return report
    report = base64::EncodeToBase64(p_quote_data, quote_data_size);
    FreeOutside(p_quote_data, quote_data_size);
    return true;
}

bool RemoteAttestor::VerifyReport(const uint8_t user_data[64], const std::string& report, std::string& mrenclave_hex) {
    std::string internal_error;
    std::string quote_report;
    std::string mrenclave_hex_in_report;
    sgx_quote3_t* p_quote = nullptr;

    qv_result_ = std::nullopt;

    error_msg_.clear();

    // Check parameters
    if (report.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = FormatStr("Parameter report is null!");
        return false;
    }

    // Try to decode report from base64 string
    try {
        quote_report = base64::DecodeFromBase64(report);
    } catch (...) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = FormatStr("Parameter report is an invalid Base64 string.");
        return false;
    }

    // Verify quote report by Intel DCAP service
    uint32_t collateral_expiration_status = 1;
    auto vry_err_code = VerifyRawQuoteWithQvE(quote_report, mrenclave_hex_in_report, qv_result_,
                                              collateral_expiration_status, internal_error);
    if (vry_err_code != ErrorCode::Success) {
        error_code_ = vry_err_code;
        error_msg_ = FormatStr("Failed to call VerifyRawQuoteWithQvE()! Detail: %s", internal_error.c_str());
        return false;
    } else if (!qv_result_.has_value()) {
        error_code_ = ErrorCode::Unknown;
        error_msg_ = FormatStr("Failed to call VerifyRawQuoteWithQvE()! Detail: unknown error");
        return false;
    } else if (accepted_qv_results_.count(qv_result_.value()) == 0) {
        // If qv_result_ is not accepted
        error_code_ = ErrorCode::VerifyQuoteFailed;
        error_msg_ =
            FormatStr("Failed to call VerifyRawQuoteWithQvE()! Detail: rejected QvResult, 0x%x", qv_result_.value());
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
        error_msg_ = FormatStr("Failed to validate user data in report");
        return false;
    }

    mrenclave_hex = mrenclave_hex_in_report;
    error_code_ = ErrorCode::Success;

    return true;
}

bool RemoteAttestor::CreateReport(const std::string& user_info, std::string& report) {
    // user_info must be provided
    if (user_info.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = FormatStr("Parameter user_info must be provided.");
        return false;
    }

    // Take the SHA256 digest value of user_info as user_data to create report
    CSHA256 sha;
    uint8_t user_data[64] = {0};
    sha.Write((uint8_t*)user_info.c_str(), user_info.length());
    sha.Finalize(user_data);

    return CreateReport(user_data, report);
}

bool RemoteAttestor::VerifyReport(const std::string& user_info, const std::string& report, std::string& mrenclave_hex) {
    qv_result_ = std::nullopt;

    // Check parameters
    if (user_info.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = FormatStr("Parameter user_info must be provided.");
        return false;
    }
    if (report.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = FormatStr("Parameter report must be provided.");
        return false;
    }

    // Take the SHA256 digest value of user_info as user_data to verify report
    CSHA256 sha;
    uint8_t user_data[64] = {0};
    sha.Write((uint8_t*)user_info.c_str(), user_info.length());
    sha.Finalize(user_data);

    return VerifyReport(user_data, report, mrenclave_hex);
}

bool RemoteAttestor::CreateReport(const std::string& user_info, uint64_t timestamp, std::string& report) {
    // user_info cannot be null
    if (user_info.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = FormatStr("Parameter user_info must be provided.");
        return false;
    }

    // append timestamp to user_info and use the new one to create report
    std::string time_str = std::to_string(timestamp);
    std::string new_user_info = user_info + "&time=" + time_str;
    return CreateReport(new_user_info, report);
}

bool RemoteAttestor::VerifyReport(const std::string& user_info, uint64_t timestamp, uint64_t validity_seconds,
                                  const std::string& report, std::string& mrenclave_hex) {
    qv_result_ = std::nullopt;

    // user_info cannot be null
    if (user_info.empty()) {
        error_code_ = ErrorCode::InvalidParameter;
        error_msg_ = FormatStr("Parameter user_info must be provided.");
        return false;
    }

    // append timestamp to user_info and use the new one to verify report
    std::string time_str = std::to_string(timestamp);
    std::string new_user_info = user_info + "&time=" + time_str;
    if (VerifyReport(new_user_info, report, mrenclave_hex)) {
        uint64_t time_now = ssgx::utils_t::DateTime::Now().GetTimestamp();
        if (time_now - timestamp > validity_seconds) {
            error_msg_ = FormatStr("Report is expired!");
            error_code_ = ErrorCode::VerifyTimeStampFailed;
            return false;
        }
        return true;
    }
    return false;
}

ErrorCode VerifyRawQuoteWithQvE(const std::string& quote_report, std::string& out_mrenclave_hex,
                                std::optional<uint32_t>& out_qv_result, uint32_t& collateral_expiration_status,
                                std::string& err_msg) {
    int ret = 0;
    ErrorCode success = ErrorCode::Success;
    sgx_status_t status = SGX_SUCCESS;
    time_t current_time = 0;
    uint32_t supplemental_buff_size = 0;
    uint8_t* supplemental_buff = nullptr;
    uint8_t* supplemental_inside_buff = nullptr;

    sgx_ql_qe_report_info_t qve_report_info = {0};
    sgx_ql_qv_result_t quote_verification_result = SGX_QL_QV_RESULT_UNSPECIFIED;
    quote3_error_t verify_qveid_ret = SGX_QL_ERROR_UNEXPECTED;
    sgx_isv_svn_t qve_isvsvn_threshold = 0;
    sgx_attributes_t attributes = {0};
    sgx_quote3_t* p_quote = nullptr;
    uint8_t rand_nonce[16] = {0};

    if (quote_report.empty()) {
        err_msg = "Parameter quote_report is null!";
        return ErrorCode::InvalidParameter;
    }

    // Set nonce
    rand::RandomBytes(rand_nonce, 16);
    memcpy(qve_report_info.nonce.rand, rand_nonce, sizeof(rand_nonce));

    status = sgx_self_target(&qve_report_info.app_enclave_target_info);
    if (status != SGX_SUCCESS) {
        err_msg = FormatStr("Failed to call sgx_self_target(), sgx_status: 0x%x", status);
        return ErrorCode::GetTargetInfoFailed;
    }

    status = ssgx_ocall_verify_quote_data(&ret, (uint8_t*)quote_report.c_str(), (uint32_t)quote_report.size(),
                                          &qve_report_info, &current_time, &collateral_expiration_status,
                                          &quote_verification_result, &supplemental_buff, &supplemental_buff_size);
    if (status != SGX_SUCCESS) {
        err_msg = FormatStr("Failed to call ssgx_ocall_verify_quote_data(), sgx_status: 0x%x", status);
        return ErrorCode::OCallOperationException;
    }
    if (ret != 0) {
        err_msg = FormatStr("Failed to call ssgx_ocall_verify_quote_data(), ret: %d", ret);
        return ErrorCode::OCallOperationFailed;
    }

    // Validate p_quote_data is outside of enclave or not
    if (sgx_is_outside_enclave(supplemental_buff, supplemental_buff_size) == 0) {
        // Don't call FreeOutside() to release supplemental_buff in this case, due to this is an exception case.
        err_msg = "Failed to validate buffer p_quote_data, it is not outside of enclave.";
        return ErrorCode::BufferValidateFailed;
    }

    // copy supplemental data into Enclave, because sgx_tvl_verify_qve_report_and_identity() requests
    // all parameters are inside of Enclave.
    if ((supplemental_buff != nullptr) && supplemental_buff_size > 0) {
        supplemental_inside_buff = static_cast<uint8_t*>(malloc(supplemental_buff_size));
        memcpy(supplemental_inside_buff, supplemental_buff, supplemental_buff_size);
        FreeOutside(supplemental_buff, supplemental_buff_size);
    }

    sgx_lfence();

    p_quote = (sgx_quote3_t*)quote_report.c_str();
    attributes = p_quote->report_body.attributes;
    if (attributes.flags & SGX_FLAGS_DEBUG) {
        free(supplemental_inside_buff);
        err_msg = "The enclave that generate this report is in Debug Mode.";
        return ErrorCode::EnclaveInDebugMode;
    }

    // Call sgx_dcap_tvl API in SampleISVEnclave to verify QvE's report and identity
    qve_isvsvn_threshold = 6;
    verify_qveid_ret = sgx_tvl_verify_qve_report_and_identity(
        (uint8_t*)quote_report.c_str(), (uint32_t)quote_report.size(), &qve_report_info, current_time,
        collateral_expiration_status, quote_verification_result, supplemental_inside_buff, supplemental_buff_size,
        qve_isvsvn_threshold);

    // Free buffer
    free(supplemental_inside_buff);

    // Check the result
    if (verify_qveid_ret != SGX_QL_SUCCESS) {
        err_msg = FormatStr("Failed to call sgx_tvl_verify_qve_report_and_identity(), verify_qveid_ret: 0x%x",
                            verify_qveid_ret);
        return ErrorCode::QvEVerificationCallFailed;
    }

    out_qv_result = quote_verification_result;
    return ErrorCode::Success;
}

} // namespace attestation_t
} // namespace ssgx
