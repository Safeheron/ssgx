#ifndef SSGXLIB_HTTP_U_SERVER_REQUESTHANDLER_H
#define SSGXLIB_HTTP_U_SERVER_REQUESTHANDLER_H

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

using namespace Poco::Net;

namespace ssgx {
namespace http_u {

class RequestHandler : public HTTPRequestHandler {
  private:
    uint64_t sgx_eid_;
    std::string server_id_;

  public:
    explicit RequestHandler(uint64_t sgx_eid, const std::string& server_id) : sgx_eid_(sgx_eid), server_id_(server_id) {
    }

    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory {
  private:
    uint64_t sgx_eid_;
    std::string server_id_;

  public:
    explicit RequestHandlerFactory(uint64_t sgx_eid, const std::string& server_id)
        : sgx_eid_(sgx_eid), server_id_(server_id) {
    }

    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) override {
        return new RequestHandler(sgx_eid_, server_id_);
    }
};

} // namespace http_u
} // namespace ssgx

#endif // SSGXLIB_HTTP_U_SERVER_REQUESTHANDLER_H
