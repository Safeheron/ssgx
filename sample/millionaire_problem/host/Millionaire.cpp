#include <cstring>
#include <stdio.h>
#include <unistd.h>

#include "ssgx_attestation_u.h"

#include "crypto-suites/crypto-bn/bn.h"
#include "crypto-suites/crypto-bn/rand.h"
#include "crypto-suites/crypto-curve/curve.h"
#include "crypto-suites/crypto-ecies/ecies.h"
#include "crypto-suites/crypto-encode/hex.h"
#include "crypto-suites/crypto-hash/sha256.h"

#include "Millionaire.h"

using safeheron::bignum::BN;
using safeheron::curve::Curve;
using safeheron::curve::CurvePoint;
using safeheron::curve::CurveType;
using safeheron::ecies::ECIES;

Millionaire::Millionaire(std::string name, std::string wealth)
    : name_(std::move(name)), wealth_(std::move(wealth)) {
    GenerateKeyPair();
}

int Millionaire::VerifyQuoteAndSaveArbiterPublicKey(const std::string& one_time_access_token, const std::string& quote,
                                                    uint64_t timestamp, const std::string& arbiter_public_key,
                                                    const std::string& peer_millionaire_public_key,
                                                    std::string &mr_enclave_in_report) {
    std::string user_info;
    /// user_info = min_pubkey + max_pubkey + arbiter_public_key_hex + one_time_access_token
    if (public_key_hex_ < peer_millionaire_public_key) {
        user_info.append(public_key_hex_);
        user_info.append(peer_millionaire_public_key);
    } else {
        user_info.append(peer_millionaire_public_key);
        user_info.append(public_key_hex_);
    }
    user_info.append(arbiter_public_key);
    user_info.append(one_time_access_token);
    if (0 != VerifyQuoteUntrusted(quote, timestamp, user_info, mr_enclave_in_report)) {
        return -1;
    }
    arbiter_public_key_hex_ = arbiter_public_key;
    printf("\n%s Save Arbiter public key succeeded\n", name_.c_str());
    return 0;
}

int Millionaire::VerifyQuoteAndShowResult(const std::string& one_time_access_token, const std::string& quote,
                                          uint64_t timestamp, const std::string& alice_vs_bob_result,
                                          std::string &mr_enclave_in_report) {
    std::string user_info = alice_vs_bob_result + one_time_access_token;
    if (0 != VerifyQuoteUntrusted(quote, timestamp, user_info, mr_enclave_in_report)) {
        return -1;
    }
    printf("\n%s presentation results:%s\n", name_.c_str(), alice_vs_bob_result.c_str());
    return 0;
}

int Millionaire::EncryptAndSignWealth(std::string& ciphertext, std::string& signature) {
    bool pass = false;

    // Parse the Arbiter public key
    std::string arbiter_pubkey_full = safeheron::encode::hex::DecodeFromHex(arbiter_public_key_hex_);
    CurvePoint arbiter_pubkey;
    pass = arbiter_pubkey.DecodeFull(arbiter_pubkey_full, CurveType::P256);
    if (!pass)
        return -1;

    // Encrypt the wealth value
    ECIES ecies;
    ecies.set_curve_type(CurveType::P256);
    std::string ciphertext_bin;
    pass = ecies.EncryptPack(arbiter_pubkey, wealth_, ciphertext_bin);
    if (!pass)
        return -2;
    ciphertext = safeheron::encode::hex::EncodeToHex(ciphertext_bin);

    // Compute hash of ciphertext
    uint8_t md[safeheron::hash::CSHA256::OUTPUT_SIZE] = {0};
    safeheron::hash::CSHA256 sha256;
    sha256.Write((unsigned char*)ciphertext.c_str(), ciphertext.length());
    sha256.Finalize(md);

    // Generate signature
    uint8_t signature_bin[64];
    safeheron::curve::ecdsa::Sign(CurveType::P256, private_key_, md, signature_bin);
    signature = safeheron::encode::hex::EncodeToHex(std::string(reinterpret_cast<const char*>(signature_bin), 64));

    printf("\n%s signing and encryption succeeded\n", name_.c_str());
    return 0;
}

std::string& Millionaire::GetPublicKey() {
    return public_key_hex_;
}

int Millionaire::GenerateKeyPair() {
    const safeheron::curve::Curve* curve = GetCurveParam(safeheron::curve::CurveType::P256);
    private_key_ = safeheron::rand::RandomBNLt(curve->n);

    uint8_t public_key_full[65] = {0};
    CurvePoint pub_key_point = curve->g * private_key_;
    pub_key_point.EncodeFull(public_key_full);
    public_key_hex_ = safeheron::encode::hex::EncodeToHex(public_key_full, 65);
    printf("\n%s created a key.public_key=%s\n", name_.c_str(), public_key_hex_.c_str());
    return 0;
}

int Millionaire::VerifyQuoteUntrusted(const std::string& quote, uint64_t timestamp, const std::string& user_info,
                                      std::string &mr_enclave_in_report) {
    ssgx::attestation_u::RemoteAttestor attestor;
    std::string quote_report(quote.c_str(), quote.size());

    if (!attestor.VerifyReport(user_info, timestamp, 100, quote_report, mr_enclave_in_report)) {
        printf("Verify report failed GetLastErrorMsg=%s\n", attestor.GetLastErrorMsg().c_str());
        return -1;
    }

    printf("\n%s Verify quote succeeded\n", name_.c_str());
    return 0;
}