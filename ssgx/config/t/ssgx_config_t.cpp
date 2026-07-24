#include "ssgx_config_t.h"

#include <cstring>
#include <string>

#include "nlohmann/json.hpp"

#include "sgx_lfence.h"
#include "sgx_trts.h"

#include "ssgx_config_t_t.h"
#include "ssgx_utils_t.h"
#include "ssgx_utils_t_seal_handler.h"

#include "crypto-suites/crypto-encode/base64.h"

using JSON = nlohmann::json;

namespace ssgx {
namespace config_t {

namespace {

constexpr char kSealedPrefix[] = "ssgxcfg.v1:";
constexpr size_t kSealedPrefixLen = sizeof(kSealedPrefix) - 1;
constexpr int32_t kConfigValueSizeLimit = 128 * 1024;  // matches u-side LIMITED_TOML_FILE_SIZE (config file cap)

} // namespace

static std::string GetPathStr(const std::vector<TomlKey>& path) {
    if (path.empty())
        return "";

    JSON paths_json;
    for (auto it = path.rbegin(); it != path.rend(); ++it) {
        if (it->type_ == TomlKey::KeyType::Integer) {
            paths_json.push_back(it->index_key_);
        } else {
            paths_json.push_back(it->str_key_);
        }
    }
    return paths_json.dump();
}


TomlConfig::~TomlConfig() {
    if (dirty_) SaveFile();
    ssgx_ocall_toml_free(ref_untrusted_toml_obj_);
}

bool TomlConfig::SaveFile() {
    if (!dirty_ || file_path_.empty()) return true;
    int ret = 0;
    sgx_status_t status = ssgx_ocall_toml_write_file(&ret, ref_untrusted_toml_obj_, file_path_.c_str());
    if (status != SGX_SUCCESS) {
        err_msg_ = ssgx::utils_t::FormatStr("ssgx_ocall_toml_write_file failed, sgx status: 0x%x", status);
        return false;
    }
    if (ret < 0) {
        err_msg_ = ssgx::utils_t::FormatStr("ssgx_ocall_toml_write_file failed, error code: %d", ret);
        return false;
    }
    dirty_ = false;
    return true;
}

bool TomlConfig::LoadFile(const char* toml_file_path) {
    int ret = 0;
    sgx_status_t status = SGX_ERROR_UNEXPECTED;
    uint64_t toml_ctx = 0;

    if (!toml_file_path || strnlen(toml_file_path, 1) == 0) {
        err_msg_ = ssgx::utils_t::FormatStr("The input toml file path is empty");
        return false;
    }

    // Refuse to reload into an already-loaded object: overwriting ref_untrusted_toml_obj_ would
    // leak the previous host TOML object, and if it had unsaved changes (dirty_) they would be
    // lost silently. Use a fresh TomlConfig instance to load another file.
    if (ref_untrusted_toml_obj_ != 0) {
        err_msg_ = ssgx::utils_t::FormatStr(
            "A TOML file is already loaded; use a new TomlConfig instance to load another file");
        return false;
    }

    status = ssgx_ocall_toml_create_from_file(&ret, toml_file_path, &toml_ctx);
    if (status != SGX_SUCCESS) {
        err_msg_ = ssgx::utils_t::FormatStr(
            "Enclave ssgx_ocall_toml_create_from_file function call failed, sgx status: 0x%x", status);
        return false;
    }
    if (ret < 0) {
        err_msg_ = ssgx::utils_t::FormatStr(
            "Enclave ssgx_ocall_toml_create_from_file function call failed, error code: %d", ret);
        return false;
    }
    ref_untrusted_toml_obj_ = toml_ctx;
    file_path_.assign(toml_file_path);

    return true;
}

std::optional<int64_t> TomlConfig::GetInteger(const std::vector<TomlKey>& path) {
    int ret = 0;
    int64_t value = 0;
    std::string paths_str;
    sgx_status_t status = SGX_ERROR_UNEXPECTED;

    if (ref_untrusted_toml_obj_ == 0) {
        err_msg_ = ssgx::utils_t::FormatStr("The TOML object has not been initialized");
        return std::nullopt;
    }

    paths_str = GetPathStr(path);
    if (paths_str.empty()) {
        err_msg_ = ssgx::utils_t::FormatStr("The input toml file path is empty");
        return std::nullopt;
    }

    // get integer from toml where is in untrusted memory
    status = ssgx_ocall_toml_find_int(&ret, ref_untrusted_toml_obj_, paths_str.c_str(), &value);
    if (status != SGX_SUCCESS) {
        err_msg_ =
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_toml_find_int function call failed, sgx status: 0x%x", status);
        return std::nullopt;
    }
    if (ret < 0) {
        err_msg_ =
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_toml_find_int function call failed, error code: %d", ret);
        return std::nullopt;
    }

    return value;
}

std::optional<std::string> TomlConfig::GetString(const std::vector<TomlKey>& path) {
    int ret = 0;
    char* ptr_value = nullptr;
    int32_t value_size = 0;
    std::string paths_str;
    std::string value;
    sgx_status_t status = SGX_ERROR_UNEXPECTED;
    if (ref_untrusted_toml_obj_ == 0) {
        err_msg_ = ssgx::utils_t::FormatStr("The TOML object has not been initialized");
        return std::nullopt;
    }

    paths_str = GetPathStr(path);
    if (paths_str.empty()) {
        err_msg_ = ssgx::utils_t::FormatStr("The input toml file path is empty");
        return std::nullopt;
    }

    // get string from toml where is in untrusted memory
    status = ssgx_ocall_toml_find_str(&ret, ref_untrusted_toml_obj_, paths_str.c_str(), &ptr_value, &value_size);
    if (status != SGX_SUCCESS) {
        err_msg_ =
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_toml_find_str function call failed, sgx status: 0x%x", status);
        return std::nullopt;
    }
    if (ret < 0) {
        err_msg_ =
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_toml_find_str function call failed, error code: %d", ret);
        return std::nullopt;
    }

    // Value is an empty string, return true
    if (!ptr_value || value_size <= 0) {
        return "";
    }

    // Reject an over-large value_size early. Pure integer check on value_size (no dereference),
    // so it is safe here and it bounds the strnlen scan below: sgx_is_outside_enclave only proves
    // the range is outside the enclave, not that the host mapped it, so a huge size over a
    // short/unmapped buffer could otherwise make strnlen walk into an unmapped page and fault.
    if (value_size > kConfigValueSizeLimit) {
        ssgx::utils_t::FreeOutside(ptr_value, value_size);
        err_msg_ = ssgx::utils_t::FormatStr("The value size %d exceeds the maximum allowed (%d)",
                                            value_size, kConfigValueSizeLimit);
        return std::nullopt;
    }

    // Return error if ptr_value is not outside of enclave
    if (sgx_is_outside_enclave(ptr_value, value_size) == 0) {
        // Don't call FreeOutside() in this case.
        err_msg_ =
            ssgx::utils_t::FormatStr("The memory block pointed to by the pointer passed from the untrusted execution "
                                     "environment contains trusted memory");
        return std::nullopt;
    }
    sgx_lfence();

    // Validate value string length
    size_t ptr_value_len = strnlen(ptr_value, value_size);
    if (ptr_value_len != value_size - 1) {
        ssgx::utils_t::FreeOutside(ptr_value, value_size);
        err_msg_ =
            ssgx::utils_t::FormatStr("The length of the string pointed to by the pointer passed from the untrusted "
                                     "execution environment is incorrect");
        return std::nullopt;
    }

    // return string
    value.assign(ptr_value, ptr_value_len);
    ssgx::utils_t::FreeOutside(ptr_value, value_size);
    return value;
}

std::optional<std::vector<int64_t>> TomlConfig::GetIntegerArray(const std::vector<TomlKey>& path) {
    std::vector<int64_t> values;

    if (ref_untrusted_toml_obj_ == 0) {
        err_msg_ = ssgx::utils_t::FormatStr("The TOML object has not been initialized");
        return std::nullopt;
    }

    const std::string paths_str = GetPathStr(path);
    if (paths_str.empty()) {
        err_msg_ = ssgx::utils_t::FormatStr("The input toml file path is empty");
        return std::nullopt;
    }

    // Get values in json string
    std::string value_str;
    if (!GetArrayValues(ref_untrusted_toml_obj_, paths_str, value_str)) {
        return std::nullopt;
    }

    // Get an empty array, return it
    if (value_str.empty()) {
        return values;
    }

    // Traverse json nodes, convert values to vector<string>
    try {
        JSON value_json = JSON::parse(value_str);

        // The values in array must be integer
        if (value_json.empty() || !value_json.is_array() ||
            !std::all_of(value_json.begin(), value_json.end(), [](const JSON& it) { return it.is_number_integer(); })) {
            err_msg_ = ssgx::utils_t::FormatStr("The results in the JSON object are incorrect");
            return std::nullopt;
        }

        values.reserve(value_json.size());
        for (auto& item : value_json) {
            values.push_back(item.get<long>());
        }
    } catch (std::exception& e) {
        err_msg_ = ssgx::utils_t::FormatStr("The JSON object containing the results failed to parse, exception msg: %s",
                                            e.what());
        return std::nullopt;
    }

    return values;
}

std::optional<std::vector<std::string>> TomlConfig::GetStringArray(const std::vector<TomlKey>& path) {
    std::vector<std::string> values;

    if (ref_untrusted_toml_obj_ == 0) {
        err_msg_ = ssgx::utils_t::FormatStr("The TOML object has not been initialized");
        return std::nullopt;
    }

    const std::string paths_str = GetPathStr(path);
    if (paths_str.empty()) {
        err_msg_ = ssgx::utils_t::FormatStr("The input toml file path is empty");
        return std::nullopt;
    }

    // Get values in json string
    std::string value_str;
    if (!GetArrayValues(ref_untrusted_toml_obj_, paths_str, value_str)) {
        return std::nullopt;
    }

    // Get an empty array, return it
    if (value_str.empty()) {
        return values;
    }

    // Traverse json nodes, convert values to vector<long>
    try {
        JSON value_json = JSON::parse(value_str);

        // The values in array must be string
        if (value_json.empty() || !value_json.is_array() ||
            !std::all_of(value_json.begin(), value_json.end(), [](const JSON& it) { return it.is_string(); })) {
            err_msg_ = ssgx::utils_t::FormatStr("The results in the JSON object are incorrect");
            return std::nullopt;
        }

        values.reserve(value_json.size());
        for (auto& item : value_json) {
            values.push_back(item.get<std::string>());
        }
    } catch (std::exception& e) {
        err_msg_ = ssgx::utils_t::FormatStr("The JSON object containing the results failed to parse, exception msg: %s",
                                            e.what());
        return std::nullopt;
    }

    return values;
}

bool TomlConfig::GetArrayValues(uint64_t ptr_toml, const std::string& path_str, std::string& values) {
    int ret = 0;
    char* ptr_value = nullptr;
    int32_t value_size = 0;
    sgx_status_t status = SGX_ERROR_UNEXPECTED;

    values.clear();

    if (ptr_toml == 0) {
        err_msg_ = ssgx::utils_t::FormatStr("The TOML object has not been initialized");
        return false;
    }
    if (path_str.empty()) {
        err_msg_ = ssgx::utils_t::FormatStr("The input toml file path is empty");
        return false;
    }

    // get array from toml where is in untrusted memory
    status = ssgx_ocall_toml_find_array(&ret, ref_untrusted_toml_obj_, path_str.c_str(), &ptr_value, &value_size);
    if (status != SGX_SUCCESS) {
        err_msg_ = ssgx::utils_t::FormatStr("Enclave ssgx_ocall_toml_find_array function call failed, sgx status: 0x%x",
                                            status);
        return false;
    }
    if (ret < 0) {
        err_msg_ =
            ssgx::utils_t::FormatStr("Enclave ssgx_ocall_toml_find_array function call failed, error code: %d", ret);
        return false;
    }
    // Value is an empty array, return true
    if (!ptr_value || value_size <= 0) {
        return true;
    }

    // Reject an over-large value_size early (see GetString): pure integer check, bounds the
    // strnlen scan below so a huge size over a short/unmapped buffer cannot make it fault.
    if (value_size > kConfigValueSizeLimit) {
        ssgx::utils_t::FreeOutside(ptr_value, value_size);
        err_msg_ = ssgx::utils_t::FormatStr("The array value size %d exceeds the maximum allowed (%d)",
                                            value_size, kConfigValueSizeLimit);
        return false;
    }

    if (sgx_is_outside_enclave(ptr_value, value_size) == 0) {
        // Don't call FreeOutside() in this case.
        err_msg_ =
            ssgx::utils_t::FormatStr("The memory block pointed to by the pointer passed from the untrusted execution "
                                     "environment contains trusted memory");
        return false;
    }
    sgx_lfence();

    size_t ptr_value_len = strnlen(ptr_value, value_size);
    if (ptr_value_len != value_size - 1) {
        ssgx::utils_t::FreeOutside(ptr_value, value_size);
        err_msg_ =
            ssgx::utils_t::FormatStr("The length of the string pointed to by the pointer passed from the untrusted "
                                     "execution environment is incorrect");
        return false;
    }

    values.assign(ptr_value, ptr_value_len);
    ssgx::utils_t::FreeOutside(ptr_value, value_size);
    ptr_value = nullptr;
    return true;
}

std::optional<std::string> TomlConfig::GetSecretString(const std::vector<TomlKey>& path) {
    if (ref_untrusted_toml_obj_ == 0) {
        err_msg_ = ssgx::utils_t::FormatStr("The TOML object has not been initialized");
        return std::nullopt;
    }
    if (path.empty()) {
        err_msg_ = ssgx::utils_t::FormatStr("Empty path passed to GetSecretString");
        return std::nullopt;
    }
    if (path.front().type_ != TomlKey::KeyType::String) {
        err_msg_ = ssgx::utils_t::FormatStr("GetSecretString leaf key must be a string (arrays not supported)");
        return std::nullopt;
    }

    const std::string dotted_path = [&path]() {
        std::string s;
        for (auto it = path.rbegin(); it != path.rend(); ++it) {
            if (!s.empty())
                s += '.';
            if (it->type_ == TomlKey::KeyType::String) {
                s += it->str_key_;
            } else {
                s += std::to_string(it->index_key_);
            }
        }
        return s;
    }();

    auto with_key = [&](const char* key) {
        auto p = path;
        p.insert(p.begin(), TomlKey(key));
        return p;
    };

    // ----- Phase 1: try <path>.sealed → unseal and return plaintext -----
    {
        auto sealed_opt = GetString(with_key("sealed"));
        if (sealed_opt.has_value()) {
            const std::string& sealed_value = *sealed_opt;
            if (sealed_value.compare(0, kSealedPrefixLen, kSealedPrefix) != 0) {
                err_msg_ = ssgx::utils_t::FormatStr(
                    "Field '%s.sealed' has unknown sealed format prefix (expected '%s')",
                    dotted_path.c_str(), kSealedPrefix);
                return std::nullopt;
            }
            const std::string b64 = sealed_value.substr(kSealedPrefixLen);
            std::string blob_str;
            try {
                blob_str = safeheron::encode::base64::DecodeFromBase64(b64);
            } catch (...) {
                err_msg_ = ssgx::utils_t::FormatStr("Field '%s.sealed' contains invalid base64", dotted_path.c_str());
                return std::nullopt;
            }
            if (blob_str.empty()) {
                err_msg_ = ssgx::utils_t::FormatStr("Field '%s.sealed' has empty sealed blob after base64 decode",
                                                    dotted_path.c_str());
                return std::nullopt;
            }

            ssgx::utils_t::SealHandler sealer(SGX_KEYPOLICY_MRENCLAVE);
            auto unsealed = sealer.UnsealData(reinterpret_cast<const uint8_t*>(blob_str.data()),
                                              static_cast<uint32_t>(blob_str.size()));
            if (!unsealed.has_value()) {
                err_msg_ = ssgx::utils_t::FormatStr(
                    "Failed to unseal field '%s' (possible causes: MRENCLAVE drift, machine drift, tampering): %s",
                    dotted_path.c_str(), sealer.GetLastError().c_str());
                return std::nullopt;
            }
            // Verify the AAD embedded in the blob matches the current field path to prevent
            // cross-field blob copying attacks.
            const std::string stored_aad(unsealed->additional_mac_text.begin(),
                                         unsealed->additional_mac_text.end());
            if (stored_aad != dotted_path) {
                err_msg_ = ssgx::utils_t::FormatStr(
                    "Failed to unseal field '%s': AAD path mismatch (blob was sealed for '%s')",
                    dotted_path.c_str(), stored_aad.c_str());
                return std::nullopt;
            }
            return std::string(unsealed->decrypted_text.begin(), unsealed->decrypted_text.end());
        }
    }

    // ----- Phase 2: try <path>.secret → seal, rewrite the line in the file, return plaintext -----
    {
        auto plain_opt = GetString(with_key("secret"));
        if (plain_opt.has_value()) {
            const std::string& plaintext = *plain_opt;
            if (plaintext.empty()) {
                err_msg_ = ssgx::utils_t::FormatStr("Field '%s.secret' is empty; refusing to seal an empty value",
                                                    dotted_path.c_str());
                return std::nullopt;
            }
            if (file_path_.empty()) {
                err_msg_ = ssgx::utils_t::FormatStr(
                    "Cannot rewrite '%s.secret': file path is unknown (was LoadFile called?)", dotted_path.c_str());
                return std::nullopt;
            }

            ssgx::utils_t::SealHandler sealer(SGX_KEYPOLICY_MRENCLAVE);
            sealer.SetAdditionalMacText(reinterpret_cast<const uint8_t*>(dotted_path.data()),
                                        static_cast<uint32_t>(dotted_path.size()));
            auto sealed = sealer.SealData(reinterpret_cast<const uint8_t*>(plaintext.data()),
                                          static_cast<uint32_t>(plaintext.size()));
            if (!sealed.has_value()) {
                err_msg_ = ssgx::utils_t::FormatStr("Failed to seal field '%s.secret': %s", dotted_path.c_str(),
                                                    sealer.GetLastError().c_str());
                return std::nullopt;
            }
            const std::string b64 =
                safeheron::encode::base64::EncodeToBase64(sealed->data(), sealed->size());
            const std::string encoded_value = std::string(kSealedPrefix) + b64;

            int ret = 0;
            const std::string secret_path_str = GetPathStr(with_key("secret"));
            sgx_status_t status = ssgx_ocall_toml_seal_in_place(
                &ret, ref_untrusted_toml_obj_, secret_path_str.c_str(), encoded_value.c_str());
            if (status != SGX_SUCCESS) {
                err_msg_ = ssgx::utils_t::FormatStr(
                    "ssgx_ocall_toml_seal_in_place failed, sgx status: 0x%x", status);
                return std::nullopt;
            }
            if (ret < 0) {
                err_msg_ = ssgx::utils_t::FormatStr(
                    "ssgx_ocall_toml_seal_in_place failed for field '%s', error code: %d",
                    dotted_path.c_str(), ret);
                return std::nullopt;
            }
            dirty_ = true;
            return plaintext;
        }
    }

    err_msg_ = ssgx::utils_t::FormatStr(
        "Field '%s' is not annotated as a secret (neither '.secret' nor '.sealed' sub-key found)",
        dotted_path.c_str());
    return std::nullopt;
}

} // namespace config_t
} // namespace ssgx
