#include "SSGXHttpServer.h"

#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Thread.h"
#include "RequestHandler.h"

namespace ssgx {
namespace http_u {

SSGXHttpServer::SSGXHttpServer(const SSGXHttpServerConfig& config)
    : thread_pool_("SSGXHttpServerPool", config.min_threads_, config.max_threads_),
      socket_(Poco::Net::SocketAddress(config.host_, config.port_)), server_(), running_(false) {

    Poco::Net::HTTPServerParams::Ptr p_params = new Poco::Net::HTTPServerParams;
    p_params->setMaxQueued(config.max_queued_requests_);
    p_params->setMaxThreads(config.max_threads_);
    p_params->setTimeout(Poco::Timespan(static_cast<long>(config.timeout_seconds_), 0));

    server_ = std::make_unique<Poco::Net::HTTPServer>(new RequestHandlerFactory(config.sgx_eid_, config.server_id_),
                                                      thread_pool_, socket_, p_params);
}

SSGXHttpServer::~SSGXHttpServer() {
    // As a safety net, ensure stop() is called upon destruction.
    // This is safe because stop() is now idempotent (safe to call multiple times).
    Stop();
}

void SSGXHttpServer::Start() {
    server_->start();
    running_ = true;
}

void SSGXHttpServer::Stop() {
    if (running_) {
        server_->stop();
        thread_pool_.joinAll();
        running_ = false;
    }
}

} // namespace http_u
} // namespace ssgx