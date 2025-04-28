#include <cstdint>
#include <optional>

#include "sgx_error.h"
#include "sgx_tseal.h"

#include "ssgx_utils_t_seal_handler.h"

#include "../../../common/internal_check.h"
#include "../../../common/tseal_migration_attr.h"

namespace ssgx {
namespace utils_t {

SealHandler::SealHandler(uint16_t key_policy) : key_policy_(key_policy), misc_mask_(TSEAL_DEFAULT_MISCMASK) {
    attribute_mask_.flags = TSEAL_DEFAULT_FLAGSMASK;
    attribute_mask_.xfrm = 0x0;
}

std::optional<std::vector<uint8_t>> SealHandler::SealData(const std::vector<uint8_t>& text_to_encrypt) {
    return SealData(text_to_encrypt.data(), text_to_encrypt.size());
}

std::optional<std::vector<uint8_t>> SealHandler::SealData(const uint8_t* text_to_encrypt, uint32_t length) {
    if ((text_to_encrypt == nullptr) || length == 0) {
        last_error_ = "Invalid input data for sealing.";
        return std::nullopt;
    }

    if (!is_valid_key_policy(key_policy_)) {
        last_error_ = "Invalid key policy.";
        return std::nullopt;
    }

    if (!is_valid_attribute_mask(attribute_mask_)) {
        last_error_ = "Invalid attribute mask.";
        return std::nullopt;
    }

    // Calculate required sealed data size
    uint32_t sealed_data_size = sgx_calc_sealed_data_size(additional_mac_text_.size(), length);
    if (sealed_data_size == UINT32_MAX) {
        last_error_ = "Failed to calculate sealed data size.";
        return std::nullopt;
    }

    std::vector<uint8_t> sealed_data(sealed_data_size);

    sgx_status_t status =
        sgx_seal_data_ex(key_policy_, attribute_mask_, misc_mask_, additional_mac_text_.size(),
                         additional_mac_text_.empty() ? nullptr : additional_mac_text_.data(), length, text_to_encrypt,
                         sealed_data_size, reinterpret_cast<sgx_sealed_data_t*>(sealed_data.data()));

    if (status != SGX_SUCCESS) {
        last_error_ = "Sealing operation failed with error code: " + std::to_string(status);
        return std::nullopt;
    }

    return sealed_data;
}

std::optional<UnsealedData> SealHandler::UnsealData(const std::vector<uint8_t>& sealed_data) {
    return UnsealData(sealed_data.data(), sealed_data.size());
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::optional<UnsealedData> SealHandler::UnsealData(const uint8_t* sealed_data, uint32_t length) {
    if ((sealed_data == nullptr) || length < sizeof(sgx_sealed_data_t)) {
        last_error_ = "Invalid sealed data.";
        return std::nullopt;
    }

    const auto* sealed_data_ptr = reinterpret_cast<const sgx_sealed_data_t*>(sealed_data);

    uint32_t add_mac_text_size = sgx_get_add_mac_txt_len(sealed_data_ptr);
    uint32_t encrypted_text_size = sgx_get_encrypt_txt_len(sealed_data_ptr);

    if (add_mac_text_size == UINT32_MAX || encrypted_text_size == UINT32_MAX) {
        last_error_ = "Failed to retrieve sealed data sizes.";
        return std::nullopt;
    }

    UnsealedData result;
    result.additional_mac_text.resize(add_mac_text_size);
    result.decrypted_text.resize(encrypted_text_size);

    sgx_status_t status = sgx_unseal_data(
        sealed_data_ptr, result.additional_mac_text.empty() ? nullptr : result.additional_mac_text.data(),
        &add_mac_text_size, result.decrypted_text.data(), &encrypted_text_size);

    if (status != SGX_SUCCESS) {
        last_error_ = "Unsealing operation failed with error code: " + std::to_string(status);
        return std::nullopt;
    }

    return result;
}

} // namespace utils_t
} // namespace ssgx