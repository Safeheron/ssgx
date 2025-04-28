#include "TrustedArbiter.h"

#include <mutex>
#include <random>

#include "ssgx_attestation_t.h"
#include "ssgx_decimal_t.h"
#include "ssgx_utils_t.h"

#include "crypto-suites/crypto-bn/rand.h"
#include "crypto-suites/crypto-curve/curve.h"
#include "crypto-suites/crypto-encode/hex.h"
#include "crypto-suites/crypto-hash/sha256.h"

using safeheron::curve::Curve;
using safeheron::curve::CurvePoint;
using safeheron::curve::CurveType;
using safeheron::curve::GetCurveParam;
using safeheron::ecies::ECIES;

using ssgx::attestation_t::RemoteAttestor;
using ssgx::decimal_t::BigDecimal;

// Singleton implementation
TrustedArbiter& TrustedArbiter::GetSingleInstance() {
    static TrustedArbiter instance;
    return instance;
}

bool TrustedArbiter::VerifySignatureAndDecrypt(const safeheron::curve::CurvePoint& pubkey,
                                               const std::string& ciphertext, const std::string& signature,
                                               std::string& plaintext) {
    bool pass = false;

    // Get hash of ciphertext
    uint8_t md[safeheron::hash::CSHA256::OUTPUT_SIZE] = {0};
    // sha256
    safeheron::hash::CSHA256 sha256;
    sha256.Write((unsigned char*)ciphertext.c_str(), ciphertext.length());
    sha256.Finalize(md);

    // Verify signature
    pass = safeheron::curve::ecdsa::Verify(CurveType::P256, pubkey, md,
                                           (const uint8_t*)safeheron::encode::hex::DecodeFromHex(signature).c_str());
    if (!pass) {
        return false;
    }

    std::string ciphertext_bin = safeheron::encode::hex::DecodeFromHex(ciphertext);
    // Decrypt ciphertext using TEE private key
    ECIES enc;
    enc.set_curve_type(CurveType::P256);
    pass = enc.DecryptPack(arbiter_privkey_, ciphertext_bin, plaintext);
    if (!pass) {
        return false;
    }
    return true;
}

// Prepare phase: store both public keys and generate one-time access token
bool TrustedArbiter::Prepare(const std::string& alice_pubkey_hex, const std::string& bob_pubkey_hex,
                             std::string& out_arbiter_pubkey_hex, std::string& out_quote_report,
                             int64_t& out_prepare_timestamp, std::string& out_one_time_access_token) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Parse alice's public key
    std::string alice_pubkey_full_bytes = safeheron::encode::hex::DecodeFromHex(alice_pubkey_hex);
    bool ok = alice_pubkey_.DecodeFull(alice_pubkey_full_bytes, CurveType::P256);
    if (!ok) {
        return false;
    }

    // Parse bob's public key
    std::string bob_pubkey_full_bytes = safeheron::encode::hex::DecodeFromHex(bob_pubkey_hex);
    ok = bob_pubkey_.DecodeFull(bob_pubkey_full_bytes, CurveType::P256);
    if (!ok) {
        return false;
    }

    // Generate arbiter private key randomly
    const Curve* curv = GetCurveParam(CurveType::P256);
    arbiter_privkey_ = safeheron::rand::RandomBNLt(curv->n);
    // Generate TEE public key
    arbiter_pubkey_ = curv->g * arbiter_privkey_;

    // Output arbiter_pubkey_hex
    std::string arbiter_pubkey_full_bytes;
    arbiter_pubkey_.EncodeFull(arbiter_pubkey_full_bytes);
    out_arbiter_pubkey_hex = safeheron::encode::hex::EncodeToHex(arbiter_pubkey_full_bytes);

    prepare_timestamp_ = ssgx::utils_t::DateTime::Now().GetTimestamp();
    out_prepare_timestamp = prepare_timestamp_;

    uint8_t token[16];
    safeheron::rand::RandomBytes(token, sizeof token);
    one_time_access_token_ = safeheron::encode::hex::EncodeToHex(token, sizeof token);
    out_one_time_access_token = one_time_access_token_;

    /// user_info = min_pubkey + max_pubkey + arbiter_public_key_hex + one_time_access_token
    std::string user_info;
    if (alice_pubkey_hex < bob_pubkey_hex) {
        user_info.append(alice_pubkey_hex);
        user_info.append(bob_pubkey_hex);
    } else {
        user_info.append(bob_pubkey_hex);
        user_info.append(alice_pubkey_hex);
    }
    user_info.append(out_arbiter_pubkey_hex);
    user_info.append(one_time_access_token_);

    // Create remote attestation report
    RemoteAttestor attestor;
    if (!attestor.CreateReport(user_info, prepare_timestamp_, out_quote_report)) {
        return false;
    }

    is_ready_ = true;

    ssgx::utils_t::Printf("\nArbiter prepares:\n");
    ssgx::utils_t::Printf("one time access token: %s\n", one_time_access_token_.c_str());
    ssgx::utils_t::Printf("remote attestation quote: %s\n\n", out_quote_report.c_str());

    return true;
}

// Check whether the arbiter is ready (i.e., both public keys are set)
bool TrustedArbiter::IsReady() {
    std::lock_guard<std::mutex> lock(mutex_);
    return is_ready_;
}

// Simulate comparison function (real version should decrypt and verify signatures)
bool TrustedArbiter::Compare(const std::string& one_time_access_token, const std::string& alice_ciphertext,
                             const std::string& alice_signature, const std::string& bob_ciphertext,
                             const std::string& bob_signature, std::string& out_comparison_result,
                             std::string& out_quote_report, uint64_t& out_compare_timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    is_ready_ = false;

    // one_time_access_token can be used once.
    if (one_time_access_token != one_time_access_token_) {
        return false;
    }

    // Get Alice's wealth
    std::string alice_plaintext;
    bool ok = VerifySignatureAndDecrypt(alice_pubkey_, alice_ciphertext, alice_signature, alice_plaintext);
    if (!ok) {
        return false;
    }
    const BigDecimal alice_wealth(alice_plaintext.c_str());

    // Get Bob's wealth
    std::string bob_plaintext;
    ok = VerifySignatureAndDecrypt(bob_pubkey_, bob_ciphertext, bob_signature, bob_plaintext);
    if (!ok) {
        return false;
    }
    const BigDecimal bob_wealth(bob_plaintext.c_str());

    // Compare and generate result
    if (alice_wealth > bob_wealth) {
        out_comparison_result = "Alice is richer";
    } else {
        out_comparison_result = "Alice is not richer than Bob";
    }

    compare_timestamp_ = ssgx::utils_t::DateTime::Now().GetTimestamp();
    out_compare_timestamp = compare_timestamp_;

    // Create remote attestation report
    // user_info = out_comparison_result + one_time_access_token
    RemoteAttestor attestor;
    std::string user_info = out_comparison_result + one_time_access_token_;

    ok = attestor.CreateReport(user_info, compare_timestamp_, out_quote_report);
    if (!ok) {
        return false;
    }

    ssgx::utils_t::Printf("\nArbiter performs a comparison:\n");
    ssgx::utils_t::Printf("one time access token: %s\n", one_time_access_token_.c_str());
    ssgx::utils_t::Printf("remote attestation quote: %s\n\n", out_quote_report.c_str());
    ssgx::utils_t::Printf("\n\n");

    return true;
}
