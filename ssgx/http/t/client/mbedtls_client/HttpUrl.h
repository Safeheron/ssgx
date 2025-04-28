/*************************************************
 * File name : HttpUrl.h
 * Introduce : Header file for http(s) url parser class.
 *
 * Create: 2021-6-10 by yyf
 *
 *************************************************/

#ifndef _HTTP_URL_H_
#define _HTTP_URL_H_

#include <string>

namespace ssgx {
namespace http_t {
namespace mbedtls_client {

/**
 * @brief A class for parsing http(s) url
 *
 */
class HttpUrl {
  private:
    struct Url {
        Url() : integerPort(0) {
        }

        std::string scheme;
        std::string username;
        std::string password;
        std::string hostname;
        std::string port;
        std::string path;
        std::string query;
        std::string fragment;
        uint16_t integerPort;
    } url;

  public:
    HttpUrl();
    explicit HttpUrl(const std::string& url);
    virtual ~HttpUrl();

  public:
    bool parse(const std::string& str);
    bool isValid() const;
    std::string scheme() const;
    std::string username() const;
    std::string password() const;
    std::string hostname() const;
    std::string port() const;
    std::string Path() const;
    std::string query() const;
    std::string fragment() const;
    uint16_t httpPort() const;

  private:
    bool isUnreserved(char ch) const;
    void parse_(const std::string& str);

  private:
    bool valid;
};

} // namespace mbedtls_client
} // namespace http_t
} // namespace ssgx
#endif // _HTTP_URL_H_
