#include <string>

#include "ssgx_http_t_server.h"
#include "ssgx_http_t_structs.h"
#include "ssgx_http_t_t.h"
#include "ssgx_log_t.h"
#include "ssgx_utils_t.h"

#include "../../../common/auxilliary.h"
#include "../../share/ObjectRegistry.h"

using namespace ssgx::utils_t;
using ssgx::http_t::Request;
using ssgx::http_t::Response;
using ssgx::http_t::Server;


extern "C" int ssgx_ecall_http_on_message(const char* server_id, const char* req_method, const char* req_path,
                                          const char* req_params_json, const char* req_headers_json,
                                          const uint8_t* req_body, size_t req_body_size,
                                          char** resp_status_headers_json_ptr, uint8_t** resp_body_ptr,
                                          size_t* resp_body_size_ptr) {
    // initial output pointers
    *resp_status_headers_json_ptr = nullptr;
    *resp_body_ptr = nullptr;
    *resp_body_size_ptr = 0;

    // check parameters
    if (!IsNonEmptyString(server_id))
        return -1;
    if (!IsNonEmptyString(req_method))
        return -2;
    if (!IsNonEmptyString(req_path))
        return -3;

    // parse and set headers and params to request
    Request http_request;
    if (req_headers_json && !http_request.FromJsonStr(req_headers_json))
        return -4;
    if (req_params_json && !http_request.FromJsonStr(req_params_json))
        return -5;

    // find the server object based on server_id
    Server* http_server = ssgx::internal::ObjectRegistry<std::string, Server>::Query(server_id);
    if (!http_server)
        return -6;

    // prepare request
    http_request.SetMethod(req_method);
    http_request.SetPath(req_path);
    if (req_body != nullptr && req_body_size > 0) {
        http_request.SetBody(std::string(reinterpret_cast<const char*>(req_body), req_body_size));
    }
    try {
        Response http_response;

        // process http_request
        ServerProcessBridge(http_server, req_method, req_path, http_request, http_response);

        // extract status, headers and body from http_response
        // 1. extract status and headers
        std::string resp_status_headers_json;
        http_response.ToJsonStrWithoutBody(resp_status_headers_json);

        *resp_status_headers_json_ptr = ssgx::utils_t::StrndupOutside(resp_status_headers_json.c_str(), resp_status_headers_json.length());
        if (*resp_status_headers_json_ptr == nullptr) {
            return -7;
        }

        // 2. extract body
        if (!http_response.Body().empty()) {
            size_t body_size = http_response.Body().size();
            auto* outside_buff = static_cast<uint8_t*>(MallocOutside(body_size));
            if (!outside_buff) {
                FreeOutside(*resp_status_headers_json_ptr, strlen(*resp_status_headers_json_ptr));
                *resp_status_headers_json_ptr = nullptr;
                return -8;
            }

            memcpy(outside_buff, http_response.Body().data(), body_size);

            // return body and size
            *resp_body_ptr = outside_buff;
            *resp_body_size_ptr = body_size;
        }

        return 0;
    } catch (const std::exception&) {
        if (*resp_status_headers_json_ptr) {
            FreeOutside(*resp_status_headers_json_ptr, strlen(*resp_status_headers_json_ptr));
            *resp_status_headers_json_ptr = nullptr;
        }
        return -9;
    }
}