#include <cstring>
#include <stdexcept>
#include <sgx_utils.h>

#include "sgx_tseal.h"

#include "ssgx_attestation_t.h"
#include "ssgx_exception_t.h"
#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

#include "Enclave_t.h"

using ssgx::utils_t::SealHandler;

/**
 * @brief Tests sealing functionality with valid input.
 */
TEST(SealHandlerTestSuite, TestSealDataValid) {
    SealHandler sealer;

    uint8_t mac_text[] = {1, 2, 3, 4};
    sealer.SetAdditionalMacText(mac_text, sizeof(mac_text));

    const char* raw_data = "Hello SGX";
    size_t data_len = strlen(raw_data);

    auto sealed_data = sealer.SealData(reinterpret_cast<const uint8_t*>(raw_data), data_len);
    ASSERT_TRUE(sealed_data.has_value());
    ASSERT_FALSE(sealed_data->empty());
}

/**
 * @brief Tests unsealing valid sealed data.
 */
TEST(SealHandlerTestSuite, TestUnsealDataValid) {
    SealHandler sealer;

    uint8_t mac_text[] = {5, 6, 7, 8};
    sealer.SetAdditionalMacText(mac_text, sizeof(mac_text));

    const char* raw_data = "Secure Data";
    size_t data_len = strlen(raw_data);

    auto sealed_data = sealer.SealData(reinterpret_cast<const uint8_t*>(raw_data), data_len);
    ASSERT_TRUE(sealed_data.has_value());

    auto unsealed_data = sealer.UnsealData(sealed_data->data(), sealed_data->size());
    ASSERT_TRUE(unsealed_data.has_value());

    ASSERT_EQ(std::string(unsealed_data->decrypted_text.begin(), unsealed_data->decrypted_text.end()), "Secure Data");
}

/**
 * @brief Tests sealing with an empty input.
 */
TEST(SealHandlerTestSuite, TestSealEmptyData) {
    SealHandler sealer;

    auto sealed_data = sealer.SealData(nullptr, 0);
    ASSERT_FALSE(sealed_data.has_value());
    ASSERT_EQ(sealer.GetLastError(), "Invalid input data for sealing.");
}

/**
 * @brief Tests unsealing with corrupted data.
 */
TEST(SealHandlerTestSuite, TestUnsealCorruptData) {
    SealHandler sealer;

    uint8_t corrupt_data[] = {0x01, 0x02, 0x03, 0x04}; // Corrupt sealed data

    auto unsealed_data = sealer.UnsealData(corrupt_data, sizeof(corrupt_data));
    ASSERT_FALSE(unsealed_data.has_value());
    ASSERT_EQ(sealer.GetLastError(), "Invalid sealed data.");
}

/**
 * @brief Tests unsealing with insufficient data.
 */
TEST(SealHandlerTestSuite, TestUnsealInsufficientData) {
    SealHandler sealer;

    std::vector<uint8_t> small_data(sizeof(sgx_sealed_data_t) - 1, 0); // Too small to be valid sealed data

    auto unsealed_data = sealer.UnsealData(small_data.data(), small_data.size());
    ASSERT_FALSE(unsealed_data.has_value());
    ASSERT_EQ(sealer.GetLastError(), "Invalid sealed data.");
}

/**
 * @brief Tests sealing and unsealing using different key policies.
 */
TEST(SealHandlerTestSuite, TestDifferentKeyPolicies) {
    SealHandler sealer1(SGX_KEYPOLICY_MRENCLAVE);
    SealHandler sealer2(SGX_KEYPOLICY_MRSIGNER);

    const char* raw_data = "TestData";
    size_t data_len = strlen(raw_data);

    auto sealed_data1 = sealer1.SealData(reinterpret_cast<const uint8_t*>(raw_data), data_len);
    auto sealed_data2 = sealer2.SealData(reinterpret_cast<const uint8_t*>(raw_data), data_len);

    ASSERT_TRUE(sealed_data1.has_value());
    ASSERT_TRUE(sealed_data2.has_value());

    auto unsealed_data1 = sealer1.UnsealData(sealed_data1->data(), sealed_data1->size());
    auto unsealed_data2 = sealer2.UnsealData(sealed_data2->data(), sealed_data2->size());

    ASSERT_TRUE(unsealed_data1.has_value());
    ASSERT_TRUE(unsealed_data2.has_value());

    ASSERT_EQ(std::string(unsealed_data1->decrypted_text.begin(), unsealed_data1->decrypted_text.end()), "TestData");
    ASSERT_EQ(std::string(unsealed_data2->decrypted_text.begin(), unsealed_data2->decrypted_text.end()), "TestData");
}

/**
 * @brief Tests unsealing with altered sealed data.
 */
TEST(SealHandlerTestSuite, TestUnsealTamperedData) {
    SealHandler sealer;
    const char* raw_data = "SecretMessage";
    size_t data_len = strlen(raw_data);

    auto sealed_data = sealer.SealData(reinterpret_cast<const uint8_t*>(raw_data), data_len);
    ASSERT_TRUE(sealed_data.has_value());

    // Modify the sealed data (simulate tampering)
    (*sealed_data)[0] ^= 0xFF;

    auto unsealed_data = sealer.UnsealData(sealed_data->data(), sealed_data->size());
    ASSERT_FALSE(unsealed_data.has_value());
    ASSERT_EQ(sealer.GetLastError(),
              "Unsealing operation failed with error code: " + std::to_string(SGX_ERROR_MAC_MISMATCH));
}

/**
 * @brief Tests sealing large data.
 */
TEST(SealHandlerTestSuite, TestSealLargeData) {
    SealHandler sealer;
    std::vector<uint8_t> large_data(4096, 0xAB);

    auto sealed_data = sealer.SealData(large_data);
    ASSERT_TRUE(sealed_data.has_value());
}

/**
 * @brief Tests unsealing large data.
 */
TEST(SealHandlerTestSuite, TestUnsealLargeData) {
    SealHandler sealer;
    std::vector<uint8_t> large_data(8192, 0x42);

    auto sealed_data = sealer.SealData(large_data);
    ASSERT_TRUE(sealed_data.has_value());

    auto unsealed_data = sealer.UnsealData(sealed_data->data(), sealed_data->size());
    ASSERT_TRUE(unsealed_data.has_value());
    ASSERT_EQ(unsealed_data->decrypted_text, large_data);
}

/**
 * @brief Tests getting errors when no error has occurred.
 */
TEST(SealHandlerTestSuite, TestNoError) {
    SealHandler sealer;
    ASSERT_EQ(sealer.GetLastError(), "");
}

/**
 * @brief Tests getting error message after failure.
 */
TEST(SealHandlerTestSuite, TestGetErrorAfterFailure) {
    SealHandler sealer;

    auto sealed_data = sealer.SealData(nullptr, 0);
    ASSERT_FALSE(sealed_data.has_value());
    ASSERT_EQ(sealer.GetLastError(), "Invalid input data for sealing.");
}

/**
 * @brief Tests sealing and unsealing with additional MAC text.
 */
TEST(SealHandlerTestSuite, TestSealingWithMacText) {
    SealHandler sealer;
    uint8_t mac_text[] = {0xAA, 0xBB, 0xCC, 0xDD};
    sealer.SetAdditionalMacText(mac_text, sizeof(mac_text));

    const char* raw_data = "MACProtectedData";
    size_t data_len = strlen(raw_data);

    auto sealed_data = sealer.SealData(reinterpret_cast<const uint8_t*>(raw_data), data_len);
    ASSERT_TRUE(sealed_data.has_value());

    auto unsealed_data = sealer.UnsealData(sealed_data->data(), sealed_data->size());
    ASSERT_TRUE(unsealed_data.has_value());

    ASSERT_EQ(unsealed_data->additional_mac_text, std::vector<uint8_t>(mac_text, mac_text + sizeof(mac_text)));

    ASSERT_EQ(std::string(unsealed_data->decrypted_text.begin(), unsealed_data->decrypted_text.end()),
              "MACProtectedData");

    auto unsealed_data_2 = sealer.UnsealData(sealed_data.value());
    ASSERT_TRUE(unsealed_data_2.has_value());

    ASSERT_EQ(unsealed_data_2->additional_mac_text, std::vector<uint8_t>(mac_text, mac_text + sizeof(mac_text)));

    ASSERT_EQ(std::string(unsealed_data_2->decrypted_text.begin(), unsealed_data_2->decrypted_text.end()),
              "MACProtectedData");
}

TEST(SealHandlerTestSuite, TestSealingByMrenclaveAndMrsigner) {
    SealHandler sealer(SGX_KEYPOLICY_MRENCLAVE | SGX_KEYPOLICY_MRSIGNER);
    uint8_t mac_text[] = {0xAA, 0xBB, 0xCC, 0xDD};
    sealer.SetAdditionalMacText(mac_text, sizeof(mac_text));

    const char* raw_data = "MACProtectedData";
    size_t data_len = strlen(raw_data);

    auto sealed_data = sealer.SealData(reinterpret_cast<const uint8_t*>(raw_data), data_len);
    ASSERT_TRUE(sealed_data.has_value());

    auto unsealed_data = sealer.UnsealData(sealed_data->data(), sealed_data->size());
    ASSERT_TRUE(unsealed_data.has_value());

    ASSERT_EQ(unsealed_data->additional_mac_text, std::vector<uint8_t>(mac_text, mac_text + sizeof(mac_text)));

    ASSERT_EQ(std::string(unsealed_data->decrypted_text.begin(), unsealed_data->decrypted_text.end()),
    "MACProtectedData");

    auto unsealed_data_2 = sealer.UnsealData(sealed_data.value());
    ASSERT_TRUE(unsealed_data_2.has_value());

    ASSERT_EQ(unsealed_data_2->additional_mac_text, std::vector<uint8_t>(mac_text, mac_text + sizeof(mac_text)));

    ASSERT_EQ(std::string(unsealed_data_2->decrypted_text.begin(), unsealed_data_2->decrypted_text.end()),
    "MACProtectedData");
}


TEST(SealHandlerTestSuite, TestSealingByAll) {
    // SGX_KEYPOLICY_NOISVPRODID | SGX_KEYPOLICY_CONFIGID | SGX_KEYPOLICY_ISVFAMILYID | SGX_KEYPOLICY_ISVEXTPRODID
    // These parameters can only be used when the enclave enables KSS feature.
    SealHandler sealer(SGX_KEYPOLICY_MRENCLAVE | SGX_KEYPOLICY_MRSIGNER | SGX_KEYPOLICY_NOISVPRODID | SGX_KEYPOLICY_CONFIGID | SGX_KEYPOLICY_ISVFAMILYID | SGX_KEYPOLICY_ISVEXTPRODID);
    uint8_t mac_text[] = {0xAA, 0xBB, 0xCC, 0xDD};
    sealer.SetAdditionalMacText(mac_text, sizeof(mac_text));

    const char* raw_data = "MACProtectedData";
    size_t data_len = strlen(raw_data);

    auto sealed_data = sealer.SealData(reinterpret_cast<const uint8_t*>(raw_data), data_len);
    ASSERT_TRUE(sealed_data.has_value());

    auto unsealed_data = sealer.UnsealData(sealed_data->data(), sealed_data->size());
    ASSERT_TRUE(unsealed_data.has_value());

    ASSERT_EQ(unsealed_data->additional_mac_text, std::vector<uint8_t>(mac_text, mac_text + sizeof(mac_text)));

    ASSERT_EQ(std::string(unsealed_data->decrypted_text.begin(), unsealed_data->decrypted_text.end()),
              "MACProtectedData");

    auto unsealed_data_2 = sealer.UnsealData(sealed_data.value());
    ASSERT_TRUE(unsealed_data_2.has_value());

    ASSERT_EQ(unsealed_data_2->additional_mac_text, std::vector<uint8_t>(mac_text, mac_text + sizeof(mac_text)));

    ASSERT_EQ(std::string(unsealed_data_2->decrypted_text.begin(), unsealed_data_2->decrypted_text.end()),
              "MACProtectedData");
}
