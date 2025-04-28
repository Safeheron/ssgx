#ifndef SAFEHERON_SGX_LOG_SHARE_H
#define SAFEHERON_SGX_LOG_SHARE_H

// namespace ssgx {
// namespace log_t/log_u {

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
    OFF = 6,
};

namespace internal {

class LogFinisher;

class LogMessage {
  public:
    LogMessage(LogLevel level, const char* filename, int line, const char* func_name);

    ~LogMessage();

    LogMessage& operator<<(const std::string& value);

    LogMessage& operator<<(const char* value);

    LogMessage& operator<<(char value);

    LogMessage& operator<<(int value);

    LogMessage& operator<<(long value);

    LogMessage& operator<<(long long value);

    LogMessage& operator<<(unsigned int value);

    LogMessage& operator<<(unsigned long value);

    LogMessage& operator<<(unsigned long long value);

    LogMessage& operator<<(float value);

    LogMessage& operator<<(double value);

    LogMessage& operator<<(long double value);

  private:
    friend class LogFinisher;

    void Finish();

    LogLevel level_;
    const char* filename_;
    int line_;
    const char* func_name_;
    std::string message_;
};

class LogFinisher {
  public:
    void operator=(LogMessage& other);
};

} // namespace internal

// } namespace log_t/log_u
// } namespace ssgx

#endif // SAFEHERON_SGX_LOG_SHARE_H
