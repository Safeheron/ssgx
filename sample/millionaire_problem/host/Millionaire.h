#ifndef SSGX_SAMPLE_MILLIONAIRE_H
#define SSGX_SAMPLE_MILLIONAIRE_H

#include "crypto-suites/crypto-bn/bn.h"
#include "crypto-suites/crypto-ecies/ecies.h"
#include "crypto-suites/crypto-hash/sha256.h"

class Millionaire {
  public:
    explicit Millionaire(std::string name, std::string wealth);

    int VerifyQuoteAndSaveArbiterPublicKey(const std::string& one_time_access_token, const std::string& quote,
                                       uint64_t timestamp, const std::string& arbiter_public_key,
                                       const std::string& peer_millionaire_public_key, std::string &mr_enclave_in_report);

    int VerifyQuoteAndShowResult(const std::string& one_time_access_token, const std::string& quote, uint64_t timestamp,
                                 const std::string& alice_vs_bob_result, std::string &mr_enclave_in_report);

    int EncryptAndSignWealth(std::string& ciphertext, std::string& signature);
    std::string& GetPublicKey();

  private:
    safeheron::bignum::BN private_key_;
    std::string public_key_hex_;
    std::string wealth_;
    std::string arbiter_public_key_hex_;
    std::string name_;
    int GenerateKeyPair();
    int VerifyQuoteUntrusted(const std::string& quote, uint64_t timestamp, const std::string& user_info,
                             std::string &mr_enclave_in_report);
};

#endif // SSGX_SAMPLE_MILLIONAIRE_H
