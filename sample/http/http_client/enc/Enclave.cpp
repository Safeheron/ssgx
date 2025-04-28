#include <ctime>
#include <pthread.h>
#include <vector>

#include "nlohmann/json.hpp"

#include "ssgx_http_t.h"
#include "ssgx_log_t.h"
#include "ssgx_utils_t.h"

#include "Enclave_t.h"

using JSON = nlohmann::json;

using ssgx::http_t::Request;
using ssgx::http_t::Response;
using ssgx::http_t::Client;
using ssgx::http_t::Result;
using ssgx::http_t::TypeHeaders;
using ssgx::http_t::TypeParams;
using ssgx::utils_t::DateTime;

int ecall_run_http_client() {
    // Server Details
    std::string host = "http://0.0.0.0"; // or "localhost" if running locally and accessible
    int port = 83;

    // Create a client
    Client client(host, port);

    // --- Test Cases ---

    // 1. GET /hello (no name)
    SSGX_LOG(INFO) << "--- GET /hello (no name) ---";
    Result result_hello = client.Get("/hello");
    if (result_hello) {
        SSGX_LOG(INFO) << "Status: " << static_cast<int>(result_hello->StatusCode());
        SSGX_LOG(INFO) << "Body: " << result_hello->Body();
    } else {
        SSGX_LOG(INFO) << "Error: " << result_hello.error().message();
    }

    // 2. GET /hello?name=TestUser
    SSGX_LOG(INFO) << "\n--- GET /hello?name=TestUser ---";
    TypeParams params;
    params["name"] = "TestUser";
    Result result_hello_name = client.Get("/hello", params);
    if (result_hello_name) {
        SSGX_LOG(INFO) << "Status: " << static_cast<int>(result_hello_name->StatusCode());
        SSGX_LOG(INFO) << "Body: " << result_hello_name->Body();
    } else {
        SSGX_LOG(INFO) << "Error: " << result_hello_name.error().message();
    }

    // 3. GET /time
    SSGX_LOG(INFO) << "\n--- GET /time ---";
    Result result_time = client.Get("/time");
    if (result_time) {
        SSGX_LOG(INFO) << "Status: " << static_cast<int>(result_time->StatusCode());
        SSGX_LOG(INFO) << "Body: " << result_time->Body();
    } else {
        SSGX_LOG(INFO) << "Error: " << result_time.error().message();
    }

    // 4. POST /echo with JSON body
    SSGX_LOG(INFO) << "\n--- POST /echo with JSON body ---";
    std::string echo_body = R"({"message": "This is a test"})"; // Raw std:string literal
    Result result_echo = client.Post("/echo", echo_body, "application/json");
    if (result_echo) {
        SSGX_LOG(INFO) << "Status: " << static_cast<int>(result_echo->StatusCode());
        SSGX_LOG(INFO) << "Body: " << result_echo->Body();
    } else {
        SSGX_LOG(INFO) << "Error: " << result_echo.error().message();
    }

    // 5. POST /add with JSON body
    SSGX_LOG(INFO) << "\n--- POST /add with JSON body ---";
    std::string add_body = R"({"a": 10, "b": 20})";
    Result result_add = client.Post("/add", add_body, "application/json");
    if (result_add) {
        SSGX_LOG(INFO) << "Status: " << static_cast<int>(result_add->StatusCode());
        SSGX_LOG(INFO) << "Body: " << result_add->Body();
    } else {
        SSGX_LOG(INFO) << "Error: " << result_add.error().message();
    }

    // 6. POST /add with JSON body and custom headers
    SSGX_LOG(INFO) << "\n--- POST /add with JSON body and custom headers ---";
    TypeHeaders add_headers;
    add_headers["Custom-Header"] = "CustomValue";
    add_headers["Content-Type"] = "application/json"; //redundant, but testing headers
    std::string add_body2 = R"({"a": 5, "b": 15})";
    Result result_add_headers = client.Post("/add", add_headers, add_body2, "application/json");
    if (result_add_headers) {
        SSGX_LOG(INFO) << "Status: " << static_cast<int>(result_add_headers->StatusCode());
        SSGX_LOG(INFO) << "Body: " << result_add_headers->Body();
    } else {
        SSGX_LOG(INFO) << "Error: " << result_add_headers.error().message();
    }

    SSGX_LOG(INFO) << "\n--- End of Tests ---";

    return 0;
}
