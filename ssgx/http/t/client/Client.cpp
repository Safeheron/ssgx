#include <memory>
#include <utility>

#include "mbedtls_client/HttpClient.h"

#include "ssgx_http_t_client.h"

#include "DefaultCA.h"

using namespace ssgx::http_t::mbedtls_client;

namespace ssgx {
namespace http_t {
/**
 * @brief Convert internal error codes to public errorcodes
 *
 * @param internal_error[in] An internal error code from mbedtls_client module.
 * @return Error Return the according error code which is defined in public header file.
 */
static ErrorCode MapErrorCode(HttpError internal_error) {
    switch (internal_error) {
    case HttpError::OK:
        return ErrorCode::Success;
    case HttpError::InvalidParam:
        return ErrorCode::InvalidParam;
    case HttpError::InvalidCall:
        return ErrorCode::InvalidCall;
    case HttpError::InvalidUrl:
        return ErrorCode::InvalidUrl;
    case HttpError::SetSeedFailed:
        return ErrorCode::SetSeedFailed;
    case HttpError::CACertsWrong:
        return ErrorCode::ServerCAError;
    case HttpError::SSLConfigFailed:
        return ErrorCode::SSLConfigFailed;
    case HttpError::SSLSetupFailed:
        return ErrorCode::SSLSetupFailed;
    case HttpError::SSLHostNameWrong:
        return ErrorCode::SSLHostWrong;
    case HttpError::HandShakeFailed:
        return ErrorCode::SSLHandShakeFailed;
    case HttpError::VerifyCACertsFailed:
        return ErrorCode::VerifyCertFailed;
    case HttpError::ConnectFailed:
        return ErrorCode::ConnectFailed;
    case HttpError::WriteFaild:
        return ErrorCode::WriteFailed;
    case HttpError::ReadFailed:
        return ErrorCode::ReadFailed;
    case HttpError::MallocFailed:
        return ErrorCode::MallocFailed;
    default:
        return ErrorCode::Unknown;
    }
}

static std::string MapErrorMessage(const ErrorCode error) {
    switch (error) {
    case ErrorCode::Success:
        return "Success (no error)";
    case ErrorCode::InvalidParam:
        return "Invalid parameters, maybe some parameters are null";
    case ErrorCode::InvalidCall:
        return "Invalid call stack";
    case ErrorCode::InvalidUrl:
        return "The specified url is invalid";
    case ErrorCode::SetSeedFailed:
        return "Internal error! Failed to set seed for random number";
    case ErrorCode::ServerCAError:
        return "Server's CA certificates chain is invalid";
    case ErrorCode::SSLConfigFailed:
        return "Failed to configure connection setting";
    case ErrorCode::SSLSetupFailed:
        return "SSL setup failed";
    case ErrorCode::SSLHostWrong:
        return "SSL set host name failed";
    case ErrorCode::SSLHandShakeFailed:
        return "Handshake with the server failed";
    case ErrorCode::VerifyCertFailed:
        return "Verify server's certificates failed";
    case ErrorCode::ConnectFailed:
        return "HTTP connection failed";
    case ErrorCode::WriteFailed:
        return "Send data to tje server failed";
    case ErrorCode::ReadFailed:
        return "Read data from the server failed";
    case ErrorCode::MallocFailed:
        return "Internal error! Failed to malloc memory";
    case ErrorCode::Unknown:
        return "Unknown";
    default:
        break;
    }

    return "Invalid";
}

Client::Client(const std::string& host) : host_(host), server_ca_(SSGX_DEFAULT_CA) {
}

Client::Client(const std::string& host, int port) : Client(host, port, SSGX_DEFAULT_CA) {
}

Client::Client(const std::string& host, int port, const std::string& server_ca)
    : host_(host), port_(port), server_ca_(server_ca) {
}

Result Client::Get(const std::string& path, int time_out_sec) {
    return Get(path, TypeHeaders (), TypeParams(), time_out_sec);
}

Result Client::Get(const std::string& path, const TypeHeaders& headers, int time_out_sec) {
    return Get(path, headers, TypeParams(), time_out_sec);
}

Result Client::Get(const std::string& path, const TypeParams& params, int time_out_sec) {
    return Get(path, TypeHeaders(), params, time_out_sec);
}

Result Client::Get(const std::string& path, const TypeHeaders& headers, const TypeParams& params, int time_out_sec) {
    return SendRequest("GET", path, headers, params, "", "", time_out_sec);
}

Result Client::Post(const std::string& path, int time_out_sec) {
    return Post(path, std::string(), std::string(), time_out_sec);
}

Result Client::Post(const std::string& path, const TypeHeaders& headers, int time_out_sec) {
    return Post(path, headers, std::string(), std::string(), time_out_sec);
}

Result Client::Post(const std::string& path, const char* body, size_t content_length, const std::string& content_type,
                    int time_out_sec) {
    return Post(path, std::string(body, content_length), content_type, time_out_sec);
}

Result Client::Post(const std::string& path, const TypeHeaders& headers, const char* body, size_t content_length,
                    const std::string& content_type, int time_out_sec) {
    return Post(path, headers, std::string(body, content_length), content_type, time_out_sec);
}

Result Client::Post(const std::string& path, const std::string& body, const std::string& content_type,
                    int time_out_sec) {
    return Post(path, TypeHeaders(), body, content_type, time_out_sec);
}

Result Client::Post(const std::string& path, const TypeHeaders& headers, const std::string& body,
                    const std::string& content_type, int time_out_sec) {
    return SendRequest("POST", path, headers, TypeParams(), body, content_type, time_out_sec);
}

Result Client::SendRequest(const std::string& method, const std::string& path, const TypeHeaders& headers,
                           const TypeParams& params, const std::string& body, const std::string& content_type,
                           int time_out_sec) const {
    // Request path must be provided
    if (path.empty()) {
        return {nullptr, Error(ErrorCode::InvalidParam, "Path must be provided.", "")};
    }

    std::list<std::string> headers_list;
    std::list<std::string> params_list;

    for (const auto& [key, value] : headers) {
        headers_list.push_back(key + ": " + value);
    }
    for (const auto& [key, value] : params) {
        params_list.push_back(key + "=" + value);
    }
    if (!content_type.empty()) {
        headers_list.push_back("Content-Type: " + content_type);
    }

    httpparser::Response resp_data;
    HttpClient client(host_, port_, server_ca_);
    HttpResult ret = (method == "GET") ? client.Get(path, headers_list, params_list, time_out_sec, resp_data)
                                       : client.Post(path, headers_list, body, time_out_sec, resp_data);

    if (ret.Code() != HttpError::OK) {
        return {nullptr, Error(MapErrorCode(ret.Code()), MapErrorMessage(MapErrorCode(ret.Code())), ret.Message())};
    }

    auto resp = std::make_unique<Response>();
    resp->SetStatusCode(static_cast<HttpStatusCode>(resp_data.statusCode));
    resp->SetBody(std::string(resp_data.content.data(), resp_data.content.size()));

    for (const auto& item : resp_data.headers) {
        resp->SetHeader(item.name, item.value);
    }

    return {std::move(resp), Error(ErrorCode::Success, "OK", "Success")};
}

} // namespace http_t
} // namespace ssgx