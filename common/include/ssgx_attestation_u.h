#ifndef SAFEHERON_SGX_UNTRUSTED_ATTESTATION_T_H_
#define SAFEHERON_SGX_UNTRUSTED_ATTESTATION_T_H_

#include <string>

namespace ssgx {
/**
 * @namespace ssgx::attestation_u
 * @brief This module is designed for verifying remote attestation reports in a untrusted execution environment.
 */
namespace attestation_u {

/**
 *  @brief Error codes definition
 *
 *  When an operation is failed, call GetLastErrorCode() to receive
 *  one of bellow error codes.
 *
 */
enum class ErrorCode {
    Success = 0,
    InvalidParameter = 1,
    InitializationFailed = 2,
    GetTargetInfoFailed = 3,
    GetQuoteSizeFailed = 4,
    GetQuoteDataFailed = 5,
    MallocFailed = 6,
    SetLoadPolicyFailed = 7,
    GetSupplementSizeFailed = 8,
    SupplementSizeIsWrong = 9,
    VerifyQuoteFailed = 10,
    VerifyUserDataFailed = 11,
    VerifyTimeStampFailed = 12,

    Unknown = 999
};

/**
 *  @brief The class for Intel DCAP remote attestation
 *
 *  This class provides APIs to Verify reports
 *
 */
class RemoteAttestor {
  public:
    RemoteAttestor() {
        error_code_ = ErrorCode::Unknown;
    }

  public:
    /**
     * @brief Verify a remote attestation report within a Untrusted Execution Environment
     * @param[in] user_data User-defined data utilized in the generation of a remote attestation report.
     * @param[in] report remote attestation report
     * @param[out] enclave_id MRENCLAVE in the remote attestation report.
     * @return Return true if successful; otherwise, return false
     */
    bool VerifyReport(const uint8_t user_data[64], const std::string& report, std::string& enclave_id);

    /**
     * @brief Verify a remote attestation report within a Untrusted Execution Environment
     * @param[in] user_info User-defined data utilized in the generation of a remote attestation report.
     * @param[in] report remote attestation report
     * @param[out] enclave_id MRENCLAVE in the remote attestation report.
     * @return Return true if successful; otherwise, return false
     */
    bool VerifyReport(const std::string& user_info, const std::string& report, std::string& enclave_id);

    /**
     * @brief Verify a remote attestation report within a Untrusted Execution Environment
     * @param[in] user_info User-defined data utilized in the generation of a remote attestation report.
     * @param[in] timestamp Timestamp used in the generation of a remote attestation report.
     * @param[in] validity_seconds If timestamp + validity_seconds < current_timestamp, the remote attestation report
     * and User-defined data are considered expired.
     * @param[in] report remote attestation report
     * @param[out] enclave_id MRENCLAVE in the remote attestation report.
     * @return Return true if successful; otherwise, return false
     */
    bool VerifyReport(const std::string& user_info, uint64_t timestamp, uint64_t validity_seconds,
                      const std::string& report, std::string& enclave_id);

    /**
     * @brief Get the error code
     * @return An error code
     */
    ErrorCode GetLastErrorCode() const {
        return error_code_;
    };

    /**
     * @brief Get the error message
     * @return An error message string
     */
    std::string GetLastErrorMsg() const {
        return error_msg_;
    };

  private:
    ErrorCode error_code_;
    std::string error_msg_;
};

} // namespace attestation_u
} // namespace ssgx

#endif // SAFEHERON_SGX_UNTRUSTED_ATTESTATION_T_H_