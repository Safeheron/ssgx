#include <cstdint>
#include <ctime>
#include <exception>
#include <ssgx_utils_t_time.h>
#include <stdexcept>
#include <string>

#include "sgx_error.h"

#include "ssgx_utils_t_t.h"

#include "time_impl.h"
#include "TimeVerifier.h"

namespace ssgx {
namespace utils_t {

DateTime DateTime::Now() {
    static TimeVerifier time_verifier;

    uint64_t t_now = 0;
    sgx_status_t status = ssgx_ocall_time(&t_now);

    if (status != SGX_SUCCESS) {
        throw std::runtime_error(std::string("Failed in ssgx_ocall_time (error code: ") + std::to_string(status) + ")");
    }

    if (!time_verifier.Verify(t_now)) {
        throw std::runtime_error(std::string("Failed in ssgx_ocall_time (invalid time from untrusted system)"));
    }

    return DateTime(static_cast<time_t >(t_now));
}

DateTime DateTime::FromFormatTime(const std::string& formattedTime) {
    int num_year = 0;
    int num_month = 0;
    int num_day = 0;
    int num_hour = 0;
    int num_min = 0;
    int num_sec = 0;

    // Only support "YYYY-MM-DD HH:MM:SS"
    if (formattedTime.length() != 19) {
        throw std::invalid_argument("Invalid formated time string!");
    }

    try {
        std::string year = formattedTime.substr(0, 4);
        std::string month = formattedTime.substr(5, 2);
        std::string day = formattedTime.substr(8, 2);
        std::string hour = formattedTime.substr(11, 2);
        std::string minute = formattedTime.substr(14, 2);
        std::string second = formattedTime.substr(17, 2);
        num_year = std::stoi(year);
        num_month = std::stoi(month);
        num_day = std::stoi(day);
        num_hour = std::stoi(hour);
        num_min = std::stoi(minute);
        num_sec = std::stoi(second);
    } catch (std::exception& e) {
        throw std::invalid_argument(std::string("Invalid formated time string! Info: ") + e.what());
    }

    return DateTime(num_year, num_month, num_day, num_hour, num_min, num_sec);
}

DateTime::DateTime() noexcept : timestamp_(0) {
}

DateTime::DateTime(time_t timestamp) noexcept : timestamp_(timestamp) {
}

DateTime::DateTime(int num_year, int num_month, int num_day, int num_hour, int num_min, int num_sec) {
    bool ok = false;

    ok = (num_year >= 1900);
    if (!ok)
        throw std::invalid_argument("Invalid num_year!");

    ok = (num_month >= 1 && num_month <= 12);
    if (!ok)
        throw std::invalid_argument("Invalid num_month!");

    ok = (num_day >= 1 && num_day <= 31);
    if (!ok)
        throw std::invalid_argument("Invalid num_day!");

    ok = (num_hour >= 0 && num_hour <= 23);
    if (!ok)
        throw std::invalid_argument("Invalid num_hour!");

    ok = (num_min >= 0 && num_min <= 59);
    if (!ok)
        throw std::invalid_argument("Invalid num_min!");

    ok = (num_sec >= 0 && num_sec <= 59);
    if (!ok)
        throw std::invalid_argument("Invalid num_sec!");

    struct tm t;

    t.tm_sec = num_sec;
    t.tm_min = num_min;
    t.tm_hour = num_hour;
    t.tm_mday = num_day;
    t.tm_mon = num_month - 1;    // tm_mon is 0 based
    t.tm_year = num_year - 1900; // tm_year is 1900 based
    t.tm_isdst = -1;

    timestamp_ = __tm_to_secs(&t);
}

DateTime& DateTime::operator=(time_t timestamp) noexcept {
    timestamp_ = timestamp;
    return (*this);
}

DateTime& DateTime::operator+=(TimeSpan span) noexcept {
    timestamp_ += span.GetTimeSpan();
    return (*this);
}

DateTime& DateTime::operator-=(TimeSpan span) noexcept {
    timestamp_ -= span.GetTimeSpan();
    return (*this);
}

TimeSpan DateTime::operator-(DateTime time) const noexcept {
    return (TimeSpan(timestamp_ - time.timestamp_));
}

DateTime DateTime::operator-(TimeSpan span) const noexcept {
    return (DateTime(timestamp_ - span.GetTimeSpan()));
}

DateTime DateTime::operator+(TimeSpan span) const noexcept {
    return (DateTime(timestamp_ + span.GetTimeSpan()));
}

bool DateTime::operator==(DateTime time) const noexcept {
    return (timestamp_ == time.timestamp_);
}

bool DateTime::operator!=(DateTime time) const noexcept {
    return (timestamp_ != time.timestamp_);
}

bool DateTime::operator<(DateTime time) const noexcept {
    return (timestamp_ < time.timestamp_);
}

bool DateTime::operator>(DateTime time) const noexcept {
    return (timestamp_ > time.timestamp_);
}

bool DateTime::operator<=(DateTime time) const noexcept {
    return (timestamp_ <= time.timestamp_);
}

bool DateTime::operator>=(DateTime time) const noexcept {
    return (timestamp_ >= time.timestamp_);
}

time_t DateTime::GetTimestamp() const noexcept {
    return (timestamp_);
}

int DateTime::GetYear() const noexcept {
    struct tm t;
    int ret = __secs_to_tm(timestamp_, &t);
    return (ret == 0) ? (t.tm_year + 1900) : -1;
}

int DateTime::GetMonth() const noexcept {
    struct tm t;
    int ret = __secs_to_tm(timestamp_, &t);
    return (ret == 0) ? (t.tm_mon + 1) : -1;
}

int DateTime::GetDay() const noexcept {
    struct tm t;
    int ret = __secs_to_tm(timestamp_, &t);
    return (ret == 0) ? t.tm_mday : -1;
}

int DateTime::GetHour() const noexcept {
    struct tm t;
    int ret = __secs_to_tm(timestamp_, &t);
    return (ret == 0) ? t.tm_hour : -1;
}

int DateTime::GetMinute() const noexcept {
    struct tm t;
    int ret = __secs_to_tm(timestamp_, &t);
    return (ret == 0) ? t.tm_min : -1;
}

int DateTime::GetSecond() const noexcept {
    struct tm t;
    int ret = __secs_to_tm(timestamp_, &t);
    return (ret == 0) ? t.tm_sec : -1;
}

int DateTime::GetDayOfWeek() const noexcept {
    struct tm t;
    int ret = __secs_to_tm(timestamp_, &t);
    return (ret == 0) ? t.tm_wday : -1;
}

std::string DateTime::ToFormatTime() const {
    struct tm t;
    memset(&t, 0, sizeof t);
    // check and throw exception
    int ret = __secs_to_tm(timestamp_, &t);
    if (ret < 0)
        throw std::invalid_argument("Unexpected exception: int ret = __secs_to_tm(timestamp_, &t);");

    std::string format_time;
    char buf[80];
    memset(buf, 0, sizeof buf);
    if (strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t) > 0) {
        format_time.assign(buf, strlen(buf));
    }
    return format_time;
}

} // namespace utils_t
} // namespace ssgx
