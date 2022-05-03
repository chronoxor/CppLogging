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

    //! Log debug message
    /*!
         Will log only in debug mode!

         \param message - Debug message
    */
    void Debug(std::string_view message) const { Debug("{}", message); }
    //! Log debug message with format arguments
    /*!
         Will log only in debug mode!

         \param message - Debug message
         \param args - Format arguments
    */
    template <typename... T>
    void Debug(fmt::format_string<T...> message, T&&... args) const;

    //! Log information message
    /*!
         \param message - Information message
    */
    void Info(std::string_view message) const { Info("{}", message); }
    //! Log information message with format arguments
    /*!
         \param message - Information message
         \param args - Format arguments
    */
    template <typename... T>
    void Info(fmt::format_string<T...> message, T&&... args) const;

    //! Log warning message
    /*!
         \param message - Warning message
    */
    void Warn(std::string_view message) const { Warn("{}", message); }
    //! Log warning message with format arguments
    /*!
         \param message - Warning message
         \param args - Format arguments
    */
    template <typename... T>
    void Warn(fmt::format_string<T...> message, T&&... args) const;

    //! Log error message
    /*!
         \param message - Error message
    */
    void Error(std::string_view message) const { Error("{}", message); }
    //! Log error message with format arguments
    /*!
         \param message - Error message
         \param args - Format arguments
    */
    template <typename... T>
    void Error(fmt::format_string<T...> message, T&&... args) const;

    //! Log fatal message
    /*!
         \param message - Fatal message
    */
    void Fatal(std::string_view message) const { Fatal("{}", message); }
    //! Log fatal message with format arguments
    /*!
         \param message - Fatal message
         \param args - Format arguments
    */
    template <typename... T>
    void Fatal(fmt::format_string<T...> message, T&&... args) const;

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
    template <typename... T>
    void Log(Level level, bool format, fmt::format_string<T...> message, T&&... args) const;
};

} // namespace CppLogging

#include "logger.inl"

/*! \example default.cpp Default logger example */
/*! \example format.cpp Format logger example */

#endif // CPPLOGGING_LOGGER_H
