#include "ssgx_config_u.h"

#include <iostream>

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
const toml::value& query_node(const toml::value& root, const JSON::iterator& key_arr_begin,
                              const JSON::iterator& key_arr_end) {
    if (key_arr_begin == key_arr_end)
        return root;
    return root.is_array()
               ? query_node(toml::get<toml::array>(root).at(*key_arr_begin), key_arr_begin + 1, key_arr_end)
               : query_node(toml::get<toml::table>(root).at(*key_arr_begin), key_arr_begin + 1, key_arr_end);
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
bool find_toml_node(const char* path, const toml::value* context, toml::value& toml_obj) {
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

} // namespace config_u
} // namespace ssgx