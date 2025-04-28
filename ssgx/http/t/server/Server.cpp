#include "ssgx_http_t_server.h"
#include "ssgx_http_t_structs.h"
#include "ssgx_http_t_t.h"
#include "ssgx_log_t.h"
#include "ssgx_utils_t.h"

#include "../../ssgx/http/share/ObjectRegistry.h"

using ssgx::utils_t::EnclaveInfo;

namespace ssgx {
namespace http_t {

void Server::Listen(const std::string& url) {
    url_ = url;
}

void Server::SetTimeOut(uint64_t timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
}

void Server::SetMaxThreads(uint64_t max_threads) {
    max_threads_ = max_threads;
}

void Server::SetMaxQueued(uint64_t max_queued) {
    max_queued_ = max_queued;
}

/// To start a  server and save it to the registry
bool Server::Start() {
    int ret = 0;
    sgx_status_t status = SGX_SUCCESS;

    if (url_.empty())
        return false;

    sgx_enclave_id_t eid = EnclaveInfo::GetCurrentEnclave().GetEnclaveEid();
    if (eid == 0)
        return false;

    // To start a server's listener outside of enclave
    status = ssgx_ocall_http_create_listener(&ret, eid, url_.c_str(), timeout_seconds_, max_threads_, max_queued_);
    if (status != SGX_SUCCESS || ret != 0) {
        return false;
    }

    // Save this server pointer to a map
    // key: eid + "_" + url
    // value: server object
    server_id_ = std::to_string(eid) + "_" + url_;
    ssgx::internal::ObjectRegistry<std::string, Server>::Register(server_id_, this);

    // Mark as running
    is_stopped_ = false;

    return true;
}

/// Stops the server and removes it from registry (only once)
void Server::Stop() {
    // Prevent duplicate calls
    if (is_stopped_)
        return;

    // Remove server from registry and release it
    if (!server_id_.empty()) {
        ssgx::internal::ObjectRegistry<std::string, Server>::Unregister(server_id_);
        ssgx_ocall_http_release_listener(server_id_.c_str());
    }

    // Mark as stopped
    is_stopped_ = true;
}

/// Destructor: Ensures cleanup if `stop()` was never called
Server::~Server() {
    Stop(); // Now safe from duplicate calls
}

void Server::Get(const std::string& path, const std::function<void(Request&, Response&)>& handler) {
    std::string method = "GET";
    RegisterHandler(method, path, handler);
}
void Server::Post(const std::string& path, const std::function<void(Request&, Response&)>& handler) {
    std::string method = "POST";
    RegisterHandler(method, path, handler);
}

void Server::RegisterHandler(const std::string& method, const std::string& path,
                             const std::function<void(Request&, Response&)>& handler) {
    std::lock_guard<std::mutex> lock(key_handler_map_mutex_);
    // key = "{method}_{path}"
    const std::string key = method + "_" + path;
    key_handler_map_[key] = handler;
}

std::function<void(Request&, Response&)> Server::GetHandler(const std::string& method, const std::string& path) {
    std::lock_guard<std::mutex> lock(key_handler_map_mutex_);
    // key = "{method}_{path}"
    const std::string key = method + "_" + path;
    if (const auto it = key_handler_map_.find(key); it != key_handler_map_.end()) {
        return it->second;
    }
    return nullptr;
}

void Server::AddFilter(std::unique_ptr<Filter>&& filter) {
    filter_chain_.AddFilter(std::move(filter));
}

void Server::Process(const std::string& method, const std::string& path, Request& req, Response& resp) {

    if (const auto func = GetHandler(method, path); !func) {
        resp.SetResp404NotFound();
    } else {
        resp = filter_chain_.Execute(req, func);
    }
}

/**
 * This is a bridge function, it will be called in ssgx_ecall_http_on_message(),
 * so that we can call Server::Process() function based on server id.
 *
 */
void ServerProcessBridge(Server* srv, const std::string& method, const std::string& path, Request& req,
                         Response& resp) {
    if (srv) {
        srv->Process(method, path, req, resp);
    }
}

} // namespace http_t
} // namespace ssgx