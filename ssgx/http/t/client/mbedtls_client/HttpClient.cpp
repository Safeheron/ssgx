#include "HttpClient.h"

#include "mbedtls/error.h"

#include "sgx_trts.h"

#include "../httpparser/httpresponseparser.h"
#include "HttpUrl.h"

#define DATA_BUF_SIZE 4 * 1024

using namespace httpparser;

namespace ssgx {
namespace http_t {
namespace mbedtls_client {

HttpClient::HttpClient(const std::string& host) : HttpClient(host, -1, "") {
}

HttpClient::HttpClient(const std::string& host, int port) : HttpClient(host, port, "") {
}

HttpClient::HttpClient(const std::string& host, int port, const std::string& server_ca) {
    host_ = host;
    port_ = port;
    server_ca_ = server_ca;
    std::transform(host_.begin(), host_.end(), host_.begin(), ::tolower);
}

HttpResult HttpClient::Get(const std::string& path, const std::list<std::string>& headers,
                           const std::list<std::string>& params, int time_out_sec, httpparser::Response& response) {
    return ExecuteRequest("GET", path, headers, params, "", time_out_sec, response);
}

HttpResult HttpClient::Post(const std::string& path, const std::list<std::string>& headers, const std::string& body,
                            int time_out_sec, httpparser::Response& response) {
    return ExecuteRequest("POST", path, headers, {}, body, time_out_sec, response);
}

HttpResult HttpClient::ExecuteRequest(const std::string& method, const std::string& path,
                                      const std::list<std::string>& headers, const std::list<std::string>& params,
                                      const std::string& body, int time_out_sec, httpparser::Response& response) {
    HttpUrl url;

    // Construct http url object
    const std::string url_str = InitUrl(path, params);
    if (!url.parse(url_str)) {
        return {HttpError::InvalidUrl, "Invalid url: " + url_str};
    }

    // Only support HTTP and HTTPS
    if (url.scheme() != "http" && url.scheme() != "https") {
        return {HttpError::InvalidUrl, "Url scheme is not either http nor https: " + url_str};
    }

    // Initialize connection
    HttpContext ctx;
    ctx.time_out_ = time_out_sec * 1000;
    HttpResult ret = InitContext(url, ctx);
    if (ret.Code() != HttpError::OK) {
        return ret;
    }

    // Initialize request with headers and body
    const std::string req_str = InitRequest(method, url, headers, body);

    // Send the request
    return DoRequest(url, ctx, req_str, response);
}

std::string HttpClient::InitUrl(const std::string& path, const std::list<std::string>& params) {
    int count = 0;
    std::string url_str;
    std::string params_str;

    // Prepare path with parameters
    std::string path_str = path;
    for (const std::string& param : params) {
        params_str += param;
        params_str += (++count < params.size()) ? "&" : "";
    }
    if (!params_str.empty()) {
        path_str += "?";
        path_str += params_str;
    }

    // no port, such as host_ is a domain
    if (port_ == -1) {
        url_str = host_ + path_str;
    } else {
        url_str = host_ + ":" + std::to_string(port_) + path_str;
    }

    return url_str;
}

std::string HttpClient::InitRequest(const std::string& method, const HttpUrl& url,
                                    const std::list<std::string>& headers, const std::string& body) {
    std::string req;

    // Request line: method + path + version
    req.append(method);
    req.append(" ");
    req.append(url.Path());
    if (!url.query().empty()) {
        req.append("?");
        req.append(url.query());
    }
    req.append(" ");
    req.append("HTTP/1.1\r\n");

    // Host and port
    req.append("Host: ");
    req.append(url.hostname());
    if (!(url.port().empty() || url.port() == "80" || url.port() == "443")) {
        req.append(":");
        req.append(url.port());
    }
    req.append("\r\n");

    // Accept
    req.append("Accept: */*\r\n");

    // Add headers
    bool has_connection_header = false;
    for (const auto& header : headers) {
        if (header.find("Connection:") == 0) {
            has_connection_header = true;
        }
        req.append(header);
        req.append("\r\n");
    }
    if (!has_connection_header) {
        req.append("Connection: Keep-Alive");
        req.append("\r\n");
    }

    // Add Content-Length for POST and PUT only
    if (method == "POST" || method == "PUT") {
        req.append("Content-Length: ");
        req.append(std::to_string(body.length()));
        req.append("\r\n");
    }

    // Header end line
    req.append("");
    req.append("\r\n");

    // body
    if (!body.empty()) {
        req.append(body);
    }

    return req;
}

HttpResult HttpClient::InitContext(const HttpUrl& url, HttpContext& ctx) {
    char err_msg[1024] = {0};

    mbedtls_net_init_ocall(&ctx.ssl_fd_);

    ctx.is_https_ = (url.scheme() == "https");

    if (ctx.is_https_) {
        int ret = 0;
        /*
         * Generate a random number as a nonce for mbedtls random number generation
         * The nonce length must be:
         * 1) at least 24 bytes for a 128-bit strength (maximum achievable strength when using AES-128);
         * 2) at least 48 bytes for a 256-bit strength (maximum achievable strength when using AES-256).
         * Refer to:
         * https://mbed-tls.readthedocs.io/projects/api/en/development/api/file/ctr__drbg_8h/#ctr__drbg_8h_1ad93d675f998550b4478c1fe6f4f34ebc
         */
        uint8_t nonce[48] = {0};
        sgx_status_t status = sgx_read_rand(nonce, sizeof(nonce));
        if (status != SGX_SUCCESS) {
            return {HttpError::SetSeedFailed, "sgx_read_rand() failed! Unable to generate nonce."};
        }

        // Initialize TLS components
        mbedtls_ssl_init(&ctx.ssl_);
        mbedtls_ssl_config_init(&ctx.conf_);
        mbedtls_x509_crt_init(&ctx.ca_cert_);
        mbedtls_ctr_drbg_init(&ctx.ctr_drbg_);
        mbedtls_entropy_init(&ctx.entropy_);

        // Turn prediction resistance on
        mbedtls_ctr_drbg_set_prediction_resistance(&ctx.ctr_drbg_, MBEDTLS_CTR_DRBG_PR_ON);

        // Set DRBG entropy source
        if ((ret = mbedtls_ctr_drbg_seed(&ctx.ctr_drbg_, mbedtls_entropy_func, &ctx.entropy_, nonce, sizeof(nonce))) !=
            0) {
            mbedtls_strerror(ret, err_msg, sizeof(err_msg));
            return {HttpError::SetSeedFailed, "mbedtls_ctr_drbg_seed() filed! Error: " + std::string(err_msg)};
        }

        // Parse and set CA certificates
        // server_ca_ is a base64 string, parameter buflen must be size() + 1
        if (!server_ca_.empty()) {
            if ((ret = mbedtls_x509_crt_parse(&ctx.ca_cert_, reinterpret_cast<const uint8_t*>(server_ca_.c_str()),
                                              server_ca_.size() + 1)) != 0) {
                mbedtls_strerror(ret, err_msg, sizeof(err_msg));
                return {HttpError::CACertsWrong, "mbedtls_x509_crt_parse() filed! Error: " + std::string(err_msg)};
            }
        }

        // Set SSL default parameters
        if ((ret = mbedtls_ssl_config_defaults(&ctx.conf_, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                               MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
            mbedtls_strerror(ret, err_msg, sizeof(err_msg));
            return {HttpError::SSLConfigFailed, "mbedtls_ssl_config_defaults() filed! Error: " + std::string(err_msg)};
        }

        // During handshake, verify server's certificate is required!
        mbedtls_ssl_conf_authmode(&ctx.conf_, MBEDTLS_SSL_VERIFY_REQUIRED);

        // Confiture ca certificates to verify server's certificates
        mbedtls_ssl_conf_ca_chain(&ctx.conf_, &ctx.ca_cert_, nullptr);

        mbedtls_ssl_conf_rng(&ctx.conf_, mbedtls_ctr_drbg_random, &ctx.ctr_drbg_);

        // Check and set time out
        if (ctx.time_out_ <= 0) {
            ctx.time_out_ = 10000;
        }
        mbedtls_ssl_conf_read_timeout(&ctx.conf_, ctx.time_out_);
    }

    return {HttpError::OK, "Success"};
}

HttpResult HttpClient::DoRequest(const HttpUrl& url, HttpContext& ctx, const std::string& request,
                                 httpparser::Response& response) {
    int ret = 0;
    size_t req_len = request.size();
    char err_msg[1024] = {0};
    HttpResult result;

    // Connect to the server
    if ((ret = mbedtls_net_connect_ocall(&ctx.ssl_fd_, url.hostname().c_str(), url.port().c_str(),
                                         MBEDTLS_NET_PROTO_TCP)) != 0) {
        mbedtls_strerror(ret, err_msg, sizeof(err_msg));
        return {HttpError::ConnectFailed, "mbedtls_net_connect_ocall() filed! Error: " + std::string(err_msg)};
    }

    // Do SSL/TLS connect for HTTPS
    if (ctx.is_https_) {
        if ((ret = mbedtls_ssl_setup(&ctx.ssl_, &ctx.conf_)) != 0) {
            mbedtls_strerror(ret, err_msg, sizeof(err_msg));
            return {HttpError::SSLSetupFailed, "mbedtls_ssl_setup() filed! Error: " + std::string(err_msg)};
        }

        if ((ret = mbedtls_ssl_set_hostname(&ctx.ssl_, url.hostname().c_str())) != 0) {
            mbedtls_strerror(ret, err_msg, sizeof(err_msg));
            return {HttpError::SSLHostNameWrong, "mbedtls_ssl_set_hostname() filed! Error: " + std::string(err_msg)};
        }

        mbedtls_ssl_set_bio(&ctx.ssl_, &ctx.ssl_fd_, mbedtls_net_send_ocall, mbedtls_net_recv_ocall,
                            mbedtls_net_recv_timeout_ocall);

        // Set time out for DTLS handshaking
        mbedtls_ssl_conf_handshake_timeout(&ctx.conf_, ctx.time_out_, 2 * ctx.time_out_);

        while ((ret = mbedtls_ssl_handshake(&ctx.ssl_)) != 0) {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
                mbedtls_strerror(ret, err_msg, sizeof(err_msg));
                if (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
                    return {HttpError::VerifyCACertsFailed,
                            "mbedtls_ssl_handshake() filed! Error: " + std::string(err_msg)};
                }
                return {HttpError::HandShakeFailed, "mbedtls_ssl_handshake() filed! Error: " + std::string(err_msg)};
            }
        }
    }

    // To send the entire request data
    if ((ret = http_write(ctx, request.c_str(), req_len)) <= 0) {
        return {HttpError::WriteFaild, "http_write() filed! ret: " + std::to_string(ret)};
    }

    // To read the entire response data
    if ((ret = http_read(ctx, response)) <= 0) {
        return {HttpError::ReadFailed, "http_read() failed! ret: " + std::to_string(ret)};
    }

    return {HttpError::OK, "Success"};
}

// write data in the connection
int HttpClient::http_write(HttpContext& ctx, const char* buffer, size_t len) {
    int ret = 0;
    size_t written_size = 0;
    size_t max_retries = 5;
    size_t retries = 0;

    if (!buffer || len <= 0) {
        return 0;
    }

    while (written_size < len) {
        if (ctx.is_https_)
            ret = mbedtls_ssl_write(&ctx.ssl_, (unsigned char*)&buffer[written_size], len - written_size);
        else
            ret = mbedtls_net_send_ocall(&ctx.ssl_fd_, (unsigned char*)&buffer[written_size], len - written_size);

        if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            if (++retries > max_retries) {
                return -1;
            }
            continue;
        } else if (ret <= 0) {
            return ret;
        }

        written_size += ret;
        retries = 0;
    }

    return written_size;
}

// read data in the connection
int HttpClient::http_read(HttpContext& ctx, httpparser::Response& resp) {
    int ret = 0;
    int read_size = 0;
    uint8_t read_buf[DATA_BUF_SIZE] = {0};
    size_t max_retries = 5;
    size_t retries = 0;
    HttpResponseParser parser;
    HttpResponseParser::ParseResult result;

    /*
     * Read response data from http connection multiple times,
     * until all data are read or meet a error.
     */
    while (true) {
        if (ctx.is_https_) {
            ret = mbedtls_ssl_read(&ctx.ssl_, read_buf, DATA_BUF_SIZE);
        } else {
            ret = mbedtls_net_recv_timeout_ocall(&ctx.ssl_fd_, read_buf, DATA_BUF_SIZE, ctx.time_out_);
        }
        if (ret == MBEDTLS_ERR_SSL_WANT_READ) {
            if (++retries > max_retries) {
                return -1;
            }
            continue;
        } else if (ret < 0) {
            return ret;
        } else if (ret == 0) {
            break;
        }

        read_size += ret;
        retries = 0;

        // Try to parse response content
        // Return immediately if we have got a completed HTTP Response, or encounter an error,
        // so that we don't need to try to read data from connection again.
        // Here, if we don't parse the read data, we don't know we have received all data or not.
        // unless we waste time to do a read operation again until we get a MBEDTLS_ERR_SSL_TIMEOUT error.
        result = parser.parse(resp, reinterpret_cast<const char*>(read_buf),
                                reinterpret_cast<const char*>(read_buf + ret));
        if (HttpResponseParser::ParsingCompleted == result || HttpResponseParser::ParsingError == result) {
            break;
        }
    }

    return read_size;
}

} // namespace mbedtls_client
} // namespace http_t
} // namespace ssgx