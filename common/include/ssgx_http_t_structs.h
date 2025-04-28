#ifndef SAFEHERON_SGX_TRUSTED_HTTP_STRUCT_H
#define SAFEHERON_SGX_TRUSTED_HTTP_STRUCT_H
#include <any>
#include <map>
#include <string>
#include <unordered_map>
namespace ssgx {
namespace http_t {
// Enum for HTTP Status Codes, used to represent various response states
enum class HttpStatusCode {
    // Informational responses
    Continue100 = 100,          ///< 100: Continue with request
    SwitchingProtocol101 = 101, ///< 101: Switching Protocols
    Processing102 = 102,        ///< 102: Processing request
    EarlyHints103 = 103,        ///< 103: Early hints for request

    // Successful responses
    OK200 = 200,                          ///< 200: Request was successful
    Created201 = 201,                     ///< 201: Resource was created
    Accepted202 = 202,                    ///< 202: Request has been accepted
    NonAuthoritativeInformation203 = 203, ///< 203: Non-authoritative information
    NoContent204 = 204,                   ///< 204: No content in the response
    ResetContent205 = 205,                ///< 205: Reset content
    PartialContent206 = 206,              ///< 206: Partial content
    MultiStatus207 = 207,                 ///< 207: Multi-status response
    AlreadyReported208 = 208,             ///< 208: Already reported
    IMUsed226 = 226,                      ///< 226: IM used

    // Redirection responses
    MultipleChoices300 = 300,   ///< 300: Multiple choices available
    MovedPermanently301 = 301,  ///< 301: Moved permanently
    Found302 = 302,             ///< 302: Found
    SeeOther303 = 303,          ///< 303: See other location
    NotModified304 = 304,       ///< 304: Not modified
    UseProxy305 = 305,          ///< 305: Use proxy
    Unused306 = 306,            ///< 306: Reserved (no longer in use)
    TemporaryRedirect307 = 307, ///< 307: Temporary redirect
    PermanentRedirect308 = 308, ///< 308: Permanent redirect

    // Client error responses
    BadRequest400 = 400,                  ///< 400: Bad request
    Unauthorized401 = 401,                ///< 401: Unauthorized
    PaymentRequired402 = 402,             ///< 402: Payment required
    Forbidden403 = 403,                   ///< 403: Forbidden
    NotFound404 = 404,                    ///< 404: Not found
    MethodNotAllowed405 = 405,            ///< 405: Method not allowed
    NotAcceptable406 = 406,               ///< 406: Not acceptable
    ProxyAuthenticationRequired407 = 407, ///< 407: Proxy authentication required
    RequestTimeout408 = 408,              ///< 408: Request timeout
    Conflict409 = 409,                    ///< 409: Conflict
    Gone410 = 410,                        ///< 410: Gone
    LengthRequired411 = 411,              ///< 411: Length required
    PreconditionFailed412 = 412,          ///< 412: Precondition failed
    PayloadTooLarge413 = 413,             ///< 413: Payload too large
    UriTooLong414 = 414,                  ///< 414: URI too long
    UnsupportedMediaType415 = 415,        ///< 415: Unsupported media type
    RangeNotSatisfiable416 = 416,         ///< 416: Range not satisfiable
    ExpectationFailed417 = 417,           ///< 417: Expectation failed
    ImATeapot418 = 418,                   ///< 418: I'm a teapot (a joke error code)
    MisdirectedRequest421 = 421,          ///< 421: Misdirected request
    UnprocessableContent422 = 422,        ///< 422: Unprocessable content
    Locked423 = 423,                      ///< 423: Locked
    FailedDependency424 = 424,            ///< 424: Failed dependency
    TooEarly425 = 425,                    ///< 425: Too early
    UpgradeRequired426 = 426,             ///< 426: Upgrade required
    PreconditionRequired428 = 428,        ///< 428: Precondition required
    TooManyRequests429 = 429,             ///< 429: Too many requests
    RequestHeaderFieldsTooLarge431 = 431, ///< 431: Request header fields too large
    UnavailableForLegalReasons451 = 451,  ///< 451: Unavailable for legal reasons

    // Server error responses
    InternalServerError500 = 500,           ///< 500: Internal server error
    NotImplemented501 = 501,                ///< 501: Not implemented
    BadGateway502 = 502,                    ///< 502: Bad gateway
    ServiceUnavailable503 = 503,            ///< 503: Service unavailable
    GatewayTimeout504 = 504,                ///< 504: Gateway timeout
    HttpVersionNotSupported505 = 505,       ///< 505: HTTP version not supported
    VariantAlsoNegotiates506 = 506,         ///< 506: Variant also negotiates
    InsufficientStorage507 = 507,           ///< 507: Insufficient storage
    LoopDetected508 = 508,                  ///< 508: Loop detected
    NotExtended510 = 510,                   ///< 510: Not extended
    NetworkAuthenticationRequired511 = 511, ///< 511: Network authentication required
};

/**
 * @brief Comparator for case-insensitive string comparison.
 */
struct ci {
    /**
     * @brief This function compares two strings lexicographically, ignoring case.
     *
     * @param s1 The first string to compare.
     * @param s2 The second string to compare.
     * @return True if s1 is less than s2 lexicographically (ignoring case).
     */
    bool operator()(const std::string& s1, const std::string& s2) const {
        return std::lexicographical_compare(
            s1.begin(), s1.end(), s2.begin(), s2.end(),
            [](unsigned char c1, unsigned char c2) { return ::tolower(c1) < ::tolower(c2); });
    }
};

using TypeHeaders = std::map<std::string, std::string, ci>;
using TypeParams = std::map<std::string, std::string>;

/**
 * @brief HTTP request class to represent an incoming HTTP request
 */
class Request {
  public:
    /**
     * @brief Set the HTTP method (GET, POST, etc.) from a char buffer.
     *
     * This method takes a character buffer and sets the HTTP method.
     * The buffer length is also provided to ensure proper parsing.
     *
     * @param buf The character buffer containing the HTTP method.
     * @param buf_len The length of the buffer.
     */
    void SetMethod(const char* buf, size_t buf_len);

    /**
     * @brief Set the HTTP method (GET, POST, etc.) from a string.
     *
     * This method takes a string and sets the HTTP method.
     *
     * @param method The HTTP method as a string (e.g., "GET", "POST").
     */
    void SetMethod(const std::string& method);

    /**
     * @brief Get the HTTP method.
     *
     * This method returns the HTTP method of the request.
     *
     * @return The HTTP method as a string.
     */
    std::string Method() const;

    /**
     * @brief Set the path of the request (e.g., "/api/v1/xxx").
     *
     * This method sets the path for the request, which is the part of the URL
     * following the domain (e.g., "/api/v1/xxx").
     *
     * @param buf The character buffer containing the path.
     * @param buf_len The length of the buffer.
     */
    void SetPath(const char* buf, size_t buf_len);

    /**
     * @brief Set the path of the request (e.g., "/api/v1/xxx").
     *
     * This method sets the path for the request as a string.
     *
     * @param path The path as a string (e.g., "/api/v1/xxx").
     */
    void SetPath(const std::string& path);

    /**
     * @brief Get the path of the request.
     *
     * This method returns the path of the HTTP request.
     *
     * @return The path of the request as a string.
     */
    std::string Path() const;

    /**
     * @brief Set the headers for the request.
     *
     * This method sets the headers for the HTTP request.
     *
     * @param headers The headers to set as a map.
     */
    void SetHeaders(const TypeHeaders& headers);

    /**
     * @brief Get the headers for the request.
     *
     * This method returns the headers of the HTTP request.
     *
     * @return A const reference to the headers of the request.
     */
    const TypeHeaders& Headers() const;

    /**
     * @brief Set a specific header for the request.
     *
     * This method sets a specific header for the HTTP request.
     *
     * @param key The key (header name) to set.
     * @param val The value for the header.
     */
    void SetHeader(const std::string& key, const std::string& val);

    /**
     * @brief Set a specific header for the request with an integer value.
     *
     * This method sets a specific header for the HTTP request with an integer value.
     *
     * @param key The key (header name) to set.
     * @param val The integer value for the header.
     */
    void SetHeader(const std::string& key, int64_t val);

    /**
     * @brief Check if the request has a specific header.
     *
     * This method checks whether a specific header exists in the request.
     *
     * @param key The key (header name) to check for.
     * @return True if the header exists, otherwise false.
     */
    bool HasHeader(const std::string& key) const;

    /**
     * @brief Get the value of a specific header (with a default value).
     *
     * This method retrieves the value of a specific header, and returns a default value if the header is not found.
     *
     * @param key The key (header name) to retrieve.
     * @param def The default value to return if the header is not found.
     * @return The value of the header, or the default value if not found.
     */
    std::string GetHeaderValue(const std::string& key, const char* def = "") const;

    /**
     * @brief Get the value of a specific header as uint64 (with a default value).
     *
     * This method retrieves the value of a header as an unsigned 64-bit integer,
     * and returns a default value if the header is not found.
     *
     * @param key The key (header name) to retrieve.
     * @param def The default value to return if the header is not found.
     * @return The value of the header as uint64, or the default value.
     */
    uint64_t GetHeaderValueUint64(const std::string& key, uint64_t def = 0) const;

    /**
     * @brief Set the parameters for the request.
     *
     * This method sets the query parameters for the HTTP request.
     *
     * @param params The parameters to set as a map.
     */
    void SetParams(const TypeParams& params);

    /**
     * @brief Get the parameters for the request.
     *
     * This method returns the query parameters of the HTTP request.
     *
     * @return A const reference to the parameters of the request.
     */
    const TypeParams& Params() const;

    /**
     * @brief Set a specific parameter for the request.
     *
     * This method sets a specific query parameter for the HTTP request.
     *
     * @param key The key (parameter name) to set.
     * @param val The value for the parameter.
     */
    void SetParam(const std::string& key, const std::string& val);

    /**
     * @brief Set a specific parameter for the request with an integer value.
     *
     * This method sets a specific query parameter for the HTTP request with an integer value.
     *
     * @param key The key (parameter name) to set.
     * @param val The integer value for the parameter.
     */
    void SetParam(const std::string& key, int64_t val);

    /**
     * @brief Check if the request has a specific parameter.
     *
     * This method checks whether a specific query parameter exists in the request.
     *
     * @param key The key (parameter name) to check for.
     * @return True if the parameter exists, otherwise false.
     */
    bool HasParam(const std::string& key) const;

    /**
     * @brief Get the value of a specific parameter (with a default value).
     *
     * This method retrieves the value of a specific parameter, and returns a
     * default value if the parameter is not found.
     *
     * @param key The key (parameter name) to retrieve.
     * @param def The default value to return if the parameter is not found.
     * @return The value of the parameter, or the default value if not found.
     */
    std::string GetParamValue(const std::string& key, const char* def = "") const;

    /**
     * @brief Set the body of the request from a character buffer.
     *
     * This method sets the body of the request using a character buffer and its length.
     *
     * @param buf The character buffer containing the body.
     * @param buf_len The length of the buffer.
     */
    void SetBody(const char* buf, size_t buf_len);

    /**
     * @brief Set the body of the request from a string.
     *
     * This method sets the body of the request using a string.
     *
     * @param body The body of the request as a string.
     */
    void SetBody(const std::string& body);

    /**
     * @brief Get the body of the request.
     *
     * This method returns the body of the HTTP request.
     *
     * @return The body of the request as a string.
     */
    const std::string& Body() const;

    /**
     * @brief Parse the request from a JSON string.
     *
     * This method parses the HTTP request from a JSON string representation.
     *
     * @param json_str The JSON string containing the request.
     * @return True if parsing is successful, otherwise false.
     */
    bool FromJsonStr(const std::string& json_str);

    /**
     * @brief Parse the request from a JSON character buffer.
     *
     * This method parses the HTTP request from a JSON character buffer.
     *
     * @param json_str The character buffer containing the JSON string.
     * @return True if parsing is successful, otherwise false.
     */
    bool FromJsonStr(const char* json_str);

    /**
     * @brief Convert the request to a JSON string.
     *
     * This method converts the HTTP request to its JSON string representation.
     *
     * @param json_str The resulting JSON string.
     * @return True if conversion is successful, otherwise false.
     */
    bool ToJsonStr(std::string& json_str) const;

    /**
     * @brief Get the attribute value associated with the given key.
     *
     * This method retrieves the value of the specified attribute.
     * The caller must ensure the requested type matches the stored type.
     *
     * @tparam T The expected type of the attribute value.
     * @param key The attribute key.
     * @return Reference to the attribute value of type T.
     */
    template <typename T>
    void SetAttribute(const std::string& key, T&& value) {
        data_[key] = std::forward<T>(value);
    }

    /**
     * @brief Get the attribute value associated with the given key.
     *
     * This method retrieves the value of the specified attribute.
     * The caller must ensure the requested type matches the stored type.
     *
     * @tparam T The expected type of the attribute value.
     * @param key The attribute key.
     * @return Reference to the attribute value of type T.
     */
    template <typename T>
    T& GetAttribute(const std::string& key) {
        return GetInternal<T>(key);
    }

    /**
     * @brief Check if an attribute exists.
     *
     * This method checks whether a given key exists in the attribute storage.
     *
     * @param key The attribute key to check.
     * @return True if the key exists, otherwise false.
     */
    template <typename T>
    [[maybe_unused]] [[nodiscard]] bool HasAttribute(const std::string& key) const {
        auto it = data_.find(key);
        return (it != data_.end() && it->second.type() == typeid(T));
    }

  private:
    std::string method_;
    std::string path_; //"/api/v1/xxx"
    TypeParams params_;
    TypeHeaders headers_;
    std::string body_;
    std::unordered_map<std::string, std::any> data_;

    template <typename T>
    T& GetInternal(const std::string& key) {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::runtime_error("Key '" + key + "' not found in Attribute");
        }
        return std::any_cast<T&>(it->second);
    }
};

/**
 * @brief HTTP response class to represent an outgoing HTTP response
 */
class Response {
  public:
    /**
     * @brief Set the HTTP status code for the response.
     *
     * This method sets the HTTP status code (e.g., 200 OK, 404 Not Found) for the response.
     *
     * @param status The HTTP status code to set.
     */
    void SetStatusCode(HttpStatusCode status);

    /**
     * @brief Get the HTTP status code of the response.
     *
     * This method returns the HTTP status code of the response (e.g., 200 OK, 404 Not Found).
     *
     * @return The HTTP status code.
     */
    HttpStatusCode StatusCode() const;

    /**
     * @brief Set the headers for the response.
     *
     * This method sets the headers for the HTTP response.
     *
     * @param headers The headers to set as a map.
     */
    void SetHeaders(const TypeHeaders& headers);

    /**
     * @brief Get the headers of the response.
     *
     * This method returns the headers of the HTTP response.
     *
     * @return A const reference to the headers of the response.
     */
    const TypeHeaders& Headers() const;

    /**
     * @brief Set a specific header for the response.
     *
     * This method sets a specific header for the HTTP response.
     *
     * @param key The key (header name) to set.
     * @param val The value for the header.
     */
    void SetHeader(const std::string& key, const std::string& val);

    /**
     * @brief Set a specific header for the response with an integer value.
     *
     * This method sets a specific header for the HTTP response with an integer value.
     *
     * @param key The key (header name) to set.
     * @param val The integer value for the header.
     */
    void SetHeader(const std::string& key, int64_t val);

    /**
     * @brief Check if the response has a specific header.
     *
     * This method checks whether a specific header exists in the response.
     *
     * @param key The key (header name) to check for.
     * @return True if the header exists, otherwise false.
     */
    bool HasHeader(const std::string& key) const;

    /**
     * @brief Get the value of a specific header in the response (with a default value).
     *
     * This method retrieves the value of a specific header, and returns a default value if the header is not found.
     *
     * @param key The key (header name) to retrieve.
     * @param def The default value to return if the header is not found.
     * @return The value of the header, or the default value if not found.
     */
    std::string GetHeaderValue(const std::string& key, const char* def = "") const;

    /**
     * @brief Get the value of a specific header as uint64 (with a default value).
     *
     * This method retrieves the value of a specific header as an unsigned 64-bit integer,
     * and returns a default value if the header is not found.
     *
     * @param key The key (header name) to retrieve.
     * @param def The default value to return if the header is not found.
     * @return The value of the header as uint64, or the default value.
     */
    uint64_t GetHeaderValueUint64(const std::string& key, uint64_t def = 0) const;

    /**
     * @brief Set the body of the response from a character buffer.
     *
     * This method sets the body of the response using a character buffer and its length.
     *
     * @param buf The character buffer containing the body.
     * @param buf_len The length of the buffer.
     */
    void SetBody(const char* buf, size_t buf_len);

    /**
     * @brief Set the body of the response from a string.
     *
     * This method sets the body of the response using a string.
     *
     * @param body The body of the response as a string.
     */
    void SetBody(const std::string& body);

    /**
     * @brief Get the body of the response.
     *
     * This method returns the body of the HTTP response.
     *
     * @return The body of the response as a string.
     */
    const std::string& Body() const;

    /**
     * @brief Parse the response from a JSON string, without including the body.
     *
     * This method parses the response from a JSON string representation, ignoring the body.
     *
     * @param json_str The JSON string containing the response.
     * @return True if parsing is successful, otherwise false.
     */
    bool FromJsonStrWithoutBody(const std::string& json_str);

    /**
     * @brief Convert the response to a JSON string, without including the body.
     *
     * This method converts the response to its JSON string representation, excluding the body.
     *
     * @param json_str The resulting JSON string.
     * @return True if conversion is successful, otherwise false.
     */
    bool ToJsonStrWithoutBody(std::string& json_str) const;

    /**
     * @brief Set the full response with a string body, content type, and status.
     *
     * This method sets the full response, including the body, content type, and status.
     *
     * @param s The response body as a string.
     * @param n The length of the body string.
     * @param content_type The content type (e.g., "application/json").
     * @param status The HTTP status code for the response.
     */
    void SetResp(const char* s, size_t n, const std::string& content_type, HttpStatusCode status);

    /**
     * @brief Set the full response with a string body, content type, and status.
     *
     * This method sets the full response, including the body, content type, and status.
     *
     * @param s The response body as a string.
     * @param content_type The content type (e.g., "application/json").
     * @param status The HTTP status code for the response.
     */
    void SetResp(const std::string& s, const std::string& content_type, HttpStatusCode status);

    /**
     * @brief Set a redirection response with the specified URL and status.
     *
     * This method sets a redirection response, including the URL and the status code.
     *
     * @param url The URL to redirect to.
     * @param status_code The HTTP status code for the redirection (default is 302 Found).
     */
    void SetRespRedirect(const std::string& url, HttpStatusCode status_code = HttpStatusCode::Found302);

    /**
     * @brief Set a "404 Not Found" response.
     *
     * This method sets a 404 Not Found response with a default body.
     */
    void SetResp404NotFound();

    /**
     * @brief Set a "500 Internal Server Error" response.
     *
     * This method sets a 500 Internal Server Error response with a default body.
     */
    void SetResp500InternalServerError();

    /**
     * @brief Set a "403 Forbidden" response.
     *
     * This method sets a 403 Forbidden response with a default body.
     */
    void SetResp403Forbidden();

    /**
     * @brief Set a "204 No Content" response.
     *
     * This method sets a 204 No Content response, indicating a successful request with no body.
     */
    void SetResp204NoContent();

  private:
    HttpStatusCode status_code_ = HttpStatusCode::InternalServerError500;
    TypeHeaders headers_;
    std::string body_;
};

} // namespace http_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_HTTP_STRUCT_H
