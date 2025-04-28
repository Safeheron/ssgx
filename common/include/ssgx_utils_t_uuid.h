#ifndef SAFEHERON_SGX_TRUSTED_UUID_H
#define SAFEHERON_SGX_TRUSTED_UUID_H

#include <mutex>
#include <string>

namespace ssgx {
namespace utils_t {

/**
 * @class UUIDGenerator
 * @brief A thread-safe UUID Generator that generates only UUID V4 (random-based).
 *
 * This class provides a method to generate Universally Unique Identifiers (UUIDs) conforming to version 4 of the UUID
 * standard. It is thread-safe and ensures unique IDs in concurrent environments.
 */
class UUIDGenerator {
  public:
    /**
     * @brief Default constructor for the UUIDGenerator class.
     *
     * Initializes the generator. No explicit setup is needed since the initialization flag ensures proper state during
     * the first use.
     */
    UUIDGenerator() = default;
    /**
     * @brief Generates a new UUID V4 (random-based).
     *
     * This method is thread-safe and ensures that a unique UUID is generated even when accessed concurrently by
     * multiple threads.
     *
     * @return A string representation of the UUID V4 in the standard format, e.g.,
     * "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx", where 'x' is a hexadecimal digit and 'y' is one of [8, 9, A, B].
     *
     * @exception std::runtime_error Failed in UUID initialization!
     */
    std::string NewUUID4();

  private:
    static std::mutex mutex_;
};

} // namespace utils_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_UUID_H
