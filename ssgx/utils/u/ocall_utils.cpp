#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <thread> // std::this_thread::sleep_for

extern "C" {

int ssgx_ocall_printf(const char* str) {
    return printf("%s", str);
}

void ssgx_ocall_time(uint64_t* now) {
    *now = (uint64_t)time(NULL);
}

void ssgx_ocall_malloc(size_t size, uint8_t** ret) {
    /**
     * Refer to: https://en.cppreference.com/w/c/memory/malloc
     * If size is zero, the behavior of malloc is implementation-defined.
     * For example, a null pointer may be returned.
     * Alternatively, a non-null pointer may be returned; but such a pointer should not be dereferenced, and should be
     * passed to free to avoid memory leaks.
     */
    if (size == 0) {
        *ret = NULL;
    } else {
        *ret = (uint8_t*)malloc(size);
        if (*ret) {
            memset(*ret, 0, size);
        }
    }
}

void ssgx_ocall_calloc(size_t num, size_t size, uint8_t** ret) {
    /**
     * Refer to: https://en.cppreference.com/w/c/memory/calloc
     * If size is zero, the behavior is implementation defined (null pointer may be returned, or some non-null pointer
     * may be returned that may not be used to access storage).
     */
    if (size == 0 || num == 0) {
        *ret = NULL;
    } else {
        *ret = (uint8_t*)calloc(num, size);
    }
}

void ssgx_ocall_free(uint8_t* ptr_out_side) {
    free(ptr_out_side);
    ptr_out_side = NULL;
}

void ssgx_ocall_time_in_milliseconds(uint64_t* now) {
    auto t_now = std::chrono::system_clock::now();
    auto duration = t_now.time_since_epoch();
    long long millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    *now = millis;
}

void ssgx_ocall_time_in_nanoseconds(uint64_t* now) {
    auto t_now = std::chrono::high_resolution_clock::now();
    auto duration = t_now.time_since_epoch();
    long long nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    *now = nanoseconds;
}

void ssgx_ocall_sleep(uint32_t seconds) {
    // std::chrono::seconds	std::chrono::duration</* int35 */>
    // Note: each of the predefined duration types up to hours covers a range of at least ±292 years.
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

} // extern "C"