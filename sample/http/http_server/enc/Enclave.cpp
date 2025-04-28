#include <ctime>
#include <pthread.h>
#include <vector>

#include "nlohmann/json.hpp"

#include "ssgx_http_t.h"
#include "ssgx_log_t.h"
#include "ssgx_utils_t.h"

#include "CustomFilters.h"
#include "Enclave_t.h"

using JSON = nlohmann::json;

using ssgx::http_t::Request;
using ssgx::http_t::Response;
using ssgx::http_t::Server;
using ssgx::http_t::HttpStatusCode;
using ssgx::utils_t::DateTime;

void HandleHello(Request& request, Response& response) {
    std::string info;
    request.ToJsonStr(info);
    std::string name = request.GetParamValue("name"); // Extract `name` from query parameters

    if (name.empty()) {
        name = "World"; // Default value if `name` is not provided
    }

    std::string reply;
    reply = "Hello " + name + "!";

    response.SetResp(reply, "text/plain", HttpStatusCode::OK200);
}

void HandleGetTime(Request& request, Response& response) {
    auto now = DateTime::Now();
    std::string time_str = now.ToFormatTime();
    JSON json;
    json["server_time"] = time_str;
    response.SetResp(json.dump(), "application/json", HttpStatusCode::OK200);
}

void HandleEcho(Request& request, Response& response) {
    JSON json;
    json["method"] = request.Method();
    json["path"] = request.Path();
    json["body"] = request.Body();
    json["headers"] = JSON::object();

    for (const auto& header : request.Headers()) {
        json["headers"][header.first] = header.second;
    }

    std::string log_str;
    request.ToJsonStr(log_str);

    response.SetResp(json.dump(4), "application/json", HttpStatusCode::OK200);
}

void HandleCalculateAddition(Request& request, Response& response) {
    if (request.GetHeaderValue("Content-Type", "") != "application/json") {
        response.SetResp("Invalid content type", "text/plain", HttpStatusCode::BadRequest400);
        return;
    }

    try {
        JSON json = nlohmann::json::parse(request.Body());

        // Validate that "a" and "b" exist and are integers
        if (!json.contains("a") || !json.contains("b") || !json["a"].is_number_integer() ||
            !json["b"].is_number_integer()) {
            response.SetResp("Invalid JSON format. Expected {\"a\": int, \"b\": int}", "text/plain", HttpStatusCode::BadRequest400);
            return;
        }

        int a = json["a"].get<int>();
        int b = json["b"].get<int>();
        int sum = a + b;

        JSON result;
        result["a"] = a;
        result["b"] = b;
        result["sum"] = sum;

        response.SetResp(result.dump(), "application/json", HttpStatusCode::OK200);
    } catch (const std::exception& e) {
        response.SetResp("JSON parsing error", "text/plain", HttpStatusCode::BadRequest400);
    }
}

int ecall_run_http_server() {
    Server srv;
    std::string url = "http://0.0.0.0:83";
    srv.Listen(url);
    srv.Get("/hello", [](auto& req, auto& resp) { HandleHello(req, resp); });
    srv.Post("/echo", [](auto& req, auto& resp) { HandleEcho(req, resp); });
    srv.Get("/time", [](auto& req, auto& resp) { HandleGetTime(req, resp); });
    srv.Post("/add", [](auto& req, auto& resp) { HandleCalculateAddition(req, resp); });
    srv.AddFilter(std::make_unique<TimingFilter>());
    srv.AddFilter(std::make_unique<LoggingFilter>());
    srv.Start();
    while (1) {
        ssgx::utils_t::Sleep(10);
    }

    return 0;
}
