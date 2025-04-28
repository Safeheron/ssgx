#include <stdexcept>

#include "ssgx_http_t_structs.h"

namespace ssgx {
namespace http_t {
namespace detail {

inline bool HasHeader(const TypeHeaders& headers, const std::string& key) {
    return headers.find(key) != headers.end();
}

inline const char* GetHeaderValue(const TypeHeaders& headers, const std::string& key, const char* def) {
    auto it = headers.find(key);
    if (it != headers.end()) {
        return it->second.c_str();
    }
    return def;
}

inline uint64_t GetHeaderValueUint64(const TypeHeaders& headers, const std::string& key, uint64_t def) {
    auto it = headers.find(key);
    if (it != headers.end()) {
        return std::strtoull(it->second.data(), nullptr, 10);
    }
    return def;
}

inline bool HasCRLF(const std::string& s) {
    auto p = s.c_str();
    while (*p) {
        if (*p == '\r' || *p == '\n') {
            return true;
        }
        p++;
    }
    return false;
}

} // namespace detail
} // namespace http_t
} // namespace ssgx