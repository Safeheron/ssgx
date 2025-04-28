#include "ssgx_config_t.h"

#include <string>

#include "nlohmann/json.hpp"

#include "sgx_lfence.h"
#include "sgx_trts.h"

#include "ssgx_config_t_t.h"
#include "ssgx_utils_t.h"

using JSON = nlohmann::json;

namespace ssgx {
namespace config_t {

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
    ssgx_ocall_toml_free(ref_untrusted_toml_obj_);
}

bool TomlConfig::LoadFile(const char* toml_file_path) {
    int ret = 0;
    sgx_status_t status = SGX_ERROR_UNEXPECTED;
    uint64_t toml_ctx = 0;

    if (!toml_file_path || strnlen(toml_file_path, 1) == 0) {
        err_msg_ = ssgx::utils_t::FormatStr("The input toml file path is empty");
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

    // Return error if ptr_value is not outside of enclave
    if (sgx_is_outside_enclave(ptr_value, value_size) == 0) {
        // Don't call FreeOutside() in this case.
        err_msg_ =
            ssgx::utils_t::FormatStr("The memory block pointed to by the pointer passed from the untrusted execution "
                                     "environment contains trusted memory");
        return std::nullopt;
    }

    // Validate value string length
    size_t ptr_value_len = strnlen(ptr_value, value_size);
    if (ptr_value_len != value_size - 1) {
        ssgx::utils_t::FreeOutside(ptr_value, value_size);
        err_msg_ =
            ssgx::utils_t::FormatStr("The length of the string pointed to by the pointer passed from the untrusted "
                                     "execution environment is incorrect");
        return std::nullopt;
    }
    sgx_lfence();

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
    if (sgx_is_outside_enclave(ptr_value, value_size) == 0) {
        // Don't call FreeOutside() in this case.
        err_msg_ =
            ssgx::utils_t::FormatStr("The memory block pointed to by the pointer passed from the untrusted execution "
                                     "environment contains trusted memory");
        return false;
    }
    size_t ptr_value_len = strnlen(ptr_value, value_size);
    if (ptr_value_len != value_size - 1) {
        ssgx::utils_t::FreeOutside(ptr_value, value_size);
        err_msg_ =
            ssgx::utils_t::FormatStr("The length of the string pointed to by the pointer passed from the untrusted "
                                     "execution environment is incorrect");
        return false;
    }
    sgx_lfence();

    values.assign(ptr_value, ptr_value_len);
    ssgx::utils_t::FreeOutside(ptr_value, value_size);
    ptr_value = nullptr;
    return true;
}

} // namespace config_t
} // namespace ssgx
