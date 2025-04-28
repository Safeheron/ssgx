#include <cstring>
#include <stdexcept>

#include "ssgx_log_t.h"
#include "ssgx_testframework_t.h"

TEST(LogTestSuite, TestLog) {
    // INFO log with a string
    SSGX_LOG(INFO) << "Hello, " << "World" << "!\n";

    // INFO log with an integer
    SSGX_LOG(INFO) << "The answer is " << 42 << "\n";

    // INFO log with a float
    SSGX_LOG(INFO) << "PI is approximately " << 3.14159 << "\n";

    // INFO log with multiple arguments
    SSGX_LOG(INFO) << "User " << "Alice" << " has ID " << 12345 << " and balance " << 567.89 << "\n";

    // WARN log with a simple message
    SSGX_LOG(WARN) << "This is a warning message\n";

    // WARN log with a dynamic message
    SSGX_LOG(WARN) << "Memory usage exceeded " << 1024 << " MB\n";

    // ERROR log with a string and integer
    SSGX_LOG(ERROR) << "Failed to open file: " << "/path/to/file" << ", error code: " << -1 << "\n";

    // FATAL log with a float
    SSGX_LOG(FATAL) << "Critical temperature reached: " << 85.6 << "°C\n";

    // Combining static and dynamic values
    int userId = 42;
    const char* status = "active";
    SSGX_LOG(INFO) << "User " << userId << " is currently " << status << "\n";

    // Testing with special characters
    SSGX_LOG(WARN) << "Special characters: newline(\\n), tab(\\t), and backslash(\\\\)\n";

    // INFO with very large values
    long long bigValue = 123456789012345LL;
    SSGX_LOG(INFO) << "Big number: " << bigValue << "\n";

    // Multiple placeholders with varying types
    SSGX_LOG(INFO) << "User: " << "John" << ", Age: " << 30 << ", Height: " << 1.75 << ", Status: " << "OK" << "\n";

    // Chaining logs
    SSGX_LOG(INFO) << "Start of chain...\n";
    SSGX_LOG(WARN) << "Middle of chain...\n";
    SSGX_LOG(FATAL) << "End of chain!\n";
}

TEST(LogTestSuite, TestLog_LongMessage) {
    SSGX_LOG(INFO) << "He was an old man who fished alone in a skiff in the Gulf Stream and he had gone eighty-four "
                      "days now without "
                      "taking a fish. In the first forty days a boy had been with him. But after forty days without a "
                      "fish the boy’s "
                      "parents had told him that the old man was now definitely and finally salao, which is the worst "
                      "form of unlucky, "
                      "and the boy had gone at their orders in another boat which caught three good fish the first "
                      "week. It made the boy "
                      "sad to see the old man come in each day with his skiff empty and he always went down to help "
                      "him carry either the "
                      "coiled lines or the gaff and harpoon and the sail that was furled around the mast. The sail was "
                      "patched with flour "
                      "sacks and, furled, it looked like the flag of permanent defeat."
                      "The old man was thin and gaunt with deep wrinkles in the back of his neck. The brown blotches "
                      "of the benevolent "
                      "skin cancer the sun brings from its reflection on the tropic sea were on his cheeks. The "
                      "blotches ran well down "
                      "the sides of his face and his hands had the deep-creased scars from handling heavy fish on the "
                      "cords. But none of "
                      "these scars were fresh. They were as old as erosions in a fishless desert."
                      "Everything about him was old except his eyes and they were the same color as the sea and were "
                      "cheerful and undefeated."
                      "Santiago,“ the boy said to him as they climbed the bank from where the skiff was hauled up. “I "
                      "could go with you again. We’ve made some money.“"
                      "The old man had taught the boy to fish and the boy loved him."
                      "“No,“ the old man said. “You’re with a lucky boat. Stay with them.“"
                      "“But remember how you went eighty-seven days without fish and then we caught big ones every day "
                      "for three weeks.“"
                      "“I remember,“ the old man said. “I know you did not leave me because you doubted.“"
                      "“It was papa made me leave. I am a boy and I must obey him.“"
                      "“I know,“ the old man said. “It is quite normal.“"
                      "“He hasn’t much faith.“"
                      "“No,“ the old man said. “But we have. Haven’t we?“"
                      "“Yes,“ the boy said. “Can I offer you a beer on the Terrace and then we’ll take the stuff home.“"
                      "“Why not?“ the old man said. “Between fishermen.“"
                      "They sat on the Terrace and many of the fishermen made fun of the old man and he was not angry. "
                      "Others, of the older "
                      "fishermen, looked at him and were sad. But they did not show it and they spoke politely about "
                      "the current and the depths "
                      "they had drifted their lines at and the steady good weather and of what they had seen. The "
                      "successful fishermen of that day "
                      "were already in and had butchered their marlin out and carried them laid full length across two "
                      "planks, with two men "
                      "staggering at the end of each plank, to the fish house where they waited for the ice truck to "
                      "carry them to the market "
                      "in Havana. Those who had caught sharks had taken them to the shark factory on the other side of "
                      "the cove where they were "
                      "hoisted on a block and tackle, their livers removed, their fins cut off and their hides skinned "
                      "out and their flesh cut "
                      "into strips for salting."
                      "\n";

    size_t TEXT_SIZE = 8192;
    std::string str;
    str.append("Start of chain...\n");
    for (size_t i = 0; i < TEXT_SIZE; ++i) {
        str.append("TestLogEntry_");
        str.append(std::to_string(i));
        str.append(" ");
        if (i % 80 == 0) {
            str.append("\n");
        }
    }
    str.append("\nMiddle of chain...\n");
    str.append("End of chain!\n");

    std::string longText = str.c_str();
    SSGX_LOG(INFO) << longText.c_str();
    SSGX_LOG(WARN) << longText.c_str();
    SSGX_LOG(FATAL) << longText.c_str();
}
