#ifndef SAFEHERON_SGX_TRUSTED_SEAL_HANDLER_H
#define SAFEHERON_SGX_TRUSTED_SEAL_HANDLER_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "sgx_attributes.h"
#include "sgx_key.h"

namespace ssgx {
namespace utils_t {

/**
 * @brief Represents unsealed data, including decrypted text and optional additional MAC text.
 *
 * This structure holds the decrypted content retrieved from a sealed SGX data structure. It includes:
 * - `additional_mac_text`: Optional MAC-protected additional data.
 * - `decrypted_text`: The original plaintext retrieved from the sealed data.
 */
struct UnsealedData {
    std::vector<uint8_t> additional_mac_text{}; ///< MAC-protected additional data.
    std::vector<uint8_t> decrypted_text{};      ///< The decrypted content.
};

/**
 * @brief Provides SGX sealing and unsealing operations.
 *
 * This class allows secure encryption and decryption of data using SGX's AES-GCM sealing mechanism. It supports both
 * vector-based and pointer-based interfaces for flexible data handling.
 */
class SealHandler {
  private:
    uint16_t key_policy_;                      ///< Key policy for SGX sealing.
    sgx_attributes_t attribute_mask_{};        ///< Enclave attributes used for key derivation.
    sgx_misc_select_t misc_mask_;              ///< Miscellaneous mask used in key derivation.
    std::vector<uint8_t> additional_mac_text_{}; ///< Optional additional data to protect with MAC.
    std::string last_error_;                   ///< Stores the last error message.

  public:
    /**
     * @brief Constructs a `SealHandler` with a specified key policy.
     * @param key_policy The key policy for sealing operations (default: SGX_KEYPOLICY_MRENCLAVE).
     */
    explicit SealHandler(uint16_t key_policy = SGX_KEYPOLICY_MRENCLAVE);

    /**
     * @brief Sets the full SGX attribute mask for sealing or key derivation operations.
     *
     * The SGX attribute mask typically includes both:
     *  - `flags` (e.g., SGX_FLAGS_INITTED, SGX_FLAGS_DEBUG)
     *  - `xfrm` (e.g., extended features like AVX, SSE)
     *
     * @param attribute_mask The combined SGX attributes to apply during key derivation.
     */
    [[maybe_unused]] void SetAttributeMask(sgx_attributes_t attribute_mask) {
        attribute_mask_ = attribute_mask;
    }

    /**
     * @brief Sets the miscellaneous mask for key derivation.
     * @param misc_mask The miscellaneous select mask.
     */
    [[maybe_unused]] void SetMiscMask(sgx_misc_select_t misc_mask) {
        misc_mask_ = misc_mask;
    }

    /**
     * @brief Sets additional data that will be protected by MAC but not encrypted.
     * @param mac_text Pointer to additional data.
     * @param length Length of the additional data.
     */
    void SetAdditionalMacText(const uint8_t* mac_text, uint32_t length) {
        additional_mac_text_.assign(mac_text, mac_text + length);
    }

    /**
     * @brief Seals data using AES-GCM encryption.
     * @param text_to_encrypt The plaintext data to be sealed.
     * @return `std::optional` containing the sealed data, or `std::nullopt` on failure. Use `GetLastError()` to
     * retrieve the error message.
     */
    std::optional<std::vector<uint8_t>> SealData(const std::vector<uint8_t>& text_to_encrypt);

    /**
     * @brief Seals data using AES-GCM encryption (pointer-based version).
     * @param text_to_encrypt Pointer to plaintext data.
     * @param length Size of the plaintext data.
     * @return `std::optional` containing the sealed data, or `std::nullopt` on failure. Use `GetLastError()` to
     * retrieve the error message.
     */
    std::optional<std::vector<uint8_t>> SealData(const uint8_t* text_to_encrypt, uint32_t length);

    /**
     * @brief Unseals previously sealed data and retrieves the original plaintext.
     * @param sealed_data The sealed data to be unsealed.
     * @return `std::optional<UnsealedData>` containing the unsealed content, or `std::nullopt` on failure. Use
     * `GetLastError()` to retrieve the error message.
     */
    std::optional<UnsealedData> UnsealData(const std::vector<uint8_t>& sealed_data);

    /**
     * @brief Unseals previously sealed data using pointer-based input.
     * @param sealed_data Pointer to the sealed data.
     * @param length Size of the sealed data.
     * @return `std::optional<UnsealedData>` containing the unsealed content, or `std::nullopt` on failure. Use
     * `GetLastError()` to retrieve the error message.
     */
    std::optional<UnsealedData> UnsealData(const uint8_t* sealed_data, uint32_t length);

    /**
     * @brief Retrieves the last error message.
     * @return A string describing the last encountered error.
     */
    [[nodiscard]] std::string GetLastError() const {
        return last_error_;
    }
};

} // namespace utils_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_SEAL_HANDLER_H
