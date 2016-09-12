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

         \param debug - Debug message
    */
    void Debug(const char* debug);

    //! Log information message
    /*!
         \param info - Information message
    */
    void Info(const char* info);

    //! Log warning message
    /*!
         \param warn - Warning message
    */
    void Warn(const char* warn);

    //! Log error message
    /*!
         \param error - Error message
    */
    void Error(const char* error);

    //! Log fatal message
    /*!
         \param fatal - Fatal message
    */
    void Fatal(const char* fatal);

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

    //! Log the given message with a given level
    /*!
         \param level - Logging level
         \param message - Logging message
    */
    void Log(Level level, const char* message);
};

} // namespace CppLogging

#endif // CPPLOGGING_LOGGER_H
