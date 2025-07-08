#ifndef SSGXHTTPSERVER_H
#define SSGXHTTPSERVER_H

#include <atomic>
#include <memory>
#include <string>

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/ThreadPool.h"

namespace ssgx {
namespace http_u {
/**
 * @brief Configuration for the SSGXHttpServer.
 */
struct SSGXHttpServerConfig {
    std::string host_ = "127.0.0.1";
    unsigned short port_ = 8080;
    int min_threads_ = 2;
    int max_threads_ = 16;
    int max_queued_requests_ = 100;
    int timeout_seconds_ = 60;
    uint64_t sgx_eid_;      // extra param
    std::string server_id_; // extra param
};

/**
 * @brief A wrapper for Poco's HTTPServer, designed as a robust, externally-controlled component.
 * This class represents a single, non-restartable server session.
 */
class SSGXHttpServer {
  public:
    // Parameter name is also updated for consistency.
    explicit SSGXHttpServer(const SSGXHttpServerConfig& config);

    ~SSGXHttpServer();

    void Start();
    void Stop();

  private:
    SSGXHttpServer(const SSGXHttpServer&) = delete;
    SSGXHttpServer& operator=(const SSGXHttpServer&) = delete;

    // Private members now use the suffix underscore naming convention.
    Poco::ThreadPool thread_pool_;
    Poco::Net::ServerSocket socket_;
    std::unique_ptr<Poco::Net::HTTPServer> server_;

    std::atomic<bool> running_;
};

} // namespace http_u
} // namespace ssgx

#endif // SSGXHTTPSERVER_H
