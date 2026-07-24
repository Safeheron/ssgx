#include <stdexcept>

#include "nlohmann/json.hpp"

#include "ssgx_http_t_structs.h"

#include "http_utils.h"

namespace ssgx {
namespace http_t {

void Request::SetMethod(const char* buf, size_t buf_len) {
    if (buf && buf_len > 0) {
        method_.assign(buf, buf_len);
    } else {
        method_ = "";
    }
}

void Request::SetMethod(const std::string& method) {
    method_ = method;
}

std::string Request::Method() const {
    return method_;
}

void Request::SetPath(const char* buf, size_t buf_len) {
    if (buf && buf_len > 0) {
        path_.assign(buf, buf_len);
    } else {
        path_ = "";
    }
}

void Request::SetPath(const std::string& path) {
    path_ = path;
}

std::string Request::Path() const {
    return path_;
}

void Request::SetHeaders(const TypeHeaders& headers) {
    headers_ = headers;
}

const TypeHeaders& Request::Headers() const {
    return headers_;
}

bool Request::SetHeader(const std::string& key, const std::string& val) {
    if (detail::HasCRLF(key) || detail::HasCRLF(val)) {
        return false;
    }
    // emplace does not overwrite an existing key; .second is false when the key
    // is already present, so a duplicate key is reported as a failure.
    return headers_.emplace(key, val).second;
}

bool Request::SetHeader(const std::string& key, int64_t val) {
    return SetHeader(key, std::to_string(val));
}

bool Request::HasHeader(const std::string& key) const {
    return detail::HasHeader(headers_, key);
}

std::string Request::GetHeaderValue(const std::string& key, const char* def) const {
    const char* v = detail::GetHeaderValue(headers_, key, def);
    return v ? std::string(v) : std::string();
}

uint64_t Request::GetHeaderValueUint64(const std::string& key, uint64_t def) const {
    return detail::GetHeaderValueUint64(headers_, key, def);
}

void Request::SetParams(const TypeParams& params) {
    params_ = params;
}

const TypeParams& Request::Params() const {
    return params_;
}

bool Request::SetParam(const std::string& key, const std::string& val) {
    if (detail::HasCRLF(key) || detail::HasCRLF(val)) {
        return false;
    }
    // emplace does not overwrite an existing key; .second is false when the key
    // is already present, so a duplicate key is reported as a failure.
    return params_.emplace(key, val).second;
}

bool Request::SetParam(const std::string& key, int64_t val) {
    return SetParam(key, std::to_string(val));
}

bool Request::HasParam(const std::string& key) const {
    return params_.find(key) != params_.end();
}

std::string Request::GetParamValue(const std::string& key, const char* def) const {
    auto it = params_.find(key);
    if (it != params_.end()) {
        return it->second;
    }
    return def;
}

void Request::SetBody(const char* buf, size_t buf_len) {
    if (buf && buf_len > 0) {
        body_.assign(buf, buf_len);
    } else {
        body_ = "";
    }
}

void Request::SetBody(const std::string& body) {
    body_ = body;
}

const std::string& Request::Body() const {
    return body_;
}

bool Request::FromJsonStr(const std::string& json_str) {
    return FromJsonStr(json_str.c_str());
}

bool Request::FromJsonStr(const char* json_str) {
    // The whole body is inside try/catch: parse AND the subsequent get<>()/iteration can throw
    // (e.g. nlohmann type_error when a host-controlled value has the wrong type). Returning false on
    // any exception keeps it from escaping the caller's ECALL and terminating the enclave.
    try {
        nlohmann::json root_obj = nlohmann::json::parse(json_str);

        if (root_obj.contains("method") && !root_obj["method"].is_null()) {
            SetMethod(root_obj["method"].get<std::string>());
        }

        if (root_obj.contains("path") && !root_obj["path"].is_null()) {
            SetPath(root_obj["path"].get<std::string>());
        }

        if (root_obj.contains("headers") && !root_obj["headers"].is_null()) {
            nlohmann::json t_obj = root_obj["headers"];
            for (auto it = t_obj.begin(); it != t_obj.end(); ++it) {
                SetHeader(it.key(), it.value().get<std::string>());
            }
        }
        if (root_obj.contains("params") && !root_obj["params"].is_null()) {
            nlohmann::json t_obj = root_obj["params"];
            for (auto it = t_obj.begin(); it != t_obj.end(); ++it) {
                SetParam(it.key(), it.value().get<std::string>());
            }
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool Request::ToJsonStr(std::string& json_str) const {
    nlohmann::json json_obj;
    json_obj["method"] = Method();
    json_obj["path"] = Path();
    if (!Headers().empty()) {
        nlohmann::json json_item;
        for (const auto& item : Headers()) {
            json_item[item.first] = item.second;
        }
        json_obj["headers"] = json_item;
    }
    if (!Params().empty()) {
        nlohmann::json json_item;
        for (const auto& item : Params()) {
            json_item[item.first] = item.second;
        }
        json_obj["params"] = json_item;
    }
    json_str = json_obj.dump();
    return true;
}

} // namespace http_t
} // namespace ssgx