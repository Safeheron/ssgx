#include <cstdio>
#include <unistd.h>

#include "nlohmann/json.hpp"

#include "ssgx_config_t_u.h"
#include "ssgx_config_u.h"

#include "toml.hpp"
#ifdef __unix__
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <malloc.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#error "We currently only support Linux."
#endif

using JSON = nlohmann::json;
#define LIMITED_TOML_FILE_SIZE (128 * 1024)

/* Create TOML Print string in enclave
 *
 * Parameters:
 *      toml_file_path[in, string] - path of TOML file
 * Return:
 *      0 - Success
 *      <0 -  Failed
 */
extern "C" int32_t ssgx_ocall_toml_create_from_file(const char* toml_file_path, uint64_t* ref_toml_file) {

    if (!toml_file_path || strnlen(toml_file_path, 1) == 0) {
        return -1;
    }
    if (!ref_toml_file) {
        return -2;
    }

    struct stat s_buf{};
    if (stat(toml_file_path, &s_buf) != 0) {
        return -3;
    }
    if (!S_ISREG(s_buf.st_mode)) {
        return -4;
    }
    size_t toml_file_size = s_buf.st_size;
    if (toml_file_size > LIMITED_TOML_FILE_SIZE) {
        return -5;
    }

    auto* context = new (std::nothrow) toml::ordered_value();
    if (!context)
        return -6;

    try {
        *context = toml::parse<toml::ordered_type_config>(toml_file_path);
    } catch (...) {
        delete context;
        context = nullptr;
        return -7;
    }
    *ref_toml_file = reinterpret_cast<uint64_t>(context);

    return 0;
}

/* Find node and get the int value
 *
 * Parameters:
 *      ref_toml_file[in] - reference to TOML Object
 *      path[in, string] - path to the node: "[\"Address\", \"IP\"]"
 *      value[out] - int value
 * Return:
 *      0 - Success
 *      <0 -  Failed
 */
extern "C" int32_t ssgx_ocall_toml_find_int(uint64_t ref_toml_file, const char* path, int64_t* value) {
    auto* context = reinterpret_cast<toml::ordered_value*>(ref_toml_file);
    if (!context) {
        return -1;
    }

    if (!path || strnlen(path, 1) == 0) {
        return -2;
    }
    if (!value) {
        return -3;
    }
    *value = 0;

    toml::ordered_value toml_obj;
    if (!ssgx::config_u::find_toml_node(path, context, toml_obj)) {
        return -4;
    }

    try {
        if (!toml_obj.is_integer())
            return -5;
        *value = toml::get<std::int64_t>(toml_obj);
    } catch (...) {
        return -6;
    }

    return 0;
}

/* Find node and get the string value
 *
 * Parameters:
 *      ref_toml_file[in] - reference to TOML Object
 *      path[in, string] - path to the node: "[\"Address\", \"IP\"]"
 *      value[out, string] - string value
 * Return:
 *      0 - Success
 *      <0 -  Failed
 */
extern "C" int32_t ssgx_ocall_toml_find_str(uint64_t ref_toml_file, const char* path, char** ptr_ptr_value,
                                            int32_t* value_len) {
    std::string str_value;
    auto* context = reinterpret_cast<toml::ordered_value*>(ref_toml_file);
    if (!context) {
        return -1;
    }

    if (!path || strnlen(path, 1) == 0) {
        return -2;
    }
    if (!ptr_ptr_value) {
        return -3;
    }
    if (!value_len) {
        return -4;
    }

    toml::ordered_value result_value;
    if (!ssgx::config_u::find_toml_node(path, context, result_value))
        return -5;

    try {
        if (!result_value.is_string())
            return -6;
        str_value = toml::get<std::string>(result_value);
        if (str_value.empty())
            return 0;
    } catch (...) {
        return -8;
    }

    auto ptr_value = static_cast<char*>(malloc(str_value.length() + 1));
    if (!ptr_value) {
        return -9;
    }
    memset(ptr_value, 0, str_value.length() + 1);
    strncpy(ptr_value, str_value.c_str(), str_value.length() + 1);
    *ptr_ptr_value = ptr_value;
    *value_len = str_value.length() + 1;
    return 0;
}

/* Find node and get the array value
 *
 * Parameters:
 *      ref_toml_file[in] - reference to TOML Object
 *      path[in, string] - path to the node: "[\"Address\", \"IP\"]"
 *      value[out] - array: [\"red\", \"green\"] or [1, 2, 3]
 * Return:
 *      0 - Success
 *      <0 -  Failed
 */
extern "C" int32_t ssgx_ocall_toml_find_array(uint64_t ref_toml_file, const char* path, char** ptr_ptr_value,
                                              int32_t* value_len) {
    std::string array_str;
    auto* context = reinterpret_cast<toml::ordered_value*>(ref_toml_file);
    if (!context) {
        return -1;
    }

    if (!path || strnlen(path, 1) == 0) {
        return -2;
    }
    if (!ptr_ptr_value) {
        return -3;
    }
    if (!value_len) {
        return -4;
    }

    toml::ordered_value toml_obj;
    if (!ssgx::config_u::find_toml_node(path, context, toml_obj)) {
        return -5;
    }

    try {
        if (!toml_obj.is_array())
            return -6;
        auto array = toml::get<std::vector<toml::ordered_value>>(toml_obj);
        if (array.empty())
            return 0;

        const bool all_str =
            std::all_of(array.begin(), array.end(), [](const toml::ordered_value& it) { return it.is_string(); });
        const bool all_int =
            std::all_of(array.begin(), array.end(), [](const toml::ordered_value& it) { return it.is_integer(); });
        // The values in array must be all string or all integer
        if (!all_str && !all_int)
            return -8;

        JSON array_json;
        for (const auto& elem : array) {
            all_str ? array_json.push_back(toml::get<std::string>(elem))
                    : array_json.push_back(toml::get<int64_t>(elem));
        }
        if (array_json.empty())
            return -9;

        array_str = array_json.dump();
        if (array_str.empty())
            return -10;

    } catch (...) {
        return -11;
    }

    auto ptr_value = static_cast<char*>(malloc(array_str.length() + 1));
    if (!ptr_value) {
        return -12;
    }
    memset(ptr_value, 0, array_str.length() + 1);
    strncpy(ptr_value, array_str.c_str(), array_str.length() + 1);
    *ptr_ptr_value = ptr_value;
    *value_len = array_str.length() + 1;

    return 0;
}

/* Free TOML Object
 *
 * Parameters:
 *      ref_toml_file[in] - reference to TOML Object
 * Return:
 *      0 - Success
 *      <0 -  Failed
 */
extern "C" void ssgx_ocall_toml_free(uint64_t ref_toml_file) {
    auto* context = reinterpret_cast<toml::ordered_value*>(ref_toml_file);
    if (context) {
        delete context;
        context = nullptr;
    }
}

/* Rename a .secret key to .sealed and update its value in the in-memory AST.
 * No I/O is performed; call ssgx_ocall_toml_write_file to persist.
 *
 * Parameters:
 *      ref_toml_file[in]    - reference to TOML Object (AST updated in-place)
 *      secret_path_json[in] - JSON array path to the "secret" node: "[\"db\", \"password\", \"secret\"]"
 *      sealed_value[in]     - sealed value string to write
 * Return:
 *      0 - Success
 *      <0 - Failed
 */
extern "C" int32_t ssgx_ocall_toml_seal_in_place(uint64_t ref_toml_file, const char* secret_path_json,
                                                  const char* sealed_value) {
    auto* context = reinterpret_cast<toml::ordered_value*>(ref_toml_file);
    if (!context) {
        return -1;
    }
    if (!secret_path_json || strnlen(secret_path_json, 1) == 0) {
        return -2;
    }
    if (!sealed_value || strnlen(sealed_value, 1) == 0) {
        return -3;
    }

    JSON path_json;
    try {
        path_json = JSON::parse(secret_path_json);
    } catch (...) {
        return -4;
    }
    if (!path_json.is_array() || path_json.size() < 2) {
        return -5;
    }

    toml::ordered_value* cur = context;
    for (size_t i = 0; i + 1 < path_json.size(); ++i) {
        if (!path_json[i].is_string()) {
            return -6;
        }
        if (!cur->is_table()) {
            return -7;
        }
        auto& tbl = toml::get<toml::ordered_table>(*cur);
        auto it = tbl.find(path_json[i].get<std::string>());
        if (it == tbl.end()) {
            return -8;
        }
        cur = &it->second;
    }

    if (!path_json.back().is_string()) {
        return -9;
    }
    const std::string secret_key = path_json.back().get<std::string>();
    if (!cur->is_table()) {
        return -10;
    }
    auto& parent_tbl = toml::get<toml::ordered_table>(*cur);
    auto it = parent_tbl.find(secret_key);
    if (it == parent_tbl.end()) {
        return -11;
    }

    // Defensive: refuse if a sibling "sealed" key already exists, otherwise renaming
    // secret->sealed would create two "sealed" keys — the file would then fail to parse
    // while the plaintext has already been replaced. Bail out without touching the AST.
    if (parent_tbl.find("sealed") != parent_tbl.end()) {
        return -12;
    }

    // ordered_map is vector<pair<Key,Val>> — rename in-place to preserve key position
    it->first = "sealed";
    auto comments = it->second.comments();
    it->second = toml::ordered_value{std::string{sealed_value}};
    it->second.comments() = std::move(comments);

    return 0;
}

/* Write the current in-memory AST to disk, replacing the existing file.
 * Zeroes the original file content before overwriting (mmap/bzero/msync)
 * to prevent plaintext recovery from freed disk blocks.
 *
 * Parameters:
 *      ref_toml_file[in]     - reference to TOML Object
 *      file_path[in, string] - path of TOML file on disk
 * Return:
 *      0 - Success
 *      <0 - Failed
 */
extern "C" int32_t ssgx_ocall_toml_write_file(uint64_t ref_toml_file, const char* file_path) {
    auto* context = reinterpret_cast<toml::ordered_value*>(ref_toml_file);
    if (!context) {
        return -1;
    }
    if (!file_path || strnlen(file_path, 1) == 0) {
        return -2;
    }

    // Step 1: write current AST to tmp file (original still intact on disk)
    const std::string tmp_path = std::string(file_path) + ".ssgxtmp";
    {
        const std::string content = toml::format(*context);
        FILE* f = fopen(tmp_path.c_str(), "w");
        if (!f) {
            return -3;
        }
        if (fwrite(content.data(), 1, content.size(), f) != content.size()) {
            fclose(f);
            unlink(tmp_path.c_str());
            return -4;
        }
        fflush(f);
        fsync(fileno(f));
        fclose(f);
    }

    // Step 2: zero original plaintext while its blocks are still allocated
    if (ssgx::config_u::zero_file_content(file_path) != 0) {
        unlink(tmp_path.c_str());
        return -5;
    }

    // Step 3: atomic rename — new content replaces original
    if (rename(tmp_path.c_str(), file_path) != 0) {
        // Do NOT unlink: the original has already been zeroed in step 2.
        // .ssgxtmp contains the only surviving content — leave it for manual recovery.
        return -6;
    }

    return 0;
}
