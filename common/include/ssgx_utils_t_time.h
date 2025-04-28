#ifndef SAFEHERON_SGX_TRUSTED_TIME_H
#define SAFEHERON_SGX_TRUSTED_TIME_H

#include <cstdint>
#include <ctime>
#include <string>

namespace ssgx {
namespace utils_t {

/**
 * @class TimeSpan
 * @brief Represents a span of time, providing utility functions for calculating days, hours, minutes, and seconds.
 */
class TimeSpan {
  public:
    /**
     * @brief Default constructor, initializes the time span to 0.
     */
    TimeSpan() noexcept;
    /**
     * @brief Constructs a TimeSpan from a given time span in seconds.
     *
     * @param span Time span in seconds.
     */
    explicit TimeSpan(time_t span) noexcept;

    /**
     * @brief Constructs a TimeSpan from days, hours, minutes, and seconds.
     *
     * @param num_days Number of days.
     * @param num_hours Number of hours.
     * @param num_mins Number of minutes.
     * @param num_secs Number of seconds.
     */
    TimeSpan(int64_t num_days, int num_hours, int num_mins, int num_secs) noexcept;

    /**
     * @brief Returns the total time span in seconds.
     *
     * @return Total time span in seconds.
     */
    [[nodiscard]] time_t GetTimeSpan() const noexcept;

    /**
     * @brief Returns the number of days in the time span.
     *
     * @return Number of days.
     */
    [[nodiscard]] int64_t GetDays() const noexcept;

    /**
     * @brief Returns the total number of hours in the time span.
     *
     * @return Total number of hours.
     */
    [[nodiscard]] int64_t GetTotalHours() const noexcept;

    /**
     * @brief Returns the total number of minutes in the time span.
     *
     * @return Total number of minutes.
     */
    [[nodiscard]] int64_t GetTotalMinutes() const noexcept;

    /**
     * @brief Returns the total number of seconds in the time span.
     *
     * @return Total number of seconds.
     */
    [[nodiscard]] int64_t GetTotalSeconds() const noexcept;

    /**
     * @brief Returns the remaining hours after extracting days.
     *
     * @return Number of hours.
     */
    [[nodiscard]] int64_t GetHours() const noexcept;

    /**
     * @brief Returns the remaining minutes after extracting hours.
     *
     * @return Number of minutes.
     */
    [[nodiscard]] int64_t GetMinutes() const noexcept;

    /**
     * @brief Returns the remaining seconds after extracting minutes.
     *
     * @return Number of seconds.
     */
    [[nodiscard]] int64_t GetSeconds() const noexcept;

    // Overloaded operators for arithmetic and comparison
    TimeSpan operator+(TimeSpan span) const noexcept;
    TimeSpan operator-(TimeSpan span) const noexcept;
    TimeSpan& operator+=(TimeSpan span) noexcept;
    TimeSpan& operator-=(TimeSpan span) noexcept;
    bool operator==(TimeSpan span) const noexcept;
    bool operator!=(TimeSpan span) const noexcept;
    bool operator<(TimeSpan span) const noexcept;
    bool operator>(TimeSpan span) const noexcept;
    bool operator<=(TimeSpan span) const noexcept;
    bool operator>=(TimeSpan span) const noexcept;

  private:
    time_t time_span_;
};

/**
 * @class DateTime
 * @brief Represents a specific point in time. Supports UTC/GMT time only.
 */
class DateTime {
  public:
    /**
     * @brief Returns the current system time as a DateTime object.
     *
     * @exception runtime_error Throw the exception if the ssgx_ocall_time call fails or the time verification fails.
     *
     * @return The current time.
     */
    static DateTime Now();

    /**
     * @brief Constructs a DateTime object from a formatted string. Supports the format "YYYY-MM-DD HH:MM:SS".
     *
     * @param formattedTime A string representation of the time.
     * @return A DateTime object representing the given time.
     */
    static DateTime FromFormatTime(const std::string& formattedTime);

    /**
     * @brief Default constructor, initializes the time to epoch (1970-01-01 00:00:00).
     */
    DateTime() noexcept;

    /**
     * @brief Constructs a DateTime object from a UNIX timestamp.
     *
     * @param timestamp The UNIX timestamp.
     */
    explicit DateTime(time_t timestamp) noexcept;

    /**
     * @brief Constructs a DateTime object from year, month, day, hour, minute, and second.
     *
     * @param num_year Year.
     * @param num_month Month.
     * @param num_day Day.
     * @param num_hour Hour.
     * @param num_min Minute.
     * @param num_sec Second.
     */
    DateTime(int num_year, int num_month, int num_day, int num_hour, int num_min, int num_sec);

    // Assignment and arithmetic operators
    DateTime& operator=(time_t timestamp) noexcept;
    DateTime& operator+=(TimeSpan span) noexcept;
    DateTime& operator-=(TimeSpan span) noexcept;
    TimeSpan operator-(DateTime time) const noexcept;
    DateTime operator-(TimeSpan span) const noexcept;
    DateTime operator+(TimeSpan span) const noexcept;

    // Comparison
    bool operator==(DateTime time) const noexcept;
    bool operator!=(DateTime time) const noexcept;
    bool operator<(DateTime time) const noexcept;
    bool operator>(DateTime time) const noexcept;
    bool operator<=(DateTime time) const noexcept;
    bool operator>=(DateTime time) const noexcept;

    // Accessors for time components
    [[nodiscard]] time_t GetTimestamp() const noexcept;
    [[nodiscard]] int GetYear() const noexcept;
    [[nodiscard]] int GetMonth() const noexcept;
    [[nodiscard]] int GetDay() const noexcept;
    [[nodiscard]] int GetHour() const noexcept;
    [[nodiscard]] int GetMinute() const noexcept;
    [[nodiscard]] int GetSecond() const noexcept;
    [[nodiscard]] int GetDayOfWeek() const noexcept;

    /**
     * @brief Returns the time as a formatted string in "YYYY-MM-DD HH:MM:SS" format.
     *
     * @return A formatted string representation of the time.
     * @exception std::invalid_argument("Unexpected exception: int ret = __secs_to_tm(timestamp_, &t);")
     */
    [[nodiscard]] std::string ToFormatTime() const;

  private:
    time_t timestamp_; // Internal representation of the timestamp
};

/**
 * @class PreciseTime
 * @brief Provides high-resolution time utilities for nanoseconds and milliseconds.
 */
class PreciseTime {
  public:
    /**
     * @brief Returns the current time in nanoseconds since the UNIX epoch.
     *
     * @exception runtime_error Throw the exception if the ssgx_ocall_time call fails or the time verification fails.
     *
     * @return Current time in nanoseconds.
     */
    static int64_t NowInNanoseconds();

    /**
     * @brief Returns the current time in milliseconds since the UNIX epoch.
     *
     * @exception runtime_error Throw the exception if the ssgx_ocall_time call fails or the time verification fails.
     *
     * @return Current time in milliseconds.
     */
    static int64_t NowInMilliseconds();
};

} // namespace utils_t
} // namespace ssgx

#endif // SAFEHERON_SGX_TRUSTED_TIME_H
