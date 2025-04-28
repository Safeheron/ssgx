#include <cstring>
#include <limits>
#include <stdexcept>

#include "ssgx_config_t.h"
#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"
using namespace ssgx::config_t;

TEST(ConfigTestSuite, TestLoadFile) {
    TomlConfig config;
    const std::string data_dir(TEST_DATA_DIR);
    std::string file_name = data_dir + "/BasicTest/test.toml";
    ASSERT_TRUE(config.LoadFile(file_name.c_str()));

    file_name = data_dir + "/BasicTest/does-not-exist-file.toml";
    ASSERT_FALSE(config.LoadFile(file_name.c_str()));
    ASSERT_TRUE(!config.GetLastErrorMsg().empty());
    ssgx::utils_t::Printf("%s\n", config.GetLastErrorMsg().c_str());
}

TEST(ConfigTestSuite, TestInteger) {
    TomlConfig config;
    const std::string data_dir(TEST_DATA_DIR);
    const std::string file_name = data_dir + "/BasicTest/test.toml";
    ASSERT_TRUE(config.LoadFile(file_name.c_str()));

    std::optional<int64_t> value = config.GetInteger("hex1");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, 0xDEADBEEF);

    value = config.GetInteger("oct1");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, 012345670);

    value = config.GetInteger("bin1");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, 0b11010110);

    value = config.GetInteger("dec1");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, 36414884);

    value = config.GetInteger("extreme_number");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, std::numeric_limits<long>::max());

    value = config.GetInteger("negative_extreme_number");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, std::numeric_limits<long>::min());

    value = config.GetInteger("hex_extreme_number");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, 0x7FFFFFFFFFFFFFFF);
}

TEST(ConfigTestSuite, TestString) {
    TomlConfig config;
    const std::string data_dir(TEST_DATA_DIR);
    const std::string file_name = data_dir + "/BasicTest/test.toml";
    ASSERT_TRUE(config.LoadFile(file_name.c_str()));

    std::optional<std::string> value = config.GetString("string1");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, "hello world");

    value = config.GetString("string2");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, "Roses are red\nViolets are blue");

    value = config.GetString("string3");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, "The quick brown fox jumps over the lazy dog.");

    value = config.GetString("string4");
    ASSERT_TRUE(value.has_value());
    ASSERT_EQ(value, "Tom \"Dubs\" Preston-Werner");
}

TEST(ConfigTestSuite, TestIntegerArray) {
    TomlConfig config;
    const std::string data_dir(TEST_DATA_DIR);
    const std::string file_name = data_dir + "/BasicTest/test.toml";
    ASSERT_TRUE(config.LoadFile(file_name.c_str()));

    const std::vector<int64_t> num_array_excepted = {-9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, +5, +6, +7, +8, +9};
    const std::optional<std::vector<int64_t>> values = config.GetIntegerArray("number");
    ASSERT_TRUE(values.has_value());
    ASSERT_EQ(values, num_array_excepted);
}

TEST(ConfigTestSuite, TestStringArray) {
    TomlConfig config;
    const std::string data_dir(TEST_DATA_DIR);
    const std::string file_name = data_dir + "/BasicTest/test.toml";
    ASSERT_TRUE(config.LoadFile(file_name.c_str()));

    const std::vector<std::string> color_array_excepted = {"red", "blue", "yellow", "green"};
    std::optional<std::vector<std::string>> values = config.GetStringArray("colors");
    ASSERT_TRUE(values.has_value());
    ASSERT_EQ(values, color_array_excepted);

    const std::vector<std::string> fruit_array_excepted = {"pear", "grapes", "strawberry"};
    values = config.GetStringArray("fruits", 1, 0);
    ASSERT_TRUE(values.has_value());
    ASSERT_EQ(values, fruit_array_excepted);
}

TEST(ConfigTestSuite, TestMultipleLevelPath) {
    TomlConfig config;
    const std::string data_dir(TEST_DATA_DIR);
    const std::string file_name = data_dir + "/BasicTest/test.toml";
    ASSERT_TRUE(config.LoadFile(file_name.c_str()));

    std::optional<int64_t> value_int = config.GetInteger("clothes", "price", 1, "dress");
    ASSERT_TRUE(value_int.has_value());
    ASSERT_EQ(value_int, 39);

    std::optional<std::string> value_str = config.GetString("fruits", 1, 1, 0);
    ASSERT_TRUE(value_str.has_value());
    ASSERT_EQ(value_str, "watermelon");

    value_str = config.GetString("fruits", 3, "banana");
    ASSERT_TRUE(value_str.has_value());
    ASSERT_EQ(value_str, "yellow");

    value_str = config.GetString("animal", "dog", "name");
    ASSERT_TRUE(value_str.has_value());
    ASSERT_EQ(value_str, "Tina");
}

TEST(ConfigTestSuite, TestEmptyValue) {
    TomlConfig config;
    const std::string data_dir(TEST_DATA_DIR);
    const std::string file_name = data_dir + "/BasicTest/test.toml";
    ASSERT_TRUE(config.LoadFile(file_name.c_str()));

    const std::optional<std::string> empty_str = config.GetString("empty", "empty_string");
    ASSERT_TRUE(empty_str.has_value());
    ASSERT_EQ(empty_str, "");

    const std::optional<std::vector<int64_t>> empty_array = config.GetIntegerArray("empty", "empty_array");
    ASSERT_TRUE(empty_array.has_value());
    ASSERT_EQ(empty_array, std::vector<int64_t>{});
}

TEST(ConfigTestSuite, TestErrorHandle) {
    TomlConfig config;
    const std::string data_dir(TEST_DATA_DIR);
    const std::string file_name = data_dir + "/BasicTest/test.toml";
    ASSERT_TRUE(config.LoadFile(file_name.c_str()));

    std::optional<int64_t> value_int = config.GetInteger("string2");
    ASSERT_FALSE(value_int.has_value());
    // Output the error details message
    //ssgx::utils_t::Printf("Error Message: %s\n", config.GetLastErrorMsg().c_str());

    std::optional<std::string> value_str = config.GetString("books", "harry_potter");
    ASSERT_FALSE(value_str.has_value());
    // Output the error details message
    //ssgx::utils_t::Printf("Error Message: %s\n", config.GetLastErrorMsg().c_str());

    value_str = config.GetString("animal");
    ASSERT_FALSE(value_str.has_value());
    // Output the error details message
    //ssgx::utils_t::Printf("Error Message: %s\n", config.GetLastErrorMsg().c_str());

    value_str = config.GetString("animal", 2, 4, 5, 6);
    ASSERT_FALSE(value_str.has_value());
    // Output the error details message
    //ssgx::utils_t::Printf("Error Message: %s\n", config.GetLastErrorMsg().c_str());

    value_str = config.GetString("clothes", "price", 2, "shirt");
    ASSERT_FALSE(value_str.has_value());
    // Output the error details message
    //ssgx::utils_t::Printf("Error Message: %s\n", config.GetLastErrorMsg().c_str());

    std::optional<std::vector<std::string>> value_array = config.GetStringArray("string3");
    ASSERT_FALSE(value_array.has_value());
    // Output the error details message
    //ssgx::utils_t::Printf("Error Message: %s\n", config.GetLastErrorMsg().c_str());

    value_array = config.GetStringArray("animal", "people");
    ASSERT_FALSE(value_array.has_value());
    // Output the error details message
    //ssgx::utils_t::Printf("Error Message: %s\n", config.GetLastErrorMsg().c_str());
}