#ifndef SSGXLIB_OBJECTREGISTRY_H
#define SSGXLIB_OBJECTREGISTRY_H

#include <vector>
#include <mutex>
#include <unordered_map>

namespace ssgx {
namespace internal {

/**
 * @brief A thread-safe global object registry for storing and retrieving objects using custom keys.
 * @tparam TKey The key type used to identify objects.
 * @tparam T The type of objects being stored.
 */
template <typename TKey, typename T>
class ObjectRegistry {
  public:
    ObjectRegistry() = delete; // Prevent instantiation

    /**
     * @brief Returns a vector containing all keys currently stored in the registry.
     * @return A vector of keys.
     */
    static std::vector<TKey> GetAllKeys() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<TKey> keys;
        keys.reserve(registry_.size());
        for (const auto& entry : registry_) {
            keys.push_back(entry.first);
        }
        return keys;
    }

    /**
     * @brief Queries an object by key.
     * @param key The unique identifier for the object.
     * @return A pointer to the object, or nullptr if not found.
     */
    static T* Query(const TKey& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = registry_.find(key);
        return (it != registry_.end()) ? it->second.first : nullptr;
    }

    /**
     * @brief Checks if an object exists in the registry.
     * @param key The unique identifier for the object.
     * @return True if the object exists, otherwise false.
     */
    static bool Contains(const TKey& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        return registry_.find(key) != registry_.end();
    }

    /**
     * @brief Registers an object using a user-defined key.
     * @param key The unique identifier for the object.
     * @param obj A pointer to the object being registered.
     * @param deleteOnUnregister If true, the object will be automatically deleted upon unregistration.
     * @note If the key already exists, the existing object will be overwritten.
     */
    static void Register(const TKey& key, T* obj, bool deleteOnUnregister = false) {
        if (!obj)
            return; // Prevent inserting nullptr
        std::lock_guard<std::mutex> lock(mutex_);
        registry_[key] = {obj, deleteOnUnregister};
    }

    /**
     * @brief Unregisters (removes) an object from the registry.
     * @param key The unique identifier of the object to be removed.
     * @note If `deleteOnUnregister` was set to `true`, the object will be deleted.
     */
    static void Unregister(const TKey& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = registry_.find(key);
        if (it != registry_.end()) {
            if (it->second.second) { // Delete the object if `deleteOnUnregister == true`
                delete it->second.first;
            }
            registry_.erase(it);
        }
    }

  private:
    static std::unordered_map<TKey, std::pair<T*, bool>> registry_; // Stores pointer & delete flag
    static std::mutex mutex_;                                       // Ensures thread safety
};

// Static member initialization
template <typename TKey, typename T>
std::unordered_map<TKey, std::pair<T*, bool>> ObjectRegistry<TKey, T>::registry_;

template <typename TKey, typename T>
std::mutex ObjectRegistry<TKey, T>::mutex_;

} // namespace internal
} // namespace ssgx

#endif // SSGXLIB_OBJECTREGISTRY_H
