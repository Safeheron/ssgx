#ifndef SAFEHERON_SGX_LIBRARY_LOG_U_H
#define SAFEHERON_SGX_LIBRARY_LOG_U_H

#include <string>

#include "ssgx_log_u_log_message.h"

namespace ssgx {
namespace log_u {

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
 * logger.Init("AppLogger", "app.log", SSGXLogger::INFO, true);
 * logger.SetTraceId("12345");
 * @endcode
 */
class SSGXLogger {
  public:
    /**
     * @brief Retrieves the singleton instance of SSGXLogger.
     *
     * This method provides thread-safe access to the single instance of the logger.
     * @return Reference to the singleton instance of SSGXLogger.
     */
    static SSGXLogger& GetInstance();

    /**
     * @brief Initializes the logger with specified settings.
     *
     * Configures the logger with a given name, log file, log level, and optional console appender.
     * This method should be called before logging any messages.
     *
     * @param logger_name Name of the logger (e.g., "AppLogger").
     * @param log_file Path to the log file.
     * @param log_level Logging level to filter log messages.
     * @param append_console Whether to output log messages to the console.
     * Default is false.
     */
    void Init(const std::string& logger_name, const std::string& log_file,
              LogLevel log_level = ssgx::log_u::LogLevel::INFO, bool append_console = false);

    /**
     * @brief Retrieves the singleton instance of SSGXLogger.
     *
     * This method provides thread-safe access to the single instance of the logger.
     * @return Reference to the singleton instance of SSGXLogger.
     */
    std::string GetLoggerName() {
        return logger_name_;
    }

    /**
     * @brief Sets a trace ID for log messages.
     *
     * The trace ID is stored in the logging context and can be used to correlate log messages across different parts of
     * the application. To include the trace ID in log messages, the log pattern must contain `%X{TraceId}`.
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

    bool IsInitialized();

    /**
     * @brief Logs a message with the specified log level.
     *
     * This function logs a message to the configured log system with the specified log level. It ensures that the
     * logger has been properly initialized before attempting to log the message.
     *
     * @param level The log level indicating the severity of the message.
     *              Possible values include INFO, WARNING, ERROR, DEBUG, FATAL,
     * etc.
     * @param msg The log message to be recorded.
     * @return
     *         0 - Success: The message was logged successfully. \n
     *        -1 - Failure: The logger is not initialized or an error occurred
     * during logging.
     */
    int WriteLog(ssgx::log_u::LogLevel log_level, const char* msg);

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
    ~SSGXLogger() = default;

    /**
     * @brief Maps the custom LogLevel to the corresponding log4cplus log level.
     *
     * @param log_level LogLevel from the SSGXLogger enum.
     * @return The corresponding log4cplus log level as an integer.
     */
    int MapLogLevel(LogLevel log_level) const;

    std::string logger_name_; ///< Name of the logger.
    bool initialized_;
};

} // namespace log_u
} // namespace ssgx

#endif // SAFEHERON_SGX_LIBRARY_LOG_U_H
