/**
 *  @brief Error codes definition
 *
 *  When an operation is failed, call GetLastErrorCode() to receive
 *  one of bellow error codes.
 *
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

    /// Failed to call the interface of retrieve the supplemental data size.
    GetSupplementSizeCallException = 0x0012,

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
    /// QvResultMax = 0x00FF,
};