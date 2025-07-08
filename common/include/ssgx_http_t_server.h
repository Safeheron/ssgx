#ifndef SSGXLIB_SSGX_HTTP_T_SERVER_H
#define SSGXLIB_SSGX_HTTP_T_SERVER_H

#include <mutex>
#include <string>
#include <unordered_map>

#include "ssgx_http_t_filter.h"
#include "ssgx_http_t_structs.h"

namespace ssgx {
/**
 * @namespace ssgx::http_t
 * @brief This module implements the functionalities of both server and client within a Trusted Execution Environment
 * (TEE).
 */
namespace http_t {
/**
 * @brief HTTP service class for managing request listening and routing.
 *
 * This class provides functionalities for handling HTTP requests, including listening, registering GET/POST handlers,
 * and supporting custom request processing. It is designed to work within an Intel SGX enclave.
 */
class Server {

  public:
    /**
     * @brief Default constructor.
     *
     * Initializes the HTTP service without setting up a listener. The server can be configured further using setter
     * functions like SetTimeOut() and set_max_connections().
     */
    Server() = default;

    /**
     * @brief Destructor.
     *
     * Cleans up any resources or connections held by the server before destruction.
     */
    ~Server();
    /**
     * @brief Set the timeout duration for requests.
     *
     * This sets the maximum time the server waits for a request before timing out. The time is specified in seconds.
     * (Refer to: https://docs.pocoproject.org/current/Poco.Net.HTTPServerParams.html)
     *
     * timeout_seconds must be greater than 0, and the default value is 60s (Refer to:
     * https://github.com/pocoproject/poco/blob/main/Net/src/HTTPServerParams.cpp#L23).
     *
     * @param timeout_seconds The timeout value in seconds.
     */
    void SetTimeOut(uint64_t timeout_seconds);

    /**
     * @brief Set the maximum number of allowed threads.
     *
     * This sets the maximum number of threads that can be used by the server to handle client requests concurrently. If
     * this limit is reached, new requests may be queued or refused depending on the server configuration.
     *
     * max_threads must be greater than 0, the default value is 4 and the max capacity of threads in the thread pool
     * is 16. (Refer to: https://github.com/pocoproject/poco/blob/main/Foundation/include/Poco/ThreadPool.h#L51)
     *
     * @param max_threads The maximum number of threads allowed.
     */
    void SetMaxThreads(uint64_t max_threads);

    /**
     * @brief Set the maximum number of queued connections.
     *
     * This sets the maximum number of incoming connections that can be queued before being accepted by the server. If
     * this limit is reached, additional incoming connections will be refused until there is space in the queue.
     *
     * Refer to:https://docs.pocoproject.org/current/Poco.Net.TCPServerParams.html#32226
     * max_queued must be greater than 0, and the default value is 64 (Refer to:
     * https://github.com/pocoproject/poco/blob/main/Net/src/TCPServerParams.cpp#L25),
     * If there are already the maximum number of connections in the queue, new connections will be silently discarded.
     *
     * @param max_queued The maximum number of queued connections allowed.
     */
    void SetMaxQueued(uint64_t max_queued);

    /**
     * @brief Start listening on the specified URL.
     *
     * This method initializes the server and begins accepting requests on the provided URL. It sets up the server to
     * handle HTTP requests.
     *
     * @param url The URL to listen on (e.g., "http://localhost:8080").
     */
    void Listen(const std::string& url);
    /**
     * @brief Add a filter to the server's filter chain.
     *
     * Filters allow modifying request/response objects before or after they are processed by the handler. Filters can
     * be used for tasks such as logging, security checks, etc.
     *
     * @param filter An unique pointer to the filter to add.
     */
    void AddFilter(std::unique_ptr<Filter>&& filter);
    /**
     * @brief Start the server to begin handling requests.
     *
     * This function starts the server and begins listening for incoming requests. If the server is already running,
     * this method will return false.
     *
     * @return true if the server successfully started, false otherwise.
     */
    bool Start();

    /**
     * @brief Stop the server.
     *
     * This method stops the server, closing all active connections and halting further request processing.
     */
    void Stop();

    /**
     * @brief Check whether the server should stop.
     *
     * This method returns true if the server has received a stop request or has been instructed to terminate.
     * It can be used inside a run loop or polling mechanism to determine whether the server should gracefully exit.
     *
     * @return true if the server is in a stopped or stopping state, false otherwise.
     *
     * @note Example usage:
     * @code
     * srv.Start();
     * while (!srv.ShouldStop()) {
     *     ssgx::utils_t::Sleep(5);
     * }
     * @endcode
     */
    bool ShouldStop() const;

    /**
     * @brief Register a handler for GET requests.
     *
     * This method registers a handler function that will be invoked whenever a GET request is made to the specified
     * path.
     *
     * @param path The path for which the handler should be registered.
     * @param handler The function to handle GET requests at the specified path.
     */
    void Get(const std::string& path,
             const std::function<void(ssgx::http_t::Request&, ssgx::http_t::Response&)>& handler);

    /**
     * @brief Register a handler for POST requests.
     *
     * This method registers a handler function that will be invoked whenever a POST request is made to the specified
     * path.
     *
     * @param path The path for which the handler should be registered.
     * @param handler The function to handle POST requests at the specified path.
     */
    void Post(const std::string& path,
              const std::function<void(ssgx::http_t::Request&, ssgx::http_t::Response&)>& handler);

  private:
    void RegisterHandler(const std::string& method, const std::string& path,
                         const std::function<void(ssgx::http_t::Request&, ssgx::http_t::Response&)>& handler);

    void Process(const std::string& method, const std::string& path, ssgx::http_t::Request& req,
                 ssgx::http_t::Response& resp);

    std::function<void(ssgx::http_t::Request&, ssgx::http_t::Response&)> GetHandler(const std::string& method,
                                                                                    const std::string& method_path);

    friend void ServerProcessBridge(Server* srv, const std::string& method, const std::string& path,
                                    ssgx::http_t::Request& req, ssgx::http_t::Response& resp);

  private:
    std::string url_;
    std::string server_id_;
    uint64_t timeout_seconds_ = 60;
    uint64_t max_threads_ = 4;
    uint64_t max_queued_ = 64;
    bool is_stopped_ = true;

    // key = "{method}_{path}"
    // map<key, handler>
    std::mutex key_handler_map_mutex_;
    std::unordered_map<std::string, std::function<void(ssgx::http_t::Request&, ssgx::http_t::Response&)>>
        key_handler_map_;

    FilterChain filter_chain_;
};
} // namespace http_t
} // namespace ssgx

#endif // SSGXLIB_SSGX_HTTP_T_SERVER_H
