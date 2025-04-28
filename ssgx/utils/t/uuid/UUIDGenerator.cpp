#include <cstring>
#include <ssgx_utils_t_uuid.h>
#include <stdexcept>

#include "uuid4/uuid4.h"

namespace ssgx {
namespace utils_t {

std::mutex UUIDGenerator::mutex_;

static const int INTERVAL = 128;

static int count = 0;

std::string UUIDGenerator::NewUUID4() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (count == 0) {
        if (uuid4_init() != UUID4_ESUCCESS)
            throw std::runtime_error("Failed in UUID initialization!");
    }

    count = (count + 1) % INTERVAL;

    std::string uuid;
    char buf[UUID4_LEN];
    memset(buf, 0, sizeof buf);
    uuid4_generate(buf);
    // exclude the null terminator
    uuid.assign(buf, UUID4_LEN - 1);

    return uuid;
}

} // namespace utils_t
} // namespace ssgx
