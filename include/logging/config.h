/*!
    \file config.h
    \brief Logger configuration definition
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_CONFIG_H
#define CPPLOGGING_CONFIG_H

#include "logging/logger.h"

#include <map>

namespace CppLogging {

//! Logger configuration static class
/*!
    Logger configuration provides static interface to configure loggers.

    Thread-safe.
*/
class Config
{
public:
    Config() = delete;
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    ~Config() = delete;

    Config& operator=(const Config&) = delete;
    Config& operator=(Config&&) = delete;

    //! Configure default logger with a given logging sink processor
    /*!
         \param sink - Logging sink processor
    */
    static void ConfigLogger(const std::shared_ptr<Processor>& sink);
    //! Configure named logger with a given logging sink processor
    /*!
         \param name - Logger name
         \param sink - Logging sink processor
    */
    static void ConfigureLogger(const std::string& name, const std::shared_ptr<Processor>& sink);

    //! Create default logger
    /*!
         If the default logger was not configured before it will be automatically
         configured to a one with TextLayout and ConsoleAppender.

         \return Created instance of the default logger
    */
    static Logger CreateLogger();
    //! Create named logger
    /*!
         If the named logger was not configured before an instance of the default
         logger will be returned.

         \param name - Logger name
         \return Created instance of the named logger
    */
    static Logger CreateLogger(const std::string& name);

private:
    static CppCommon::Mutex _lock;
    static std::map<std::string, std::shared_ptr<Processor>> _config;
};

} // namespace CppLogging

#endif // CPPLOGGING_LOGGER_H
