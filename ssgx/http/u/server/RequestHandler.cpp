#include "RequestHandler.h"

#include "nlohmann/json.hpp"

#include "sgx_error.h"

#include "ssgx_http_t_u.h"
#include "ssgx_http_u.h"
#include "ssgx_log_u.h"

#include "Poco/StreamCopier.h"
#include "Poco/URI.h"

using JSON = nlohmann::json;

namespace ssgx {
namespace http_u {

void RequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    // Parse headers and parameters in request
    JSON params_json;
    JSON headers_json;
    std::string req_path;
    try {
        Poco::URI uri;
        uri = Poco::URI(request.getURI());
        for (const auto& item : uri.getQueryParameters()) {
            params_json[item.first] = item.second;
        }
        for (const auto& header : request) {
            headers_json[header.first] = header.second;
        }
        req_path = uri.getPath();
    } catch (...) {
        response.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << "Bad Request, invalid URL.";
        return;
    }

    // Parse Query parameters and headers
    JSON req_params_json;
    JSON req_headers_json;
    req_params_json["params"] = params_json;
    req_headers_json["headers"] = headers_json;
    std::string req_params_json_str = req_params_json.dump();
    std::string req_header_json_str = req_headers_json.dump();

    // Parse Body
    std::string req_body_str;
    Poco::StreamCopier::copyToString(request.stream(), req_body_str);

    // Invoke request handler in enclave
    int ret = 0;
    size_t raw_body_size = 0;
    uint8_t* raw_body_ptr = nullptr;
    char* raw_status_headers_ptr = nullptr;

    HttpOnMessageCallback callback = HttpCallbackManager::GetInstance().GetHttpCallback(sgx_eid_);
    if (callback == nullptr) {
        response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
        response.send() << "Not Found: HttpOnMessageCallback.";
        return;
    }

    sgx_status_t result = callback(
        sgx_eid_, &ret, server_id_.c_str(), request.getMethod().c_str(), req_path.c_str(), req_params_json_str.c_str(),
        req_header_json_str.c_str(), (uint8_t*)req_body_str.c_str(), req_body_str.size(), &raw_status_headers_ptr,
        &raw_body_ptr, &raw_body_size);

    // Use unique_ptr to host buffers raw_body_ptr and raw_status_headers_ptr,
    // so that they can be released automatically.
    std::unique_ptr<uint8_t, decltype(&free)> res_body_ptr(raw_body_ptr, free);
    std::unique_ptr<char, decltype(&free)> res_status_headers_ptr(raw_status_headers_ptr, free);

    // Failed to handle request in enclave
    if (result != SGX_SUCCESS || ret != 0) {
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << "Internal Server Error, failed to call enclave API.";
        return;
    }

    // No header string, return errors
    if (!raw_status_headers_ptr) {
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << "Internal Server Error, missing headers in response.";
        return;
    }

    // Construct JSON object for header which is returned by enclave.
    JSON jsonObj;
    try {
        jsonObj = nlohmann::json::parse(raw_status_headers_ptr);
    } catch (const std::exception& e) {
        response.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        response.send() << "Internal Server Error, invalid headers in response.";
        return;
    }

    // Parse status code and set headers to Poco response object
    unsigned short status_code = 500;
    if (jsonObj.contains("status_code") && jsonObj["status_code"].is_number_unsigned()) {
        status_code = jsonObj["status_code"].get<unsigned short>();
    }
    response.setStatus(static_cast<HTTPResponse::HTTPStatus>(status_code));

    // Parse headers and set headers to Poco response object
    if (jsonObj.contains("headers") && !jsonObj["headers"].is_null()) {
        for (auto& [key, value] : jsonObj["headers"].items()) {
            response.set(key, value.get<std::string>());
        }
    }

    // Set body to Poco response object
    if (raw_body_ptr && raw_body_size > 0) {
        response.sendBuffer(raw_body_ptr, raw_body_size);
    } else {
        response.send();
    }
}

} // namespace http_u
} // namespace ssgx