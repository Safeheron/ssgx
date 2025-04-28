#include <memory>
#include <cstring>

#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

using namespace ssgx::utils_t;


struct CustomStruct {
    int a;
    double b;
    char c;
};

TEST(MemoryTest, MallocOutsideStruct) {
    CustomStruct* p = static_cast<CustomStruct*>(MallocOutside(sizeof(CustomStruct)));
    ASSERT_NE(p, nullptr);

    p->a = 42;
    p->b = 3.14;
    p->c = 'x';

    ASSERT_EQ(p->a, 42);
    ASSERT_EQ(p->b, 3.14);
    ASSERT_EQ(p->c, 'x');

    FreeOutside(p, sizeof(CustomStruct));
}

TEST(MemoryTest, CallocOutsideLargeArray) {
    const size_t count = 1024;
    int* arr = static_cast<int*>(CallocOutside(count, sizeof(int)));
    ASSERT_NE(arr, nullptr);

    for (size_t i = 0; i < count; ++i) {
        ASSERT_EQ(arr[i], 0);
    }

    FreeOutside(arr, count * sizeof(int));
}

TEST(MemoryTest, MallocOutsideWriteReadback) {
    const char* msg = "Hello Enclave";
    size_t len = strlen(msg);
    char* p = static_cast<char*>(MallocOutside(len + 1));
    ASSERT_NE(p, nullptr);

    memcpy(p, msg, len);
    p[len] = '\0';

    ASSERT_STR_EQ(p, msg);
    FreeOutside(p, len + 1);
}

TEST(MemoryTest, MultipleAllocationsAndFrees) {
    std::vector<void*> blocks;
    for (int i = 0; i < 10; ++i) {
        void* ptr = MallocOutside(64);
        ASSERT_NE(ptr, nullptr);
        blocks.push_back(ptr);
    }

    for (void* ptr : blocks) {
        FreeOutside(ptr, 64);
    }
}

TEST(MemoryTest, FreeAfterFreeIsUndefinedBehavior) {
    void* p = MallocOutside(32);
    ASSERT_NE(p, nullptr);
    FreeOutside(p, 32);

    // Uncomment if running with AddressSanitizer or death tests
    // EXPECT_DEATH(FreeOutside(p, 32), ".*");
    SUCCEED(); // Placeholder for documentation: double free is undefined
}

TEST(MemoryTest, FreeGarbagePointerIsUndefined) {
    void* bogus = reinterpret_cast<void*>(0x12345678);
    // EXPECT_DEATH(FreeOutside(bogus, 32), ".*");
    SUCCEED(); // Placeholder: invalid free is undefined
}

TEST(MemoryTest, MallocOutsideOneByte) {
    char* p = static_cast<char*>(MallocOutside(1));
    ASSERT_NE(p, nullptr);
    *p = 'A';
    ASSERT_EQ(*p, 'A');
    FreeOutside(p, 1);
}

TEST(MemoryTest, CallocOutsideOneByte) {
    char* p = static_cast<char*>(CallocOutside(1, 1));
    ASSERT_NE(p, nullptr);
    ASSERT_EQ(*p, 0);
    FreeOutside(p, 1);
}

TEST(MemoryTest, MallocOutsideLargeAllocation) {
    constexpr size_t alloc_size = 1 << 20; // 1 MB
    char* p = static_cast<char*>(MallocOutside(alloc_size));
    ASSERT_NE(p, nullptr);
    memset(p, 0xAB, alloc_size);  // fill with pattern
    for (size_t i = 0; i < alloc_size; i += 4096) {
        ASSERT_EQ(static_cast<unsigned char>(p[i]), 0xAB);  // sparse check
    }
    FreeOutside(p, alloc_size);
}

TEST(MemoryTest, CallocOutsideLargeZeroed) {
    constexpr size_t alloc_size = 1024 * 64;
    uint8_t* p = static_cast<uint8_t*>(CallocOutside(alloc_size, sizeof(uint8_t)));
    ASSERT_NE(p, nullptr);
    for (size_t i = 0; i < alloc_size; i += 2048) {
        ASSERT_EQ(p[i], 0);
    }
    FreeOutside(p, alloc_size);
}

TEST(MemoryTest, MallocOutsideStructArray) {
    constexpr size_t count = 128;
    CustomStruct* arr = static_cast<CustomStruct*>(MallocOutside(sizeof(CustomStruct) * count));
    ASSERT_NE(arr, nullptr);
    for (size_t i = 0; i < count; ++i) {
        arr[i] = {static_cast<int>(i), i * 1.1, static_cast<char>('a' + i % 26)};
    }
    for (size_t i = 0; i < count; ++i) {
        ASSERT_EQ(arr[i].a, static_cast<int>(i));
        ASSERT_NEAR(arr[i].b, i * 1.1, 1e-6);
        ASSERT_EQ(arr[i].c, static_cast<char>('a' + i % 26));
    }
    FreeOutside(arr, sizeof(CustomStruct) * count);
}

TEST(MemoryTest, AllocFreeInLoop) {
    for (int i = 1; i <= 100; ++i) {
        void* p = MallocOutside(i);
        ASSERT_NE(p, nullptr);
        memset(p, i, i);
        FreeOutside(p, i);
    }
}

TEST(MemoryTest, InterleavedMallocCalloc) {
    void* a = MallocOutside(32);
    void* b = CallocOutside(1, 64);
    void* c = MallocOutside(16);

    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);

    FreeOutside(a, 32);
    FreeOutside(b, 64);
    FreeOutside(c, 16);
}

TEST(MemoryTest, MallocOutsideWritePatternAndCheck) {
    constexpr size_t len = 256;
    uint8_t* p = static_cast<uint8_t*>(MallocOutside(len));
    ASSERT_NE(p, nullptr);
    for (size_t i = 0; i < len; ++i) {
        p[i] = static_cast<uint8_t>(i);
    }
    for (size_t i = 0; i < len; ++i) {
        ASSERT_EQ(p[i], static_cast<uint8_t>(i));
    }
    FreeOutside(p, len);
}

TEST(StrndupOutsideTest, ValidInput) {
    const char* src = "hello world";
    size_t len = strlen(src);
    char* dup = StrndupOutside(src, len);

    ASSERT_NE(dup, nullptr);
    ASSERT_STR_EQ(dup, src);

    FreeOutside(dup, len + 1);
}

TEST(StrndupOutsideTest, NullInput) {
    ASSERT_EQ(StrndupOutside(nullptr, 5), nullptr);
}

TEST(StrndupOutsideTest, ZeroLength) {
    const char* src = "";
    char* dup = StrndupOutside(src, 0);
    ASSERT_NE(dup, nullptr);
    ASSERT_STR_EQ(dup, "");
    FreeOutside(dup, strlen(src));
}

TEST(StrndupOutsideTest, LengthMismatchFails) {
    const char* src = "abcdef";
    ASSERT_EQ(StrndupOutside(src, 10), nullptr);  // strnlen fails here
}

TEST(MemdupOutsideTest, ValidCopy) {
    uint8_t src[] = {1, 2, 3, 4, 5};
    void* dup = MemdupOutside(src, sizeof(src));

    ASSERT_NE(dup, nullptr);
    ASSERT_EQ(std::memcmp(dup, src, sizeof(src)), 0);

    FreeOutside(dup, sizeof src);
}

TEST(MemdupOutsideTest, NullSourceReturnsNull) {
    ASSERT_EQ(MemdupOutside(nullptr, 10), nullptr);
}

TEST(MemdupOutsideTest, ZeroSizeReturnsNull) {
    uint8_t dummy[1] = {0};
    ASSERT_EQ(MemdupOutside(dummy, 0), nullptr);
}