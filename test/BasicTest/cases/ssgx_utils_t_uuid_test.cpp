#include <cstring>
#include <stdexcept>

#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

using namespace ssgx::utils_t;
// Simple linear search function to check the uniqueness of a UUID
bool is_unique(const std::vector<std::string>& uuids, const std::string& uuid) {
    for (const auto& existing_uuid : uuids) {
        if (strcmp(existing_uuid.c_str(), uuid.c_str()) == 0) {
            return false;
        }
    }
    return true;
}

TEST(UUIDTestSuite, TestUUIDFormat) {
    UUIDGenerator generator;
    std::string uuid = generator.NewUUID4();
    // Check if the UUID length is 36 characters (including 4 hyphens)
    ASSERT_EQ(uuid.length(), 36);
    // Check if the 9th, 14th, 19th, and 24th characters are hyphens
    ASSERT_EQ(uuid[8], '-');
    ASSERT_EQ(uuid[13], '-');
    ASSERT_EQ(uuid[18], '-');
    ASSERT_EQ(uuid[23], '-');
    // Check if the 14th character is '4', indicating the version number
    ASSERT_EQ(uuid[14], '4');
    // Check if the 19th character is '8', '9', 'a', or 'b', indicating the variant
    ASSERT_TRUE(uuid[19] == '8' || uuid[19] == '9' || uuid[19] == 'a' || uuid[19] == 'b');
}

TEST(UUIDTestSuite, TestUUIDUniqueness) {
    UUIDGenerator generator;
    std::vector<std::string> uuids;
    const int num_uuids = 1000;
    for (int i = 0; i < num_uuids; ++i) {
        std::string uuid = generator.NewUUID4();
        // Check if the generated UUID is unique
        ASSERT_TRUE(is_unique(uuids, uuid));
        uuids.push_back(uuid);
    }
}
