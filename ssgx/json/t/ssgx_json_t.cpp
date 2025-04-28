#include "ssgx_json_t.h"

#include <functional>
#include <string>

#include "nlohmann/json.hpp"

namespace ssgx {
namespace json_t {

auto isNumberInteger = [](const nlohmann::json& node) { return node.is_number_integer(); };
auto isString = [](const nlohmann::json& node) { return node.is_string(); };
auto isArray = [](const nlohmann::json& node) { return node.is_array(); };
auto isObject = [](const nlohmann::json& node) { return node.is_object(); };
auto isBoolean = [](const nlohmann::json& node) { return node.is_boolean(); };

template <typename T>
bool fetch_json_node_value(const nlohmann::json& root, const std::string& node_name, T& value,
                           std::function<bool(const nlohmann::json&)> check_type_func, std::string& err_msg) {
    char msg[256] = {0};

    try {
        auto it = root.find(node_name);
        if (it == root.end()) {
            snprintf(msg, sizeof(msg) - 1, "Invalid json! node '%s' is not exist!", node_name.c_str());
            err_msg = msg;
            return false;
        }
        if (!check_type_func(it.value())) {
            snprintf(msg, sizeof(msg) - 1, "Invalid json! node '%s' does not have the expected type!",
                     node_name.c_str());
            err_msg = msg;
            return false;
        }
        value = it->get<T>();
    } catch (nlohmann::json::exception& e) {
        snprintf(msg, sizeof(msg) - 1, "Invalid json! Encounter an exception: %s", e.what());
        err_msg = msg;
        return false;
    }

    return true;
}

bool fetch_json_string_node(const nlohmann::json& root, const std::string& node_name, std::string& value,
                            std::string& err_msg) {
    return fetch_json_node_value(root, node_name, value, isString, err_msg);
}

bool fetch_json_int_node(const nlohmann::json& root, const std::string& node_name, int& value, std::string& err_msg) {
    return fetch_json_node_value(root, node_name, value, isNumberInteger, err_msg);
}

bool fetch_json_bool_node(const nlohmann::json& root, const std::string& node_name, bool& value, std::string& err_msg) {
    return fetch_json_node_value(root, node_name, value, isBoolean, err_msg);
}

bool fetch_json_long_node(const nlohmann::json& root, const std::string& node_name, long& value, std::string& err_msg) {
    return fetch_json_node_value(root, node_name, value, isNumberInteger, err_msg);
}

bool fetch_json_array_node(const nlohmann::json& root, const std::string& node_name, nlohmann::json& value,
                           std::string& err_msg) {
    return fetch_json_node_value(root, node_name, value, isArray, err_msg);
}

bool fetch_json_object_node(const nlohmann::json& root, const std::string& node_name, nlohmann::json& value,
                            std::string& err_msg) {
    return fetch_json_node_value(root, node_name, value, isObject, err_msg);
}

} // namespace json_t
} // namespace ssgx
