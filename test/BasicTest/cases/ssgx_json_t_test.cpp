#include <cstring>

#include "nlohmann/json.hpp"

#include "ssgx_json_t.h"
#include "ssgx_testframework_t.h"

namespace {

TEST(JsonFetchTests, FetchStringNode) {
    nlohmann::json test_json = {{"name", "OpenAI"},
                                {"age", 3},
                                {"active", true},
                                {"tags", {"AI", "ML", "NLP"}},
                                {"info", {{"type", "organization"}, {"founded", 2015}}}};

    std::string value;
    std::string err_msg;
    ASSERT_TRUE(ssgx::json_t::fetch_json_string_node(test_json, "name", value, err_msg));
    ASSERT_STR_EQ(value, "OpenAI");
}

TEST(JsonFetchTests, FetchIntegerNode) {
    nlohmann::json test_json = {{"name", "OpenAI"},
                                {"age", 3},
                                {"active", true},
                                {"tags", {"AI", "ML", "NLP"}},
                                {"info", {{"type", "organization"}, {"founded", 2015}}}};

    int value;
    std::string err_msg;
    ASSERT_TRUE(ssgx::json_t::fetch_json_int_node(test_json, "age", value, err_msg));
    ASSERT_EQ(value, 3);
}

TEST(JsonFetchTests, FetchBooleanNode) {
    nlohmann::json test_json = {{"name", "OpenAI"},
                                {"age", 3},
                                {"active", true},
                                {"tags", {"AI", "ML", "NLP"}},
                                {"info", {{"type", "organization"}, {"founded", 2015}}}};

    bool value;
    std::string err_msg;
    ASSERT_TRUE(ssgx::json_t::fetch_json_bool_node(test_json, "active", value, err_msg));
    ASSERT_TRUE(value);
}

TEST(JsonFetchTests, FetchArrayNode) {
    nlohmann::json test_json = {{"name", "OpenAI"},
                                {"age", 3},
                                {"active", true},
                                {"tags", {"AI", "ML", "NLP"}},
                                {"info", {{"type", "organization"}, {"founded", 2015}}}};

    nlohmann::json value;
    std::string err_msg;
    ASSERT_TRUE(ssgx::json_t::fetch_json_array_node(test_json, "tags", value, err_msg));
    ASSERT_EQ(value.size(), 3);
    ASSERT_STR_EQ(value[0], "AI");
    ASSERT_STR_EQ(value[1], "ML");
    ASSERT_STR_EQ(value[2], "NLP");
}

TEST(JsonFetchTests, FetchObjectNode) {
    nlohmann::json test_json = {{"name", "OpenAI"},
                                {"age", 3},
                                {"active", true},
                                {"tags", {"AI", "ML", "NLP"}},
                                {"info", {{"type", "organization"}, {"founded", 2015}}}};

    nlohmann::json value;
    std::string err_msg;
    ASSERT_TRUE(ssgx::json_t::fetch_json_object_node(test_json, "info", value, err_msg));
    ASSERT_TRUE(value.contains("type"));
    ASSERT_TRUE(value.contains("founded"));
    ASSERT_STR_EQ(value["type"], "organization");
    ASSERT_EQ(value["founded"], 2015);
}

TEST(JsonFetchTests, FetchNonExistentNode) {
    nlohmann::json test_json = {{"name", "OpenAI"},
                                {"age", 3},
                                {"active", true},
                                {"tags", {"AI", "ML", "NLP"}},
                                {"info", {{"type", "organization"}, {"founded", 2015}}}};

    std::string value;
    std::string err_msg;
    ASSERT_FALSE(ssgx::json_t::fetch_json_string_node(test_json, "nonexistent", value, err_msg));
    ASSERT_FALSE(err_msg.empty());
}

TEST(JsonFetchTests, FetchTypeMismatch) {
    nlohmann::json test_json = {{"name", "OpenAI"},
                                {"age", 3},
                                {"active", true},
                                {"tags", {"AI", "ML", "NLP"}},
                                {"info", {{"type", "organization"}, {"founded", 2015}}}};

    int value;
    std::string err_msg;
    ASSERT_FALSE(ssgx::json_t::fetch_json_int_node(test_json, "name", value, err_msg));
    ASSERT_FALSE(err_msg.empty());
}

} // namespace
