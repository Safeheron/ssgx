#ifndef SAFEHERON_SGX_TRUSTED_ATTESTATION_T_H_
#define SAFEHERON_SGX_TRUSTED_ATTESTATION_T_H_

#include <optional>
#include <unordered_set>
#include <string>

namespace ssgx {
/**
 * @namespace ssgx::attestation_t
 * @brief This module is designed for generating and verifying remote attestation reports within a Trusted Execution
 * Environment (TEE).
 */
namespace attestation_t {

/**
 * @brief Application-defined result codes for remote attestation verification.
 */
enum class ErrorCode : uint32_t {
    /// All verification steps passed successfully.
    Success = 0x0000,

    /// Invalid parameter passed to a verification or report generation function.
    InvalidParameter = 0x0001,

    /// Failed to retrieve target info from the enclave.
    GetTargetInfoFailed = 0x0002,

    /// Failed to generate a report from the enclave.
    CreateReportFailed = 0x0003,

    /// An exception occurred while performing an OCall.
    OCallOperationException = 0x0004,

    /// An OCall completed but returned a failure result.
    OCallOperationFailed = 0x0005,

    /// One or more input buffers failed validation (e.g., size, alignment, or null checks).
    BufferValidateFailed = 0x0006,

    /// Quote verification logic failed internally (e.g., malformed report, unparseable quote).
    VerifyQuoteFailed = 0x0007,

    /// The embedded user data in the quote did not match the expected value.
    VerifyUserDataFailed = 0x0008,

    /// The timestamp in the attestation report is missing, invalid, or has expired.
    VerifyTimeStampFailed = 0x0009,

    /// The enclave is in debug mode, which may not be trusted in production.
    EnclaveInDebugMode = 0x000A,

    /// Failed to verify QvE’s identity or report using sgx_tvl_verify_qve_report_and_identity.
    QvEVerificationCallFailed = 0x000B,

    /// The collateral used to verify the quote has expired or is invalid.
    CollateralExpired = 0x000C,

    /// Failed to initialize the verification environment or components.
    InitializationFailed = 0x000D,

    /// Failed to retrieve the required quote size.
    GetQuoteSizeFailed = 0x000E,

    /// Failed to obtain the actual quote data.
    GetQuoteDataFailed = 0x000F,

    /// Failed to allocate memory inside or outside the enclave.
    MallocFailed = 0x0010,

    /// Failed to set the quote provider loading policy.
    SetLoadPolicyFailed = 0x0011,

    /// Failed to retrieve the supplemental data size.
    GetSupplementSizeFailed = 0x0012,

    /// The actual size of supplemental data did not match the expected value.
    SupplementSizeIsWrong = 0x0013,

    /// An unspecified or unexpected error occurred.
    Unknown = 0xFFFF
};


/**
 * @brief Combined Quote Verification Result and Internal Error Codes.
 *
 * This enum merges the SGX Quote Verification Library (QVL) results from
 * The value corresponds directly to the `sgx_ql_qv_result_t` enum defined in the Intel SGX DCAP library.
 * These result codes are declared in:
 * https://github.com/intel/SGXDataCenterAttestationPrimitives/blob/main/QuoteVerification/QvE/Include/sgx_qve_header.h
 *
 * SGX QVL results are defined in the range 0x0000 ~ 0x00FF:
 *  - e.g., Ok (0x0000), ConfigNeeded (0x0001), etc.
 *
 * This clear separation helps to distinguish errors originating from SGX verification
 * versus errors caused by internal library processing.
 */
enum class QvResult : uint32_t {
    // SGX Quote Verification Library error codes
    /// Corresponds to SGX_QL_QV_RESULT_OK.
    /// Quote verification passed and is at the latest TCB level.
    Ok = 0x0000,

    /// Corresponds to SGX_QL_QV_RESULT_CONFIG_NEEDED.
    /// Quote verification passed, but additional configuration of the platform may be needed.
    ConfigNeeded = 0x0001,

    /// Corresponds to SGX_QL_QV_RESULT_OUT_OF_DATE.
    /// Quote is good, but the platform's TCB level is out of date and patching is required.
    OutOfDate = 0x0002,

    /// Corresponds to SGX_QL_QV_RESULT_OUT_OF_DATE_CONFIG_NEEDED.
    /// Quote is good, but both patching and additional configuration of the platform are needed.
    OutOfDateConfigNeeded = 0x0003,

    /// Corresponds to SGX_QL_QV_RESULT_INVALID_SIGNATURE.
    /// The quote signature is invalid.
    InvalidSignature = 0x0004,

    /// Corresponds to SGX_QL_QV_RESULT_REVOKED.
    /// The platform or one of its components has been revoked.
    Revoked = 0x0005,

    /// Corresponds to SGX_QL_QV_RESULT_UNSPECIFIED.
    /// The verification result is unspecified or indicates a general failure.
    Unspecified = 0x0006,

    /// Corresponds to SGX_QL_QV_RESULT_SW_HARDENING_NEEDED.
    /// TCB is up to date, but software hardening is required.
    SwHardeningNeeded = 0x0007,

    /// Corresponds to SGX_QL_QV_RESULT_CONFIG_AND_SW_HARDENING_NEEDED.
    /// TCB is up to date, but both software hardening and platform configuration are required.
    ConfigAndSwHardeningNeeded = 0x0008,

    /// Corresponds to SGX_QL_QV_RESULT_TD_RELAUNCH_ADVISED.
    /// For TDX: TD ran with an outdated TDX module; relaunch or re-provision is advised.
    TDRelaunchAdvised = 0x0009,

    /// Corresponds to SGX_QL_QV_RESULT_TD_RELAUNCH_ADVISED_CONFIG_NEEDED.
    /// For TDX: Same as above, and additional platform configuration is also required.
    TDRelaunchAdvisedConfigNeeded = 0x000A,

    /// Corresponds to SGX_QL_QV_RESULT_MAX.
    /// Maximum defined SGX QVL result value.
    // QvResultMax = 0x00FF,
};

/**
 * @brief The class for Intel DCAP remote attestation.
 *
 * This class provides APIs to create and verify remote attestation reports for Intel SGX Enclave applications.
 *
 * Example usage:
 * @code
 *  ssgx::attestation_t::RemoteAttestor attestor;
 *
 *  // Step 1: (Prover side) Create a remote attestation report — usually done inside the enclave.
 *  std::string report;
 *  uint8_t user_data[64] = {0};  // Application-defined data
 *
 *  if (attestor.CreateReport(user_data, report)) {
 *      // Step 2: (Verifier side) Configure acceptable SGX Quote Verification results
 *      attestor.SetAcceptableResults({
 *          ssgx::attestation_t::QvResult::Ok,
 *          ssgx::attestation_t::QvResult::ConfigNeeded
 *      });
 *
 *      // Step 3: Verify the attestation report and extract MRENCLAVE
 *      std::string mrenclave_hex;
 *      if (attestor.VerifyReport(user_data, report, mrenclave_hex)) {
 *          // Verification succeeded under the accepted QvResult policy
 *
 *          // WARNING: mrenclave_hex (MRENCLAVE) is returned as-is from the report.
 *          // You MUST verify mrenclave_hex against a known trusted value (e.g., allowlist or policy)
 *          // before trusting the enclave's identity.
 *
 *          static const std::string kTrustedEnclaveId = "abcdef1234567890...";  // Expected MRENCLAVE
 *          if (mrenclave_hex == kTrustedEnclaveId) {
 *              // The enclave identity is trusted
 *          } else {
 *              // The enclave identity does not match — reject or raise an alert
 *          }
 *      } else {
 *          // Verification failed — inspect error details
 *          auto err = attestor.GetLastErrorCode();
 *          auto msg = attestor.GetLastErrorMsg();
 *      }
 *  } else {
 *      // Report creation failed — inspect error details
 *      auto err = attestor.GetLastErrorCode();
 *      auto msg = attestor.GetLastErrorMsg();
 *  }
 *
 *  // Note: If `SetAcceptableResults` is not called, only QvResult::Ok is accepted by default.
 *  // This may cause verification to fail on otherwise trustworthy platforms (e.g., when QvResult is ConfigNeeded).
 * @endcode
 */
class RemoteAttestor {
  public:
    RemoteAttestor();

  public:
    /**
     * @brief Generate a remote attestation report.
     * @param[in] user_data User-defined data, totaling 64 bytes, will be included in the remote attestation report.
     * @param[out] report remote attestation report
     * @return Return true if successful; otherwise, return false
     */
    bool CreateReport(const uint8_t user_data[64], std::string& report);

    /**
     * @brief Set the list of SGX Quote Verification results that are considered acceptable.
     *
     * Intel SGX Quote Verification Library (QVL) produces result codes such as `QvResult::Ok`,
     * `QvResult::ConfigNeeded`, or `QvResult::OutOfDate` after quote verification. With the upgrade
     * to PCS v4 and QVL v4, it has become increasingly rare to receive the ideal result `QvResult::Ok`,
     * even on properly configured platforms.
     *
     * This function allows the application to explicitly define which of those QvResult codes are acceptable
     * within its own trust model.
     *
     * If the quote result is included in this list, and all other validation steps pass (e.g. user data,
     * timestamp), the verification will be considered successful (i.e. `ErrorCode::Success`).
     *
     * If the quote result is not included, the verification will be rejected regardless of other conditions.
     *
     *
     * @param accepted_results A list of SGX QVL quote results (`QvResult` enum) that the application deems acceptable.
     *                         Internal error codes (e.g., invalid parameters) must not be passed here.
     *
     * @note If this function is not called, the default behavior is to only accept `QvResult::Ok`
     *       (i.e., SGX_QL_QV_RESULT_OK). This may result in failed verification on otherwise secure platforms,
     *       especially when using Intel PCS v4.
     *
     * @note According to Intel SGX QVL specification, only the following `SGX_QL_QV_RESULT_*` values
     *       may be considered *acceptable* under a user-defined trust policy:
     *
     *       -  QvResult::Ok                          ------  SGX_QL_QV_RESULT_OK
     *       -  QvResult::ConfigNeeded                ------  SGX_QL_QV_RESULT_CONFIG_NEEDED
     *       -  QvResult::OutOfDate                   ------  SGX_QL_QV_RESULT_OUT_OF_DATE
     *       -  QvResult::OutOfDateConfigNeeded       ------  SGX_QL_QV_RESULT_OUT_OF_DATE_CONFIG_NEEDED
     *       -  QvResult::SwHardeningNeeded           ------  SGX_QL_QV_RESULT_SW_HARDENING_NEEDED
     *       -  QvResult::ConfigAndSwHardeningNeeded  ------  SGX_QL_QV_RESULT_CONFIG_AND_SW_HARDENING_NEEDED
     *
     *       These results are classified by Intel's Quote Verification Library (QVL) as "non-terminal",
     *       meaning the quote is structurally valid and does not represent a definitive security failure.
     *       However, they may indicate that the platform requires additional configuration,
     *       software hardening, or security updates.
     *
     *       Whether to treat any of these results as acceptable depends entirely on the application’s
     *       trust model and security policy. This function allows the caller to explicitly define
     *       which of them should be accepted.
     *
     *       All other SGX_QL_QV_RESULT_* values—such as `INVALID_SIGNATURE`, `REVOKED`, or `UNSPECIFIED`—
     *       indicate unrecoverable errors and must not be accepted under any trust model.
     *
     * @warning It is strongly recommended to call this function to explicitly accept "non-ideal but trusted" states
     *          such as `ConfigNeeded` or `OutOfDate`, depending on the deployment scenario.
     */
    void SetAcceptableResults(std::initializer_list<QvResult> accepted_results);

    /**
     * @brief Verify a remote attestation report within a Trusted Execution Environment.
     *
     * This function verifies a remote attestation report generated with fixed-size user data.
     * If quote verification passes according to the accepted QvResult policy (see SetAcceptableResults),
     * and internal validations (user data, timestamp) succeed, the verification is considered successful.
     *
     * The verified MRENCLAVE value will be returned via `mrenclave_hex`.
     *
     * @warning The `mrenclave_hex` is extracted as-is from the quote. This function DOES NOT validate whether
     *          the mrenclave_hex matches any known or trusted value. You MUST validate it against an expected
     *          allowlist or policy externally to ensure the remote enclave is trusted.
     *
     * @param[in] user_data Fixed 64-byte user data used during report creation.
     * @param[in] report Remote attestation report generated by a trusted enclave.
     * @param[out] mrenclave_hex  Hex-encoded MRENCLAVE extracted from the report.
     * @return Return true if the report is valid and trusted under the defined policy.
     *
     * Example:
     * @code
     * std::string mrenclave_hex;
     * if (attestor.VerifyReport(user_data, report, mrenclave_hex)) {
     *     static const std::string expected_mrenclave_hex = "<trusted_mrenclave>";
     *     if (mrenclave_hex == expected_mrenclave_hex) {
     *         // Enclave is trusted
     *     } else {
     *         // Identity mismatch — do not trust
     *     }
     * }
     * @endcode
     */
    bool VerifyReport(const uint8_t user_data[64], const std::string& report, std::string& mrenclave_hex);

    /**
     * @brief Generate a remote attestation report.
     * @param[in] user_info User-defined data of unrestricted length, whose SHA-256 hash will be included in the remote
     * attestation report.
     * @param[out] report remote attestation report
     * @return Return true if successful; otherwise, return false
     */
    bool CreateReport(const std::string& user_info, std::string& report);

    /**
     * @brief Verify a remote attestation report within a Trusted Execution Environment.
     *
     * The function verifies a report generated with arbitrary-length user data.
     * It checks quote validity and user data consistency.
     *
     * @warning The `mrenclave_hex` is returned directly from the quote.
     *          This function does NOT validate that the enclave matches any known trusted identity.
     *          You MUST compare `mrenclave_hex` manually against your trusted policy.
     *
     * @param[in] user_info Arbitrary user data provided during report creation.
     * @param[in] report Remote attestation report.
     * @param[out] mrenclave_hex  Hex-encoded MRENCLAVE extracted from the report.
     * @return Return true if the report passes all validations and quote result is accepted.
     *
     * Example:
     * @code
     * std::string mrenclave_hex;
     * if (attestor.VerifyReport(user_info, report, mrenclave_hex)) {
     *     static const std::string expected_mrenclave_hex = "<trusted_mrenclave>";
     *     if (mrenclave_hex == "<expected_mrenclave_hex>") {
     *         // Trusted enclave
     *     } else {
     *         // Untrusted identity
     *     }
     * }
     * @endcode
     */
    bool VerifyReport(const std::string& user_info, const std::string& report, std::string& mrenclave_hex);

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
     * @brief Verify a remote attestation report that includes timestamp-based freshness validation.
     *
     * This version of VerifyReport includes temporal checks: if the report's timestamp is older
     * than the allowed validity window, the verification fails.
     *
     * @warning This function returns `mrenclave_hex` (MRENCLAVE) from the report as-is.
     *          It does NOT check that the enclave matches any trusted value. You must perform this
     *          verification manually (e.g., against an allowlist or expected hash).
     *
     * @param[in] user_info User-defined data used during report generation.
     * @param[in] timestamp Timestamp used to bind the report to a specific moment in time.
     * @param[in] validity_seconds Allowed validity window in seconds.
     * @param[in] report Remote attestation report.
     * @param[out] mrenclave_hex  Hex-encoded MRENCLAVE extracted from the report.
     * @return Return true if all checks succeed and quote result is within accepted policy.
     *
     * Example:
     * @code
     * std::string mrenclave_hex;
     * if (attestor.VerifyReport(user_info, timestamp, validity_seconds, report, mrenclave_hex)) {
     *     static const std::string expected_mrenclave_hex = "<trusted_mrenclave>";
     *     if (mrenclave_hex == "<expected_mrenclave_hex>") {
     *         // Valid and trusted
     *     } else {
     *         // MRENCLAVE mismatch
     *     }
     * }
     * @endcode
     */
    bool VerifyReport(const std::string& user_info, uint64_t timestamp, uint64_t validity_seconds,
                      const std::string& report, std::string& mrenclave_hex);

    /**
     * @brief Get the last error code
     * @return an error code
     */
    [[nodiscard]] ErrorCode GetLastErrorCode() const {
        return error_code_;
    };

    /**
     * @brief Get the last error message
     * @return an error message
     */
    std::string GetLastErrorMsg() const {
        return error_msg_;
    }

    /**
     * @brief Get the raw SGX Quote Verification result from the last verification.
     *
     * This returns the numeric result code produced by the Intel SGX Quote Verification Library (QVL),
     * such as `SGX_QL_QV_RESULT_OK`, `SGX_QL_QV_RESULT_OUT_OF_DATE`, or `SGX_QL_QV_RESULT_CONFIG_NEEDED`.
     *
     * The value corresponds directly to the `sgx_ql_qv_result_t` enum defined in the Intel SGX DCAP library.
     * These result codes are declared in:
     * https://github.com/intel/SGXDataCenterAttestationPrimitives/blob/main/QuoteVerification/QvE/Include/sgx_qve_header.h
     *
     * @return An SGX QVL result code (`sgx_ql_qv_result_t`) as a `uint32_t`, or `std::nullopt` if not available.
     *
     * @note This is a low-level result intended for diagnostics or logging. For policy-level decisions,
     *       use `VerifyReport()` in conjunction with `SetAcceptableResults()` to determine trust.
     */
    [[nodiscard]] std::optional<uint32_t> GetRawQvResult() const {
        return qv_result_;
    }

  private:
    ErrorCode error_code_;
    std::string error_msg_;
    std::optional<uint32_t> qv_result_;
    std::unordered_set<uint32_t> accepted_qv_results_; // Accept only SGX_QL_QV_RESULT_OK by default
};

} // namespace attestation_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_ATTESTATION_T_H_