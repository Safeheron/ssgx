#ifndef SSGXLIB_CUSTOMFILTERS_H
#define SSGXLIB_CUSTOMFILTERS_H

#include "ssgx_http_t.h"
#include "ssgx_log_t.h"
#include "ssgx_utils_t.h"

/**
 * @brief Logs request and response details.
 */
#include <string>

/**
 * @brief Serializes a Request object into raw HTTP format, including query parameters.
 * @param request The request object to serialize.
 * @return A string containing the serialized HTTP request.
 */
std::string SerializeHttpRequest(const ssgx::http_t::Request& request) {
    std::string serialized;

    // Start with the request line (METHOD PATH?QUERY HTTP/1.1)
    serialized += request.Method() + " " + request.Path();

    // Append query parameters if present
    if (!request.Params().empty()) {
        serialized += "?";
        bool first = true;
        for (const auto& [key, value] : request.Params()) {
            if (!first)
                serialized += "&";
            serialized += key + "=" + value;
            first = false;
        }
    }

    serialized += " HTTP/1.1\r\n";

    // Add headers
    for (const auto& [key, value] : request.Headers()) {
        serialized += key + ": " + value + "\r\n";
    }

    // Separate headers and body
    serialized += "\r\n";

    // Add body (if exists)
    if (!request.Body().empty()) {
        serialized += request.Body();
    }

    return serialized;
}

/**
 * @brief Serializes a Response object into raw HTTP format.
 * @param response The response object to serialize.
 * @return A string containing the serialized HTTP response.
 */
std::string SerializeHttpResponse(const ssgx::http_t::Response& response) {
    std::string serialized;

    // Start with the status line (HTTP/1.1 STATUS_CODE STATUS_MESSAGE)
    serialized += "HTTP/1.1 " + std::to_string(static_cast<int>(response.StatusCode())) + " \r\n";

    // Add headers
    for (const auto& [key, value] : response.Headers()) {
        serialized += key + ": " + value + "\r\n";
    }

    // Separate headers and body
    serialized += "\r\n";

    // Add body (if exists)
    if (!response.Body().empty()) {
        serialized += response.Body();
    }

    return serialized;
}

/**
 * @brief Logs complete request and response details in a single log statement.
 */
class LoggingFilter : public ssgx::http_t::Filter {
  public:
    bool Before(ssgx::http_t::Request& request, ssgx::http_t::Response& response) override {
        std::string log_message = "[LoggingFilter] " + request.Method() + " " + request.Path() + "\r\n";
        log_message += SerializeHttpRequest(request);

        SSGX_LOG(INFO) << log_message;
        return true; // Allow request to proceed
    }

    void After(ssgx::http_t::Request& request, ssgx::http_t::Response& response) override {
        std::string log_message = "[LoggingFilter] " + request.Method() + " " + request.Path() + "\n";

        log_message += SerializeHttpResponse(response);

        SSGX_LOG(INFO) << log_message;
    }
};

/**
 * @brief Measures and logs the time taken for request processing.
 */
class TimingFilter : public ssgx::http_t::Filter {
    struct Context{
        int64_t start_time;
    };

  public:
    bool Before(ssgx::http_t::Request& request, ssgx::http_t::Response& response) override {
        Context ctx{};
        ctx.start_time = ssgx::utils_t::PreciseTime::NowInMilliseconds();
        request.SetAttribute("TimingFilterCtx", ctx);
        return true;
    }

    void After(ssgx::http_t::Request& request, ssgx::http_t::Response& response) override {
        if(request.HasAttribute<Context>("TimingFilterCtx")){
            Context& ctx = request.GetAttribute<Context>("TimingFilterCtx");
            auto end_time = ssgx::utils_t::PreciseTime::NowInMilliseconds();
            auto duration = end_time - ctx.start_time;
            std::string log_message = "[TimingFilter] " + request.Method() + " " + request.Path();
            log_message += "\n  - Processed in " + std::to_string(duration) + " milliseconds";

            SSGX_LOG(INFO) << log_message;
        }
    }
};

#endif // SSGXLIB_CUSTOMFILTERS_H
