#ifndef SAFEHERON_SGX_TRUSTED_ATTESTATION_T_H_
#define SAFEHERON_SGX_TRUSTED_ATTESTATION_T_H_

#include <string>

namespace ssgx {
/**
 * @namespace ssgx::attestation_t
 * @brief This module is designed for generating and verifying remote attestation reports within a Trusted Execution
 * Environment (TEE).
 */
namespace attestation_t {

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
    GetTargetInfoFailed = 2,
    CreateReportFailed = 3,
    OCallOperationException = 4,
    OCallOperationFailed = 5,
    BufferValidateFailed = 6,
    VerifyQuoteFailed = 7,
    VerifyUserDataFailed = 8,
    VerifyTimeStampFailed = 9,

    Unknown = 9999
};

/**
 *  @brief The class for Intel DCAP remote attestation
 *
 *  This class provides APIs to Create reports and Verify reports for Intel SGX Enclave applications
 *
 */
class RemoteAttestor {
  public:
    RemoteAttestor() {
        error_code_ = ErrorCode::Unknown;
    }

  public:
    /**
     * @brief Generate a remote attestation report.
     * @param[in] user_data User-defined data, totaling 64 bytes, will be included in the remote attestation report.
     * @param[out] report remote attestation report
     * @return Return true if successful; otherwise, return false
     */
    bool CreateReport(const uint8_t user_data[64], std::string& report);

    /**
     * @brief Verify a remote attestation report within a Trusted Execution Environment
     * @param[in] user_data User-defined data utilized in the generation of a remote attestation report.
     * @param[in] report remote attestation report
     * @param[out] enclave_id MRENCLAVE in the remote attestation report.
     * @return Return true if successful; otherwise, return false
     */
    bool VerifyReport(const uint8_t user_data[64], const std::string& report, std::string& enclave_id);

    /**
     * @brief Generate a remote attestation report.
     * @param[in] user_info User-defined data of unrestricted length, whose SHA-256 hash will be included in the remote
     * attestation report.
     * @param[out] report remote attestation report
     * @return Return true if successful; otherwise, return false
     */
    bool CreateReport(const std::string& user_info, std::string& report);

    /**
     * @brief Verify a remote attestation report within a Trusted Execution Environment
     * @param[in] user_info User-defined data utilized in the generation of a remote attestation report.
     * @param[in] report remote attestation report
     * @param[out] enclave_id MRENCLAVE in the remote attestation report.
     * @return Return true if successful; otherwise, return false
     */
    bool VerifyReport(const std::string& user_info, const std::string& report, std::string& enclave_id);

    /**
     * @brief Generate a remote attestation report.
     * @param[in] user_info User-defined data of unrestricted length, concatenated with timestamp, will have its SHA-256
     * hash included in the remote attestation report.
     * @param[in] timestamp The current timestamp, concatenated with user-defined data, will have its SHA-256 hash
     * included in the remote attestation report.
     * @param[out] report remote attestation report
     * @return Return true if successful; otherwise, return false
     */
    bool CreateReport(const std::string& user_info, uint64_t timestamp, std::string& report);

    /**
     * @brief Verify a remote attestation report within a Trusted Execution Environment
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
     * @brief Get the last error code
     * @return an error code
     */
    ErrorCode GetLastErrorCode() const {
        return error_code_;
    };

    /**
     * @brief Get the last error message
     * @return an error message
     */
    std::string GetLastErrorMsg() const {
        return error_msg_;
    }

  private:
    ErrorCode error_code_;
    std::string error_msg_;
};

} // namespace attestation_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_ATTESTATION_T_H_