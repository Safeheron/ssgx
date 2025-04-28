#include "ssgx_testframework_t.h"
#include "ssgx_utils_t.h"

using namespace ssgx::utils_t;

// TimeSpan Tests
TEST(TimeSpan, DefaultConstructor) {
    TimeSpan span;
    ASSERT_EQ(span.GetTimeSpan(), 0);
}

TEST(TimeSpan, ConstructorFromSeconds) {
    TimeSpan span(3600);
    ASSERT_EQ(span.GetTimeSpan(), 3600);
    ASSERT_EQ(span.GetTotalMinutes(), 60);
}

TEST(TimeSpan, ConstructorFromComponents) {
    TimeSpan span(1, 2, 3, 4);           // 1 day, 2 hours, 3 minutes, 4 seconds
    ASSERT_EQ(span.GetTimeSpan(), 93784); // Total seconds
    ASSERT_EQ(span.GetDays(), 1);
    ASSERT_EQ(span.GetHours(), 2);
    ASSERT_EQ(span.GetMinutes(), 3);
    ASSERT_EQ(span.GetSeconds(), 4);
}

TEST(TimeSpan, GetTotalValues) {
    TimeSpan span(1, 2, 3, 4); // 1 day, 2 hours, 3 minutes, 4 seconds
    ASSERT_EQ(span.GetTotalHours(), 26);
    ASSERT_EQ(span.GetTotalMinutes(), 1563);
    ASSERT_EQ(span.GetTotalSeconds(), 93784);
}

TEST(TimeSpan, ArithmeticOperators) {
    TimeSpan span1(3600); // 1 hour
    TimeSpan span2(1800); // 30 minutes

    TimeSpan sum = span1 + span2;
    ASSERT_EQ(sum.GetTimeSpan(), 5400); // 1.5 hours

    TimeSpan diff = span1 - span2;
    ASSERT_EQ(diff.GetTimeSpan(), 1800); // 30 minutes

    span1 += span2;
    ASSERT_EQ(span1.GetTimeSpan(), 5400); // Updated span1

    span1 -= span2;
    ASSERT_EQ(span1.GetTimeSpan(), 3600); // Back to original
}

TEST(TimeSpan, ComparisonOperators) {
    TimeSpan span1(3600); // 1 hour
    TimeSpan span2(1800); // 30 minutes

    ASSERT_TRUE(span1 > span2);
    ASSERT_TRUE(span2 < span1);
    ASSERT_FALSE(span1 == span2);
    ASSERT_TRUE(span1 >= span2);
    ASSERT_FALSE(span2 >= span1);
}

// DateTime Tests
TEST(DateTime, DefaultConstructor) {
    DateTime time;
    ASSERT_EQ(time.GetTimestamp(), 0);
}

TEST(DateTime, ConstructorFromTimestamp) {
    DateTime time(1672531200); // "2023-01-01 00:00:00" UTC
    ASSERT_EQ(time.GetYear(), 2023);
    ASSERT_EQ(time.GetMonth(), 1);
    ASSERT_EQ(time.GetDay(), 1);
    ASSERT_EQ(time.GetHour(), 0);
    ASSERT_EQ(time.GetMinute(), 0);
    ASSERT_EQ(time.GetSecond(), 0);
    ASSERT_EQ(time.ToFormatTime(), "2023-01-01 00:00:00");
}

TEST(DateTime, ConstructorFromComponents) {
    DateTime time(2023, 12, 21, 14, 30, 0);
    ASSERT_EQ(time.GetYear(), 2023);
    ASSERT_EQ(time.GetMonth(), 12);
    ASSERT_EQ(time.GetDay(), 21);
    ASSERT_EQ(time.GetHour(), 14);
    ASSERT_EQ(time.GetMinute(), 30);
    ASSERT_EQ(time.GetSecond(), 0);
    ASSERT_EQ(time.ToFormatTime(), "2023-12-21 14:30:00");
}

TEST(DateTime, GetDayOfWeek) {
    DateTime time(2023, 12, 21, 0, 0, 0); // Thursday
    ASSERT_EQ(time.GetDayOfWeek(), 4); // 0=Sunday, ..., 4=Thursday
}

TEST(DateTime, ArithmeticOperators) {
    DateTime time(1672531200); // "2023-01-01 00:00:00" UTC
    TimeSpan span(86400);  // 1 day in seconds

    DateTime newTime = time + span;
    ASSERT_EQ(newTime.ToFormatTime(), "2023-01-02 00:00:00");

    newTime -= span;
    ASSERT_EQ(newTime.ToFormatTime(), "2023-01-01 00:00:00");

    TimeSpan diff = newTime - time;
    ASSERT_EQ(diff.GetTimeSpan(), 0); // Same time
}

TEST(DateTime, ComparisonOperators) {
    DateTime time1(1672531200); // "2023-01-01 00:00:00" UTC
    DateTime time2(1672617600); // "2023-01-02 00:00:00" UTC

    ASSERT_TRUE(time1 < time2);
    ASSERT_TRUE(time2 > time1);
    ASSERT_FALSE(time1 == time2);
    ASSERT_TRUE(time1 <= time2);
    ASSERT_FALSE(time2 <= time1);
}

TEST(DateTime, FromFormatTime) {
    DateTime time = DateTime::FromFormatTime("2023-12-21 14:30:00");
    ASSERT_EQ(time.GetYear(), 2023);
    ASSERT_EQ(time.GetMonth(), 12);
    ASSERT_EQ(time.GetDay(), 21);
    ASSERT_EQ(time.GetHour(), 14);
    ASSERT_EQ(time.GetMinute(), 30);
    ASSERT_EQ(time.GetSecond(), 0);
}

// PreciseTime Tests
TEST(PreciseTime, NowInMilliseconds) {
    int64_t start = PreciseTime::NowInMilliseconds();
    ssgx::utils_t::Printf("Sleeping for 3s...\n");
    ssgx::utils_t::Sleep(3); // Assuming a custom sleep function
    int64_t end = PreciseTime::NowInMilliseconds();

    ssgx::utils_t::Printf("end - start = %zu\n", end - start);
    ASSERT_GE(end - start, 3 * 1000);
}

TEST(PreciseTime, NowInNanoseconds) {
    int64_t start = PreciseTime::NowInNanoseconds();
    ssgx::utils_t::Printf("Sleeping for 5s...\n");
    ssgx::utils_t::Sleep(5); // Assuming a custom sleep function

    int64_t end = PreciseTime::NowInNanoseconds();

    ssgx::utils_t::Printf("end - start = %zu\n", end - start);
    ASSERT_GE(end - start, 5 * (int64_t)1000000000); // 5s in nanoseconds
}
