#ifndef SSGX_HTTP_T_COMMON_HTTP_UTILS_H
#define SSGX_HTTP_T_COMMON_HTTP_UTILS_H

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
    return s.find_first_of("\r\n") != std::string::npos;
}

} // namespace detail
} // namespace http_t
} // namespace ssgx

#endif // SSGX_HTTP_T_COMMON_HTTP_UTILS_H