/*!
    \file logger.h
    \brief Logger interface definition
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_LOGGER_H
#define CPPLOGGING_LOGGER_H

#include "logging/processor.h"

namespace CppLogging {

//! Logger interface
/*!
    Logger is a main interface to produce logging records with a desired level.

    Thread-safe or not thread-safe.
*/
class Logger
{
public:
    //! Initialize a new default or named logger
    /*!
         \param name - Logger name (default is "")
    */
    explicit Logger(const std::string& name = "");
    Logger(const Logger&) = default;
    Logger(Logger&&) = default;
    ~Logger() = default;

    Logger& operator=(const Logger&) = default;
    Logger& operator=(Logger&&) = default;

    //! Log the given logging record
    /*!
         \param record - Logging record
    */
    void Log(Record& record);

    //! Log debug message
    /*!
         Will log only in debug mode!

         \param debug - Debug message
    */
    void Debug(const std::string& debug);

    //! Log information message
    /*!
         \param info - Information message
    */
    void Info(const std::string& info);

    //! Log warning message
    /*!
         \param warn - Warning message
    */
    void Warn(const std::string& warn);

    //! Log error message
    /*!
         \param error - Error message
    */
    void Error(const std::string& error);

    //! Log fatal message
    /*!
         \param fatal - Fatal message
    */
    void Fatal(const std::string& fatal);

    //! Update the current logger sink by taking the most recent one from configuration
    void Update();

private:
    std::string _name;
    std::shared_ptr<Processor> _sink;

    //! Log the given message with a given level
    /*!
         \param level - Logging level
         \param message - Logging message
    */
    void Log(Level level, const std::string& message);
};

} // namespace CppLogging

#endif // CPPLOGGING_LOGGER_H
