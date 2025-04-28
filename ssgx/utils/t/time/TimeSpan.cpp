#include <cstdint>
#include <ctime>

#include "ssgx_utils_t_time.h"

namespace ssgx {
namespace utils_t {

TimeSpan::TimeSpan() noexcept : time_span_(0) {
}

TimeSpan::TimeSpan(time_t time) noexcept : time_span_(time) {
}

TimeSpan::TimeSpan(int64_t num_days, int num_hours, int num_mins, int num_secs) noexcept {
    time_span_ = num_secs + 60 * (num_mins + 60 * (num_hours + 24 * num_days));
}

time_t TimeSpan::GetTimeSpan() const noexcept {
    return time_span_;
}

int64_t TimeSpan::GetDays() const noexcept {
    return (time_span_ / (24 * 3600));
}

int64_t TimeSpan::GetTotalHours() const noexcept {
    return (time_span_ / 3600);
}

int64_t TimeSpan::GetTotalMinutes() const noexcept {
    return (time_span_ / 60);
}

int64_t TimeSpan::GetTotalSeconds() const noexcept {
    return (time_span_);
}

int64_t TimeSpan::GetHours() const noexcept {
    return (GetTotalHours() - (GetDays() * 24));
}

int64_t TimeSpan::GetMinutes() const noexcept {
    return (GetTotalMinutes() - (GetTotalHours() * 60));
}

int64_t TimeSpan::GetSeconds() const noexcept {
    return (GetTotalSeconds() - (GetTotalMinutes() * 60));
}

TimeSpan TimeSpan::operator+(TimeSpan span) const noexcept {
    return TimeSpan(time_span_ + span.time_span_);
}

TimeSpan TimeSpan::operator-(TimeSpan span) const noexcept {
    return TimeSpan(time_span_ - span.time_span_);
}

TimeSpan& TimeSpan::operator+=(TimeSpan span) noexcept {
    time_span_ += span.time_span_;
    return (*this);
}

TimeSpan& TimeSpan::operator-=(TimeSpan span) noexcept {
    time_span_ -= span.time_span_;
    return (*this);
}

bool TimeSpan::operator==(TimeSpan span) const noexcept {
    return (time_span_ == span.time_span_);
}

bool TimeSpan::operator!=(TimeSpan span) const noexcept {
    return (time_span_ != span.time_span_);
}

bool TimeSpan::operator<(TimeSpan span) const noexcept {
    return (time_span_ < span.time_span_);
}

bool TimeSpan::operator>(TimeSpan span) const noexcept {
    return (time_span_ > span.time_span_);
}

bool TimeSpan::operator<=(TimeSpan span) const noexcept {
    return (time_span_ <= span.time_span_);
}

bool TimeSpan::operator>=(TimeSpan span) const noexcept {
    return (time_span_ >= span.time_span_);
}

} // namespace utils_t
} // namespace ssgx