#ifndef SSGXLIB_SAMPLE_MILLIONAIRE_PROBLEM_TRUSTEDARBITER_H_
#define SSGXLIB_SAMPLE_MILLIONAIRE_PROBLEM_TRUSTEDARBITER_H_

#include <cstring>

#include "crypto-suites/crypto-bn/bn.h"
#include "crypto-suites/crypto-ecies/ecies.h"

class TrustedArbiter {
  public:
    TrustedArbiter() : is_ready_(false) {};
    static TrustedArbiter& GetSingleInstance();
    bool Prepare(const std::string& alice_pubkey_hex, const std::string& bob_pubkey_hex,
                 std::string& out_arbiter_pubkey_hex, std::string& out_quote_report, int64_t& out_prepare_timestamp,
                 std::string& one_time_access_token);
    bool IsReady();
    bool Compare(const std::string& one_time_access_token, const std::string& alice_ciphertext,
                 const std::string& alice_signature, const std::string& bob_ciphertext,
                 const std::string& bob_signature, std::string& out_comparison_result, std::string& out_quote_report,
                 uint64_t& out_compare_timestamp);

  private:
    bool VerifySignatureAndDecrypt(const safeheron::curve::CurvePoint& pubkey, const std::string& ciphertext,
                                   const std::string& signature, std::string& plaintext);

  private:
    std::string one_time_access_token_;
    safeheron::bignum::BN arbiter_privkey_;
    safeheron::curve::CurvePoint arbiter_pubkey_;
    safeheron::curve::CurvePoint alice_pubkey_;
    safeheron::curve::CurvePoint bob_pubkey_;
    int64_t prepare_timestamp_;
    int64_t compare_timestamp_;
    bool is_ready_;
    std::mutex mutex_;
};
#endif // SSGXLIB_SAMPLE_MILLIONAIRE_PROBLEM_TRUSTEDARBITER_H_
