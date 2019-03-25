/*!
    \file logger.h
    \brief Logger interface definition
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_LOGGER_H
#define CPPLOGGING_LOGGER_H

#include "logging/processors.h"

namespace CppLogging {

//! Logger interface
/*!
    Logger is a main interface to produce logging records with a desired level.

    Thread-safe or not thread-safe depends on the current logging sink.
*/
class Logger
{
    friend class Config;

public:
    //! Initialize default logger
    Logger();
    //! Initialize named logger
    /*!
         \param name - Logger name
    */
    explicit Logger(const std::string& name);
    Logger(const Logger&) = default;
    Logger(Logger&&) = default;
    ~Logger();

    Logger& operator=(const Logger&) = default;
    Logger& operator=(Logger&&) = default;

    //! Log debug message with format arguments
    /*!
         Will log only in debug mode!

         \param debug - Debug message
         \param args - Format arguments
    */
    template <typename... Args>
    void Debug(std::string_view debug, const Args&... args);

    //! Log information message with format arguments
    /*!
         \param info - Information message
         \param args - Format arguments
    */
    template <typename... Args>
    void Info(std::string_view info, const Args&... args);

    //! Log warning message with format arguments
    /*!
         \param warn - Warning message
         \param args - Format arguments
    */
    template <typename... Args>
    void Warn(std::string_view warn, const Args&... args);

    //! Log error message with format arguments
    /*!
         \param error - Error message
         \param args - Format arguments
    */
    template <typename... Args>
    void Error(std::string_view error, const Args&... args);

    //! Log fatal message with format arguments
    /*!
         \param fatal - Fatal message
         \param args - Format arguments
    */
    template <typename... Args>
    void Fatal(std::string_view fatal, const Args&... args);

    //! Flush the current logger
    void Flush();

    //! Update the current logger sink by taking the most recent one from configuration
    void Update();

private:
    std::string _name;
    std::shared_ptr<Processor> _sink;

    //! Initialize logger
    /*!
         \param name - Logger name
         \param sink - Logger sink processor
    */
    explicit Logger(const std::string& name, const std::shared_ptr<Processor>& sink);

    //! Log the given message with a given level and format arguments list
    /*!
         \param level - Logging level
         \param format - Format flag
         \param message - Logging message
         \param args - Format arguments list
    */
    template <typename... Args>
    void Log(Level level, bool format, std::string_view message, const Args&... args);
};

} // namespace CppLogging

#include "logger.inl"

/*! \example default.cpp Default logger example */
/*! \example format.cpp Format logger example */

#endif // CPPLOGGING_LOGGER_H
