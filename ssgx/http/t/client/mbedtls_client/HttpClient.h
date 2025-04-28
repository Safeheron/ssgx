#ifndef _MBEDTLS_CLIENT_HTTP_CLIENT_H_
#define _MBEDTLS_CLIENT_HTTP_CLIENT_H_

#include <list>
#include <string>

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/net_sockets.h"

#include "../httpparser/response.h"
#include "HttpUrl.h"

namespace ssgx {
namespace http_t {
namespace mbedtls_client {

/**
 * @brief error codes definition
 *
 */
enum class HttpError {
    OK = 0,                   /* Success */
    InvalidParam = 1,         /* Invalid parameters, maybe some parameters are nullptr */
    InvalidCall = 2,          /* Invalid calling stack */
    InvalidUrl = 3,           /* The specified url is invalid. */
    SetSeedFailed = 4,        /* Set mbedtls drgb seed failed */
    CACertsWrong = 5,         /* mbedtls library parse ca certitifcate chain string failed */
    SSLConfigFailed = 6,      /* mbedtls config failed */
    SSLSetupFailed = 7,       /* SSL setup failed in mbedtls */
    SSLHostNameWrong = 8,     /* Set host name failed in mbedtls */
    HandShakeFailed = 9,      /* Handshake failed in mbedtls */
    VerifyCACertsFailed = 10, /* Verify host ssl certificate failed in mbedtls */
    ConnectFailed = 11,       /* mbedtls connect failed */
    WriteFaild = 12,          /* mbedtls write data failed */
    ReadFailed = 13,          /* mbedtls read data failed. */
    MallocFailed = 14,        /* Failed to malloc memory */
};

class HttpResult {
    HttpError code_;
    std::string msg_;

  public:
    HttpResult() : code_(HttpError::OK), msg_("Success") {
    }
    HttpResult(HttpError code, const std::string& msg = "") : code_(code), msg_(msg) {
    }
    HttpResult(const HttpResult& other) = default;
    HttpResult& operator=(const HttpResult& other) = default;
    HttpError Code() const {
        return code_;
    }
    const std::string& Message() const {
        return msg_;
    }
};

/**
 * @brief The context for http client
 *
 */
class HttpContext {
  public:
    HttpContext() {};
    ~HttpContext() {
        if (is_https_) {
            mbedtls_ssl_close_notify(&ssl_);
        }

        mbedtls_net_free_ocall(&ssl_fd_);

        if (is_https_) {
            mbedtls_x509_crt_free(&ca_cert_);
            mbedtls_ssl_free(&ssl_);
            mbedtls_ssl_config_free(&conf_);
            mbedtls_ctr_drbg_free(&ctr_drbg_);
            mbedtls_entropy_free(&entropy_);
        }
    }
    // class HttpClient can access private members
    friend class HttpClient;

  private:
    bool is_https_ = false;
    int time_out_ = 10000; // in milliseconds
    mbedtls_net_context ssl_fd_;
    mbedtls_entropy_context entropy_;
    mbedtls_ctr_drbg_context ctr_drbg_;
    mbedtls_ssl_context ssl_;
    mbedtls_ssl_config conf_;
    mbedtls_x509_crt ca_cert_;
};

class HttpClient {
  public:
    explicit HttpClient(const std::string& host);
    explicit HttpClient(const std::string& host, int port);
    explicit HttpClient(const std::string& host, int port, const std::string& server_ca);
    virtual ~HttpClient() = default;

  public:
    HttpResult Get(const std::string& path, const std::list<std::string>& headers, const std::list<std::string>& params,
                   int time_out_sec, httpparser::Response& response);
    HttpResult Post(const std::string& path, const std::list<std::string>& headers, const std::string& body,
                    int time_out_sec, httpparser::Response& response);

  private:
    HttpResult ExecuteRequest(const std::string& method, const std::string& path, const std::list<std::string>& headers,
                              const std::list<std::string>& params, const std::string& body, int time_out_sec,
                              httpparser::Response& response);
    std::string InitUrl(const std::string& path, const std::list<std::string>& params);
    std::string InitRequest(const std::string& method, const HttpUrl& url, const std::list<std::string>& headers,
                            const std::string& body = "");
    HttpResult InitContext(const HttpUrl& url, HttpContext& ctx);
    HttpResult DoRequest(const HttpUrl& url, HttpContext& ctx, const std::string& request, httpparser::Response& response);
    int http_write(HttpContext& ctx, const char* buffer, size_t len);
    int http_read(HttpContext& ctx, httpparser::Response& resp);

  private:
    int port_ = -1;
    std::string host_;
    std::string server_ca_;
};

} // namespace mbedtls_client
} // namespace http_t
} // namespace ssgx

#endif //_MBEDTLS_CLIENT_HTTP_CLIENT_H_