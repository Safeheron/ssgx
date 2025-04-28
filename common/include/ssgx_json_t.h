#ifndef SAFEHERON_SGX_TRUSTED_JSON_H
#define SAFEHERON_SGX_TRUSTED_JSON_H

#include <string>

#include "nlohmann/json_fwd.hpp"

namespace ssgx {
/**
 * @namespace ssgx::json_t
 * @brief This module is a lightweight wrapper for nlohmann::json, designed for extracting values from a JSON object.
 */
namespace json_t {

/**
 * @brief Extract a string from a JSON object.
 * @param[in] root json object
 * @param[in] node_name key in a JSON object
 * @param[out] value value in a JSON object
 * @param[out] err_msg error message
 * @return Return true if successful; otherwise, return false
 */
bool fetch_json_string_node(const nlohmann::json& root, const std::string& node_name, std::string& value,
                            std::string& err_msg);

/**
 * @brief Extract an integer from a JSON object.
 * @param[in] root json object
 * @param[in] node_name key in a JSON object
 * @param[out] value value in a JSON object
 * @param[out] err_msg error message
 * @return Return true if successful; otherwise, return false
 */
bool fetch_json_int_node(const nlohmann::json& root, const std::string& node_name, int& value, std::string& err_msg);

/**
 * @brief Extract a boolean value from a JSON object.
 * @param[in] root
 * @param[in] node_name
 * @param[out] value
 * @param[out] err_msg
 * @return Return true if successful; otherwise, return false
 */
bool fetch_json_bool_node(const nlohmann::json& root, const std::string& node_name, bool& value, std::string& err_msg);

/**
 * @brief Extract a long integer from a JSON object.
 * @param[in] root json object
 * @param[in] node_name key in a JSON object
 * @param[out] value value in a JSON object
 * @param[out] err_msg error message
 * @return Return true if successful; otherwise, return false
 */
bool fetch_json_long_node(const nlohmann::json& root, const std::string& node_name, long& value, std::string& err_msg);

/**
 * @brief Extract a json array from a JSON object.
 * @param[in] root json object
 * @param[in] node_name key in a JSON object
 * @param[out] value value in a JSON object
 * @param[out] err_msg error message
 * @return Return true if successful; otherwise, return false
 */
bool fetch_json_array_node(const nlohmann::json& root, const std::string& node_name, nlohmann::json& value,
                           std::string& err_msg);

/**
 * @brief Extract a json object from a JSON object.
 * @param[in] root json object
 * @param[in] node_name key in a JSON object
 * @param[out] value value in a JSON object
 * @param[out] err_msg error message
 * @return Return true if successful; otherwise, return false
 */
bool fetch_json_object_node(const nlohmann::json& root, const std::string& node_name, nlohmann::json& value,
                            std::string& err_msg);

} // namespace json_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_JSON_H
