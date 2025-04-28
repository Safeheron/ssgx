// namespace ssgx {
// namespace log_t/log_u {
namespace internal {

LogMessage::LogMessage(LogLevel level, const char* filename, int line, const char* func_name)
    : level_(level), filename_(filename), line_(line), func_name_(func_name) {
}

LogMessage& LogMessage::operator<<(const std::string& value) {
    message_ += value;
    return *this;
}

LogMessage& LogMessage::operator<<(const char* value) {
    message_ += value;
    return *this;
}

LogMessage& LogMessage::operator<<(char value) {
    std::string str(&value, 1);
    message_ += str;
    return *this;
}

LogMessage& LogMessage::operator<<(int value) {
    return *this << std::to_string(value);
}

LogMessage& LogMessage::operator<<(long value) {
    return *this << std::to_string(value);
}

LogMessage& LogMessage::operator<<(long long value) {
    return *this << std::to_string(value);
}

LogMessage& LogMessage::operator<<(unsigned value) {
    return *this << std::to_string(value);
}

LogMessage& LogMessage::operator<<(unsigned long value) {
    return *this << std::to_string(value);
}

LogMessage& LogMessage::operator<<(unsigned long long value) {
    return *this << std::to_string(value);
}

LogMessage& LogMessage::operator<<(float value) {
    return *this << std::to_string(value);
}

LogMessage& LogMessage::operator<<(double value) {
    return *this << std::to_string(value);
}

LogMessage& LogMessage::operator<<(long double value) {
    return *this << std::to_string(value);
}

LogMessage::~LogMessage() {
}

void LogFinisher::operator=(LogMessage& other) {
    other.Finish();
}

} // namespace internal
// } namespace log_t/log_u
// } namespace ssgx
