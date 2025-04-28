#include <stdexcept>

#include "nlohmann/json.hpp"

#include "ssgx_http_t_structs.h"

#include "http_utils.h"

namespace ssgx {
namespace http_t {

void Response::SetStatusCode(HttpStatusCode status_code) {
    status_code_ = status_code;
}

HttpStatusCode Response::StatusCode() const {
    return status_code_;
}

void Response::SetHeaders(const TypeHeaders& headers) {
    headers_ = headers;
}

const TypeHeaders& Response::Headers() const {
    return headers_;
}

void Response::SetHeader(const std::string& key, const std::string& val) {
    if (!detail::HasCRLF(key) && !detail::HasCRLF(val)) {
        headers_.emplace(key, val);
    }
}

void Response::SetHeader(const std::string& key, int64_t val) {
    SetHeader(key, std::to_string(val));
}

bool Response::HasHeader(const std::string& key) const {
    return headers_.find(key) != headers_.end();
}

std::string Response::GetHeaderValue(const std::string& key, const char* def) const {
    return detail::GetHeaderValue(headers_, key, "");
}

uint64_t Response::GetHeaderValueUint64(const std::string& key, uint64_t def) const {
    return detail::GetHeaderValueUint64(headers_, key, def);
}

void Response::SetBody(const char* buf, size_t buf_len) {
    if (buf && buf_len > 0) {
        body_.assign(buf, buf_len);
    } else {
        body_ == "";
    }
}

void Response::SetBody(const std::string& body) {
    body_ = body;
}

const std::string& Response::Body() const {
    return body_;
}

bool Response::FromJsonStrWithoutBody(const std::string& json_str) {
    nlohmann::json root_obj;
    try {
        root_obj = nlohmann::json::parse(json_str);
    } catch (const std::exception& e) {
        return false;
    }

    if (root_obj.contains("status_code") && !root_obj["status_code"].is_null()) {
        SetStatusCode(static_cast<HttpStatusCode>(root_obj["status_code"].get<unsigned short>()));
    }

    if (root_obj.contains("headers") && !root_obj["headers"].is_null()) {
        nlohmann::json headers_obj = root_obj["headers"];
        for (auto it = headers_obj.begin(); it != headers_obj.end(); ++it) {
            SetHeader(it.key(), it.value().get<std::string>());
        }
    }

    return true;
}

bool Response::ToJsonStrWithoutBody(std::string& json_str) const {
    nlohmann::json root_obj;
    root_obj["status_code"] = status_code_;
    if (!Headers().empty()) {
        nlohmann::json headers_obj;
        for (const auto& item : Headers()) {
            headers_obj[item.first] = item.second;
        }
        root_obj["headers"] = headers_obj;
    }
    json_str = root_obj.dump();
    return true;
}

void Response::SetResp(const char* s, size_t n, const std::string& content_type, HttpStatusCode status) {
    if (s && n > 0) {
        body_.assign(s, n);
    } else {
        body_ = "";
    }
    status_code_ = status;

    auto rng = headers_.equal_range("Content-Type");
    headers_.erase(rng.first, rng.second);
    SetHeader("Content-Type", content_type);
    SetHeader("Content-Length", std::to_string(body_.size()));
}

void Response::SetResp(const std::string& s, const std::string& content_type, HttpStatusCode status) {
    return SetResp(s.c_str(), s.size(), content_type, status);
}

void Response::SetRespRedirect(const std::string& url, HttpStatusCode status_code) {
    if (url.empty() || detail::HasCRLF(url)) {
        return; // Invalid URL, do nothing
    }

    SetHeader("Location", url);

    // Ensure the status code is a valid redirect
    if (static_cast<int>(status_code) < 300 || static_cast<int>(status_code) >= 400) {
        status_code = HttpStatusCode::Found302; // Default to 302 Found
    }
    status_code_ = status_code;

    body_ = "<html><body><h1>Redirecting...</h1></body></html>";
    SetHeader("Content-Type", "text/html; charset=utf-8");
    SetHeader("Content-Length", std::to_string(body_.length()));
}

void Response::SetResp404NotFound() {
    const std::string body = "<html><body><h1>404 Not Found</h1></body></html>";
    status_code_ = HttpStatusCode::NotFound404; // Set the HTTP status code

    SetHeader("Content-Type", "text/html; charset=utf-8");      // Set the content type
    SetHeader("Content-Length", std::to_string(body.length())); // Set the content length

    body_ = body; // Set the response body
}

void Response::SetResp500InternalServerError() {
    const std::string body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
    status_code_ = HttpStatusCode::InternalServerError500; // Correctly set the status code

    SetHeader("Content-Type", "text/html; charset=utf-8");      // No extra spaces
    SetHeader("Content-Length", std::to_string(body.length())); // Convert length to string

    body_ = body; // Set the response body
}

void Response::SetResp403Forbidden() {
    const std::string body = "<html><body><h1>403 Forbidden</h1></body></html>";
    status_code_ = HttpStatusCode::Forbidden403; // Correctly set the HTTP status code

    SetHeader("Content-Type", "text/html; charset=utf-8");      // No extra spaces
    SetHeader("Content-Length", std::to_string(body.length())); // Convert length to string

    body_ = body; // Set the response body
}

void Response::SetResp204NoContent() {
    status_code_ = HttpStatusCode::NoContent204; // Correctly set the HTTP status code

    SetHeader("Content-Length", std::to_string(0)); // No extra spaces, convert 0 to a string

    body_.clear(); // Ensure the body is empty (204 must not have a response body)
}

} // namespace http_t
} // namespace ssgx