#ifndef SSGX_HTTP_U_CALLBACK_MANAGER_H
#define SSGX_HTTP_U_CALLBACK_MANAGER_H

#include <cstddef>
#include <mutex>
#include <sgx_error.h>
#include <sgx_urts.h>
#include <unordered_map>

namespace ssgx {

/**
 * @namespace ssgx::http_u
 * @brief This module is the untrusted part of http. Its purpose is to initialize the http_t module. It needs to be called after creating the enclave and before calling the http_t function.
 */
namespace http_u {

/** @brief Typedef for HTTP message callback */
using HttpOnMessageCallback = sgx_status_t (*)(sgx_enclave_id_t eid, int* retval, const char* server_id,
                                               const char* req_method, const char* req_path,
                                               const char* req_params_json, const char* req_headers_json,
                                               const uint8_t* req_body, size_t req_body_size,
                                               char** resp_status_headers_json, uint8_t** resp_body,
                                               size_t* resp_body_size);

/** @brief Typedef for Enclave ID registration callback */
using RegisterEidCallback = sgx_status_t (*)(sgx_enclave_id_t eid, sgx_enclave_id_t enclave_eid);


/**
 * @brief Http module callback function registration and management
 */
class HttpCallbackManager {
  public:
    /**
     * @brief Singleton Pattern
     * @return Global HttpCallbackManager object
     */
    static HttpCallbackManager& GetInstance() {
        static HttpCallbackManager instance;
        return instance;
    }
    /**
     * @brief Http module callback function registration
     * @param[in] eid enclave id
     * @param[in] register_cb The function name of the eid registration callback function
     * @param[in] http_cb The function name of http message callback function
     * @return Return true if successful; otherwise, return false
     */
    bool RegisterCallbacks(sgx_enclave_id_t eid, RegisterEidCallback register_cb, HttpOnMessageCallback http_cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        http_callback_map_[eid] = http_cb;
        sgx_status_t status = register_cb(eid, eid);
        return status == SGX_SUCCESS;
    }

    /**
     * @brief Check if the enclave has registered a callback function
     * @param[in] eid enclave id
     * @return Return true if it registered; otherwise, return false
     */
    bool HasHttpCallback(sgx_enclave_id_t eid) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return http_callback_map_.count(eid) > 0;
    }

    /**
     * @brief Get the http message callback function of the corresponding enclave
     * @param[in] eid enclave id
     * @return Return the callback function if it registered; otherwise, return nullptr
     */
    HttpOnMessageCallback GetHttpCallback(sgx_enclave_id_t eid) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = http_callback_map_.find(eid);
        if (it != http_callback_map_.end()) {
            return it->second;
        }
        return nullptr;
    }

  public:
    HttpCallbackManager(const HttpCallbackManager&) = delete;
    HttpCallbackManager& operator=(const HttpCallbackManager&) = delete;

    HttpCallbackManager(HttpCallbackManager&&) = delete;
    HttpCallbackManager& operator=(HttpCallbackManager&&) = delete;

  private:
    HttpCallbackManager() = default;
    ~HttpCallbackManager() = default;

    mutable std::mutex mutex_;
    std::unordered_map<sgx_enclave_id_t, HttpOnMessageCallback> http_callback_map_;
};

} // namespace http_u
} // namespace ssgx

#endif // SSGX_HTTP_U_CALLBACK_MANAGER_H
