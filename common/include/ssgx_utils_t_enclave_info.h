#ifndef SSGXLIB_SSGX_UTILS_ENCLAVEINFO_H_
#define SSGXLIB_SSGX_UTILS_ENCLAVEINFO_H_

#include <mutex>

#include "sgx_eid.h"

namespace ssgx {
namespace utils_t {

/**
 * @brief Used to store some information related to the running enclave. (the eid corresponding to the running enclave)
 */
class EnclaveInfo {
  public:
    /**
     * @brief Singleton Pattern
     * @return Global EnclaveInfo object
     */
    static EnclaveInfo& GetCurrentEnclave();

    /**
     * @brief Set eid
     * @param[in] eid The eid corresponding to the running enclave
     */
    void SetEnclaveEid(sgx_enclave_id_t eid);

    /**
     * @brief Get eid
     * @return The eid corresponding to the running enclave
     */
    sgx_enclave_id_t GetEnclaveEid();

  private:
    EnclaveInfo();
    ~EnclaveInfo();

    EnclaveInfo(const EnclaveInfo&) = delete;
    EnclaveInfo& operator=(const EnclaveInfo&) = delete;

    sgx_enclave_id_t enclave_eid_ = 0;
    std::mutex mutex_;
};

} // namespace utils_t
} // namespace ssgx

#endif // SSGXLIB_SSGX_UTILS_ENCLAVEINFO_H_
