#ifndef SAFEHERON_SGX_TRUSTED_HTTP_CLIENT_H
#define SAFEHERON_SGX_TRUSTED_HTTP_CLIENT_H

#include <map>
#include <string>

#include "ssgx_http_t_structs.h"

namespace ssgx {
namespace http_t {

/* Time out for connection, in seconds */
constexpr int SSGX_HTTP_CLIENT_TIMEOUT = 5;

/**
 * @brief Error code of the client operation.
 */
enum class ErrorCode {
    Success = 0,            /**< OK */
    InvalidParam = 1,       /**< Invalid parameters, maybe some parameters are nullptr. */
    InvalidCall = 2,        /**< Invalid calling stack. */
    InvalidUrl = 3,         /**< The specified url is invalid. */
    SetSeedFailed = 4,      /**< Failed to set seed for random number. */
    ServerCAError = 5,      /**< Failed to parse ca certificate chain string. */
    SSLConfigFailed = 6,    /**< Failed to configure connection setting. */
    SSLSetupFailed = 7,     /**< Failed to set up ssl. */
    SSLHostWrong = 8,       /**< Failed to set host name. */
    SSLHandShakeFailed = 9, /**< Failed to handshake with the server. */
    VerifyCertFailed = 10,  /**< Failed to verify server's certificates chain. */
    ConnectFailed = 11,     /**< Failed to connect to host. */
    WriteFailed = 12,       /**< Failed to send data to tje server. */
    ReadFailed = 13,        /**< Failed to read data from the server. */
    MallocFailed = 14,      /**< Failed to malloc memory. */

    Unknown = 9999, /**< Unknown error occurred. */
};

/**
 * @brief Represents detailed error information, typically from client operations.
 */
class Error {
  public:
    Error() = default;
    Error(const Error& other) = default;
    Error(ErrorCode error_code, std::string error_msg, std::string internal_err)
        : error_code_(error_code), error_msg_(std::move(error_msg)), internal_err_(std::move(internal_err)) {
    }

    [[nodiscard]] ErrorCode code() const {
        return error_code_;
    }
    [[nodiscard]] const std::string& message() const {
        return error_msg_;
    }
    [[nodiscard]] const std::string& internal_error() const {
        return internal_err_;
    }

  private:
    ErrorCode error_code_ = ErrorCode::Unknown;
    std::string error_msg_;
    std::string internal_err_;
};

/**
 * @brief The result class of the client operation stores the response data of the request or the error information of the client operation.
 */
class Result {
  public:
    Result() = default;
    Result(std::unique_ptr<Response>&& res, const Error& err) : res_(std::move(res)), err_(err) {
    }
    // Response
    explicit operator bool() const {
        return res_ != nullptr;
    }
    bool operator==(std::nullptr_t) const {
        return res_ == nullptr;
    }
    bool operator!=(std::nullptr_t) const {
        return res_ != nullptr;
    }
    [[nodiscard]] const Response& value() const {
        return *res_;
    }
    Response& value() {
        return *res_;
    }
    const Response& operator*() const {
        return *res_;
    }
    Response& operator*() {
        return *res_;
    }
    const Response* operator->() const {
        return res_.get();
    }
    Response* operator->() {
        return res_.get();
    }

    // Error
    [[nodiscard]] Error error() const {
        return err_;
    }

  private:
    std::unique_ptr<Response> res_;
    Error err_ = Error(ErrorCode::Unknown, "Invalid", "Unknown");
};

/**
 * @brief Client class for making HTTP and HTTPS requests.
 *
 * @details This class encapsulates the functionality required to connect to an HTTP/HTTPS
 * server and send requests using common methods like GET and POST. It supports
 * setting custom headers, URL parameters, request bodies, and timeouts.
 * For HTTPS connections, it handles server certificate verification against
 * either the system's default CA certificate store or a user-provided CA certificate.
 *
 */
class Client {
  public:
    /**
     * @brief Constructor to initialize the client with a hostname.
     *
     * 1. The default port will be used for connection.
     * 2. For HTTPS, will use the default ca root certificate store to verify
     * server's certificate.
     *
     * @param host Server hostname.
     */
    explicit Client(const std::string& host);

    /**
     * @brief Constructor to initialize the client with a hostname and port.
     *
     * For HTTPS, will use the default ca root certificate store to verify
     * server's certificate.
     *
     * @param host Server hostname.
     * @param port Server port number.
     */
    explicit Client(const std::string& host, int port);

    /**
     * @brief Constructor to initialize the client with a hostname, port, and
     * server CA certificate.
     *
     * For HTTPS, will use server_ca to verify server's certificate.
     *
     * @param host Server hostname.
     * @param port Server port number.
     * @param server_ca Server CA certificate chain, in PEM format.
     */
    explicit Client(const std::string& host, int port, const std::string& server_ca);

    /**
     * @brief Destructor to clean up resources.
     */
    virtual ~Client() = default;

    /**
     * @brief Sends a GET request.
     * @param path Request path.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Get(const std::string& path, int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

    /**
     * @brief Sends a GET request with HTTP headers.
     * @param path Request path.
     * @param headers HTTP headers.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Get(const std::string& path, const TypeHeaders & headers, int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

    /**
     * @brief Sends a GET request with URL parameters.
     * @param path Request path.
     * @param params URL parameters.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Get(const std::string& path, const TypeParams& params, int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

    /**
     * @brief Sends a GET request with HTTP headers and URL parameters.
     * @param path Request path.
     * @param headers HTTP headers.
     * @param params URL parameters.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Get(const std::string& path, const TypeHeaders& headers, const TypeParams& params,
               int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

    /**
     * @brief Sends a POST request.
     * @param path Request path.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Post(const std::string& path, int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

    /**
     * @brief Sends a POST request with HTTP headers.
     * @param path Request path.
     * @param headers HTTP headers.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Post(const std::string& path, const TypeHeaders& headers, int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

    /**
     * @brief Sends a POST request with a body.
     * @param path Request path.
     * @param body Request body data (C-style string).
     * @param content_length Body length.
     * @param content_type Body MIME type.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Post(const std::string& path, const char* body, size_t content_length, const std::string& content_type,
                int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

    /**
     * @brief Sends a POST request with HTTP headers and a body.
     * @param path Request path.
     * @param headers HTTP headers.
     * @param body Request body data (C-style string).
     * @param content_length Body length.
     * @param content_type Body MIME type.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Post(const std::string& path, const TypeHeaders& headers, const char* body, size_t content_length,
                const std::string& content_type, int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

    /**
     * @brief Sends a POST request with a body.
     * @param path Request path.
     * @param body Request body data (C++ string).
     * @param content_type Body MIME type.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Post(const std::string& path, const std::string& body, const std::string& content_type,
                int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

    /**
     * @brief Sends a POST request with HTTP headers and a body.
     * @param path Request path.
     * @param headers HTTP headers.
     * @param body Request body data (C++ string).
     * @param content_type Body MIME type.
     * @param time_out_sec Timeout duration (in seconds), default is
     * SSGX_HTTP_CLIENT_TIMEOUT.
     * @return Request result.
     */
    Result Post(const std::string& path, const TypeHeaders& headers, const std::string& body,
                const std::string& content_type, int time_out_sec = SSGX_HTTP_CLIENT_TIMEOUT);

  private:
    [[nodiscard]] Result SendRequest(const std::string& method, const std::string& path, const TypeHeaders& headers,
                                     const TypeParams& params, const std::string& body, const std::string& content_type,
                                     int time_out_sec) const;

    int port_ = -1;         ///< Server port number, default is -1.
    std::string host_;      ///< Server hostname.
    std::string server_ca_; ///< Server CA certificate path.
};

} // namespace http_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_HTTP_CLIENT_H
