#include "ssgx_utils_t.h"

#include "Enclave_t.h"
#include "TrustedArbiter.h"

int ecall_create_tee_pubkey(const char* alice_pubkey_hex, const char* bob_pubkey_hex,
                            char** out_arbiter_public_key_hex_ptr, char** out_quote_report_ptr,
                            uint64_t* out_prep_timestamp, char** out_one_time_access_token_ptr) {
    std::string arbiter_pubkey_hex;
    std::string quote_report;
    int64_t prep_timestamp;
    std::string one_time_access_token;
    bool ok = TrustedArbiter::GetSingleInstance().Prepare(alice_pubkey_hex, bob_pubkey_hex, arbiter_pubkey_hex,
                                                          quote_report, prep_timestamp, one_time_access_token);
    if (!ok) {
        return -1;
    }
    *out_arbiter_public_key_hex_ptr =
        ssgx::utils_t::StrndupOutside(arbiter_pubkey_hex.c_str(), arbiter_pubkey_hex.length());
    if (*out_arbiter_public_key_hex_ptr == nullptr) {
        return -2;
    }

    *out_quote_report_ptr = ssgx::utils_t::StrndupOutside(quote_report.c_str(), quote_report.length());
    if (*out_quote_report_ptr == nullptr) {
        return -3;
    }

    *out_prep_timestamp = prep_timestamp;

    *out_one_time_access_token_ptr =
        ssgx::utils_t::StrndupOutside(one_time_access_token.c_str(), one_time_access_token.length());
    if (*out_one_time_access_token_ptr == nullptr) {
        return -4;
    }
    return 0;
}

int ecall_who_is_richer(const char* one_time_access_token, const char* alice_ciphertext, const char* alice_signature,
                        const char* bob_ciphertext, const char* bob_signature, char** out_alice_vs_bob_result_ptr,
                        char** out_quote_report_ptr, uint64_t* out_compare_timestamp) {

    bool ok = TrustedArbiter::GetSingleInstance().IsReady();
    if (!ok) {
        return -5;
    }

    std::string comparison_result;
    std::string quote_report;
    uint64_t compare_timestamp;
    ok = TrustedArbiter::GetSingleInstance().Compare(one_time_access_token, alice_ciphertext, alice_signature,
                                                     bob_ciphertext, bob_signature, comparison_result, quote_report,
                                                     compare_timestamp);

    if (!ok) {
        return -6;
    }

    *out_alice_vs_bob_result_ptr = ssgx::utils_t::StrndupOutside(comparison_result.c_str(), comparison_result.length());
    if (*out_alice_vs_bob_result_ptr == nullptr) {
        return -7;
    }

    *out_quote_report_ptr = ssgx::utils_t::StrndupOutside(quote_report.c_str(), quote_report.length());
    if (*out_quote_report_ptr == nullptr) {
        return -8;
    }

    *out_compare_timestamp = compare_timestamp;

    return 0;
}