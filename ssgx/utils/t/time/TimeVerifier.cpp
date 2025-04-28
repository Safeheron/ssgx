#include <cstdint>
#include <mutex>

#include "TimeVerifier.h"

namespace ssgx {
namespace utils_t {

TimeVerifier::TimeVerifier() : past_(0) {
}

bool TimeVerifier::Verify(uint64_t now) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (now < past_) {
        return false;
    }
    past_ = now;
    return true;
}

} // namespace utils_t
} // namespace ssgx