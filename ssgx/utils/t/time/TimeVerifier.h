#ifndef SSGXLIB_CTIMEVERIFIER_H
#define SSGXLIB_CTIMEVERIFIER_H
#include <cstdint>
#include <mutex>

namespace ssgx {
namespace utils_t {

class TimeVerifier {
  public:
    TimeVerifier();
    bool Verify(uint64_t now);

  private:
    uint64_t past_;
    std::mutex mutex_;
};

}; // namespace utils_t
}; // namespace ssgx

#endif // SSGXLIB_CTIMEVERIFIER_H
