#include <ctime>
#include <pthread.h>
#include <vector>

#include "nlohmann/json.hpp"

#include "ssgx_http_t.h"
#include "ssgx_log_t.h"
#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

#include "Enclave_t.h"

using JSON = nlohmann::json;

using ssgx::http_t::Client;
using ssgx::http_t::TypeHeaders;
using ssgx::http_t::TypeParams;
using ssgx::http_t::Request;
using ssgx::http_t::Response;
using ssgx::http_t::Server;
using ssgx::utils_t::DateTime;

void GetHandler(Request& request, Response& response) {
    // set header
    for (const auto& item : request.Headers()) {
        response.SetHeader(item.first, item.second);
    }
    ssgx::utils_t::Sleep(2);
    // set body
    JSON json;
    json["method"] = request.Method();
    json["path"] = request.Path();
    json["params"] = JSON::object();
    for (const auto& item : request.Params()) {
        json["params"][item.first] = item.second;
    }
    std::string reply = json.dump();

    response.SetResp(reply, "application/json", ssgx::http_t::HttpStatusCode::OK200);
}

void PostHandler(Request& request, Response& response) {
    // set header
    for (const auto& item : request.Headers()) {
        response.SetHeader(item.first, item.second);
    }
    // set body
    JSON json;
    json["method"] = request.Method();
    json["path"] = request.Path();
    json["params"] = JSON::object();
    json["request_body"] = JSON::parse(request.Body());
    for (const auto& item : request.Params()) {
        json["params"][item.first] = item.second;
    }
    std::string reply = json.dump();

    response.SetResp(reply, "application/json", ssgx::http_t::HttpStatusCode::OK200);
}
class TimingFilter : public ssgx::http_t::Filter {
  public:
    struct Context{
        int64_t start_time;
    };

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

            // Inject attribute data into the response body for use in test case verification.
            std::string body = response.Body();
            JSON body_json = JSON::parse(body);
            body_json["processing_time"] = duration;
            response.SetBody(body_json.dump());
        }
    }
};

TEST(HttpServerTestSuite, Get_Request) {
    std::string path = "/sample1?p1=a&p2=b";
    Client client = Client("http://0.0.0.0:83");
    TypeHeaders headers = {{"Content-Type", "text/plain"}, {"test_key1", "test_val1"}, {"test_key2", "test_val2"}};

    ssgx::utils_t::Printf("\n\n");

    ssgx::http_t::Result result = client.Get(path, headers, 5);
    if (!result) { // Failed!
        ssgx::utils_t::Printf("Error code: %d\n", result.error().code());
        ssgx::utils_t::Printf("Error message: %s\n\n", result.error().message().c_str());
        ASSERT_TRUE(false);
    }

    std::string result_headers_json;
    result->ToJsonStrWithoutBody(result_headers_json);
    ssgx::utils_t::Printf("result header =%s\n", result_headers_json.c_str());
    ssgx::utils_t::Printf("result body=%s\n", result->Body().c_str());
    JSON body_json = JSON::parse(result->Body());
    ASSERT_EQ(result->StatusCode(), ssgx::http_t::HttpStatusCode::OK200);
    ASSERT_EQ("/sample1", body_json["path"].get<std::string>());
    ASSERT_EQ("GET", body_json["method"].get<std::string>());
    ASSERT_EQ("test_val1", result->GetHeaderValue("test_key1"));
    ASSERT_EQ("test_val2", result->GetHeaderValue("test_key2"));
    ASSERT_EQ("a", body_json["params"]["p1"].get<std::string>());
    ASSERT_EQ("b", body_json["params"]["p2"].get<std::string>());
    // Check the time consumed by the http call.
    ASSERT_GE(body_json["processing_time"].get<int64_t>(), 2000);
    ssgx::utils_t::Printf("Request1 Successful\n\n");
}

TEST(HttpServerTestSuite, Post_Request) {
    std::string path = "/sample2?p1=a&p2=b";
    Client client = Client("http://0.0.0.0:83");
    TypeHeaders headers = {{"test_key1", "test_val1"}, {"test_key2", "test_val2"}};
    std::string body = "{\"body_params1\":\"test1\",\"body_params_array\":{\"k1\":\"1\",\"k2\":\"2\"}}";

    ssgx::utils_t::Printf("\n\n");

    ssgx::http_t::Result result = client.Post(path, headers, body, "application/json", 5);
    if (!result) { // Failed!
        ssgx::utils_t::Printf("Error code: %d\n", result.error().code());
        ssgx::utils_t::Printf("Error message: %s\n\n", result.error().message().c_str());
        ASSERT_TRUE(false);
    }

    std::string result_headers_json;
    result->ToJsonStrWithoutBody(result_headers_json);
    ssgx::utils_t::Printf("result header =%s\n", result_headers_json.c_str());
    ssgx::utils_t::Printf("result body=%s\n", result->Body().c_str());
    JSON body_json = JSON::parse(result->Body());
    ASSERT_EQ(result->StatusCode(), ssgx::http_t::HttpStatusCode::OK200);
    ASSERT_EQ("/sample2", body_json["path"].get<std::string>());
    ASSERT_EQ("POST", body_json["method"].get<std::string>());
    ASSERT_EQ("test_val1", result->GetHeaderValue("test_key1"));
    ASSERT_EQ("test_val2", result->GetHeaderValue("test_key2"));
    ASSERT_EQ("a", body_json["params"]["p1"].get<std::string>());
    ASSERT_EQ("b", body_json["params"]["p2"].get<std::string>());
    ASSERT_EQ("test1", body_json["request_body"]["body_params1"].get<std::string>());
    ASSERT_EQ("1", body_json["request_body"]["body_params_array"]["k1"].get<std::string>());
    ASSERT_EQ("2", body_json["request_body"]["body_params_array"]["k2"].get<std::string>());
    ASSERT_GE(body_json["processing_time"].get<int64_t>(), 0);
    ssgx::utils_t::Printf("Request2 Successful\n");
}

int ecall_run_server(int alive_time_sec) {
    Server srv;
    std::string url = "http://0.0.0.0:83";
    srv.Listen(url);
    srv.Get("/sample1", [](auto& req, auto& resp) { GetHandler(req, resp); });
    srv.Post("/sample2", [](auto& req, auto& resp) { PostHandler(req, resp); });
    srv.AddFilter(std::make_unique<TimingFilter>());

    if (!srv.Start()) {
        return -1;
    }

    ssgx::utils_t::Sleep(alive_time_sec);
    return 0;
}

int ecall_run_client() {
    return ssgx::testframework_t::TestManager::RunAllSuites();
}