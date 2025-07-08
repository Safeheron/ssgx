#include "ssgx_http_u.h"

namespace ssgx {
namespace http_u {

std::atomic<bool> SignalFlag::stop_flag_{false};

void StopHttpServers() {
    SignalFlag::SetStopFlag();
}

}
}
