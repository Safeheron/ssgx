#ifndef SAFEHERON_SGX_LIBRARY_LOG_T_H
#define SAFEHERON_SGX_LIBRARY_LOG_T_H

#include <string>

#include "ssgx_log_t_log_message.h"

namespace ssgx {
namespace log_t {

/**
 * @class SSGXLogger
 * @brief Singleton class for managing logging within the SGX library.
 *
 * This class provides a centralized logging interface for the SGX library.
 * It supports log level configuration, trace ID management, and the ability
 * to initialize a logger with file and console appenders.
 *
 * Usage:
 * @code
 * auto& logger = SSGXLogger::GetInstance();
 * logger.SetTraceId("12345");
 * @endcode
 */
class SSGXLogger {
  public:
    /**
     * @brief Retrieves the singleton instance of SSGXLogger.
     *
     * This method provides thread-safe access to the single instance of the
     * logger.
     * @return Reference to the singleton instance of SSGXLogger.
     */
    static SSGXLogger& GetInstance();

    /**
     * @brief Sets a trace ID for log messages.
     *
     * The trace ID is stored in the logging context and can be used to correlate
     * log messages across different parts of the application. To include the
     * trace ID in log messages, the log pattern must contain `%X{TraceId}`.
     *
     * Example:
     * @code
     * SSGXLogger::GetInstance().SetTraceId("12345");
     * @endcode
     *
     * @param trace_id The trace ID string to be included in subsequent log
     * messages.
     */
    void SetTraceId(const std::string& trace_id);

    // Delete copy constructor and assignment operator
    SSGXLogger(const SSGXLogger&) = delete;
    SSGXLogger& operator=(const SSGXLogger&) = delete;

  private:
    /**
     * @brief Constructs the logger. Private to enforce singleton pattern.
     */
    SSGXLogger();

    /**
     * @brief Destructs the logger. Private to enforce singleton pattern.
     */
    ~SSGXLogger();
};

} // namespace log_t
} // namespace ssgx

#endif // SAFEHERON_SGX_LIBRARY_LOG_T_H
