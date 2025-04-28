#include <cstring>
#include <iostream>
#include <cstdio>

#include "sgx_urts.h"

#include "Enclave_u.h"
#include "Millionaire.h"

int SGX_CDECL main(int argc, char* argv[]) {
    int ret = 0;
    sgx_status_t sgx_status = SGX_SUCCESS;
    sgx_enclave_id_t enclave_id;

    printf("Try to create testing enclave ...\n");
    sgx_status = sgx_create_enclave((const char*)argv[1], 0, nullptr, nullptr, &enclave_id, nullptr);
    if (sgx_status != SGX_SUCCESS) {
        printf("--->Initialize enclave failed! enclave file: %s, sgx message: %s\n", argv[1],
               strerror((int)sgx_status));
        return -1;
    }
    printf("Enclave is created!\n\n");

    /// alice & bob both know the code content and the corresponding mr_enclave value
	/// Note: Remote attestation in Intel SGX relies on the MRENCLAVE measurement to verify the identity of the enclave.
	/// Therefore, it is essential to use the enclave built in release mode.
	/// Using a debug build or any non-identical environment will result in a different MRENCLAVE, causing the attestation to fail.
    const std::string Expected_MR_ENCLAVE_HEX = "381ad7d4591bcb47edfc666904fc6140b0f61b5cd0d466ed0344024828383087";

    /// alice generates a key pair
    Millionaire alice{"alice", "111.23"};

    /// alice generates a key pair
    Millionaire bob{"bob", "112.23"};

    /// get TEE public key and remote attestation report
    char* quote_report_ptr = nullptr;
    char* arbiter_pubkey_hex_ptr = nullptr;
    char* one_time_access_token = nullptr;
    uint64_t timestamp;

    sgx_status =
        ecall_create_tee_pubkey(enclave_id, &ret, alice.GetPublicKey().c_str(), bob.GetPublicKey().c_str(),
                                &arbiter_pubkey_hex_ptr, &quote_report_ptr, &timestamp, &one_time_access_token);
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        printf("Failed to obtain Arbiter public key. ret: %d, sgx message: %s\n", ret, strerror((int)sgx_status));
        return -2;
    }
    std::string quote_report{quote_report_ptr};
    std::string arbiter_pubkey_hex{arbiter_pubkey_hex_ptr};

    /// alice and bob exchange their public keys and assemble the same Userinfo.
    /// They both verify the quote and the report with the same data.

    std::string mr_enclave_in_report;

    if (0 != alice.VerifyQuoteAndSaveArbiterPublicKey(one_time_access_token, quote_report, timestamp, arbiter_pubkey_hex,
                                                  bob.GetPublicKey(), mr_enclave_in_report)) {
        printf("Failed ocall_verify_quote_untrusted.\n\n");
        return -3;
    }
    printf("\nMRENCLAVE in report: %s\n", mr_enclave_in_report.c_str());
    // alice must perform an additional check to ensure that the MRENCLAVE value in the report matches the expected MRENCLAVE of the enclave.
    // If they differ, the verification is considered failed and the enclave should not be trusted.
    // if ( mr_enclave_in_report !== Expected_MR_ENCLAVE_HEX ) {
    //    printf("Failed to verify the remote attestation quote\n\n");
    //    return -4;
    // }

    if (0 != bob.VerifyQuoteAndSaveArbiterPublicKey(one_time_access_token, quote_report, timestamp, arbiter_pubkey_hex,
                                                alice.GetPublicKey(), mr_enclave_in_report)) {
        printf("Failed ocall_verify_quote_untrusted.\n\n");
        return -3;
    }
    printf("\nMRENCLAVE in report: %s\n", mr_enclave_in_report.c_str());
    // bob must perform an additional check to ensure that the MRENCLAVE value in the report matches the expected MRENCLAVE of the enclave.
    // If they differ, the verification is considered failed and the enclave should not be trusted.
    // if ( mr_enclave_in_report !== Expected_MR_ENCLAVE_HEX ) {
    //    printf("Failed to verify the remote attestation quote\n\n");
    //    return -4;
    // }

    /// alice encrypts her wealth and signs it
    std::string alice_ciphertext;
    std::string alice_signature;
    if (0 != alice.EncryptAndSignWealth(alice_ciphertext, alice_signature)) {
        printf("Failed in alice.EncryptAndSignWealth(alice_ciphertext, alice_signature).\n\n");
        return -3;
    }

    /// bob encrypts his wealth and signs it
    std::string bob_ciphertext;
    std::string bob_signature;
    if (0 != bob.EncryptAndSignWealth(bob_ciphertext, bob_signature)) {
        printf("Failed in bob.EncryptAndSignWealth(bob_ciphertext, bob_signature).\n\n");
        return -3;
    }

    /// ask Arbiter to compare the two values and return the result along with a remote attestation report.
    /// (Alternatively, the Arbiter can sign the result with its key to prove authenticity)
    char* alice_vs_bob_result_ptr = nullptr;
    char* result_quote_report_ptr = nullptr;
    sgx_status = ecall_who_is_richer(enclave_id, &ret, one_time_access_token, alice_ciphertext.c_str(),
                                     alice_signature.c_str(), bob_ciphertext.c_str(), bob_signature.c_str(),
                                     &alice_vs_bob_result_ptr, &result_quote_report_ptr, &timestamp);
    if (sgx_status != SGX_SUCCESS || ret != 0) {
        printf("Failed to obtain alice vs bob result. ret=%d,sgx_status=%s\n\n", ret, strerror((int)sgx_status));
        return -3;
    }

    std::string alice_vs_bob_result = std::string(alice_vs_bob_result_ptr);
    std::string result_quote_report = std::string(result_quote_report_ptr);
    /// alice verifies the result to prove its authenticity
    if ( 0 != alice.VerifyQuoteAndShowResult(one_time_access_token, result_quote_report, timestamp, alice_vs_bob_result, mr_enclave_in_report)) {
        printf("Failed in alice.VerifyQuoteAndShowResult(one_time_access_token, result_quote_report, timestamp, alice_vs_bob_result).\n\n");
        return -3;
    }
    printf("\nMRENCLAVE in report: %s\n", mr_enclave_in_report.c_str());
    // alice must perform an additional check to ensure that the MRENCLAVE value in the report matches the expected MRENCLAVE of the enclave.
    // If they differ, the verification is considered failed and the enclave should not be trusted.
    // if ( mr_enclave_in_report !== Expected_MR_ENCLAVE_HEX ) {
    //    printf("Failed to verify the remote attestation quote\n\n");
    //    return -4;
    // }

    if (0 != bob.VerifyQuoteAndShowResult(one_time_access_token, result_quote_report, timestamp, alice_vs_bob_result, mr_enclave_in_report)) {
        printf("Failed in bob.VerifyQuoteAndShowResult(one_time_access_token, result_quote_report, timestamp, alice_vs_bob_result).\n\n");
        return -3;
    }
    printf("\nMRENCLAVE in report: %s\n", mr_enclave_in_report.c_str());
    // bob must perform an additional check to ensure that the MRENCLAVE value in the report matches the expected MRENCLAVE of the enclave.
    // If they differ, the verification is considered failed and the enclave should not be trusted.
    // if ( mr_enclave_in_report !== Expected_MR_ENCLAVE_HEX ) {
    //    printf("Failed to verify the remote attestation quote\n\n");
    //    return -4;
    // }

    printf("\nExit from function\n");

_exit:
    // Waiting for server exit
    sgx_destroy_enclave(enclave_id);
    printf("End!\n\n");

    return ret;
}