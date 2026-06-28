#include "ssgx_config_u.h"

#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nlohmann/json.hpp"
using JSON = nlohmann::json;

namespace ssgx {
namespace config_u {

/* Recursive query node
 *
 * Parameters:
 *      root[in] - the root of node
 *      key_arr_begin[in] - the begin of key array
 *      key_arr_end[in] - the end of key array
 * Return:
 *      Node found
 */
const toml::ordered_value& query_node(const toml::ordered_value& root, const JSON::iterator& key_arr_begin,
                                       const JSON::iterator& key_arr_end) {
    if (key_arr_begin == key_arr_end)
        return root;
    return root.is_array()
               ? query_node(toml::get<std::vector<toml::ordered_value>>(root).at(key_arr_begin->get<std::size_t>()),
                            key_arr_begin + 1, key_arr_end)
               : query_node(toml::find(root, key_arr_begin->get<std::string>()),
                            key_arr_begin + 1, key_arr_end);
}

/* Find node in toml
 *
 * Parameters:
 *      path[in] - path to the node: "[\"Address\", \"IP\"]"
 *      context[in] - the root of node
 *      result_value[out] - the node we want to find
 * Return:
 *      0 - Success
 *      <0 -  Failed
 */
bool find_toml_node(const char* path, const toml::ordered_value* context, toml::ordered_value& toml_obj) {
    // parse json from input string
    try {
        JSON paths_json = JSON::parse(path);
        if (paths_json.empty() || !paths_json.is_array())
            return false;
        toml_obj = query_node(*context, paths_json.begin(), paths_json.end());
    } catch (...) {
        return false;
    }
    return true;
}

// Prevent plaintext recovery via raw block device by zeroing the file content before it is freed.
int zero_file_content(const char* file_path) {
    struct stat st{};
    if (stat(file_path, &st) != 0) {
        return -1;
    }
    if (st.st_size == 0) {
        return 0;
    }
    int fd = open(file_path, O_RDWR);
    if (fd == -1) {
        return -2;
    }
    void* mapped = mmap(nullptr, static_cast<size_t>(st.st_size),
                        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        close(fd);
        return -3;
    }
    explicit_bzero(mapped, static_cast<size_t>(st.st_size));
    const int rc = msync(mapped, static_cast<size_t>(st.st_size), MS_SYNC);
    munmap(mapped, static_cast<size_t>(st.st_size));
    close(fd);
    return rc != 0 ? -4 : 0;
}

} // namespace config_u
} // namespace ssgx