/*!
    \file console_appender.h
    \brief Console (stdout) appender definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_CONSOLE_APPENDER_H
#define CPPLOGGING_APPENDERS_CONSOLE_APPENDER_H

#include "logging/appender.h"

namespace CppLogging {

//! Console (stdout) appender
/*!
    Console appender prints the given logging record into
    the console or system output stream (stdout).

    Thread-safe.
*/
class ConsoleAppender : public Appender
{
public:
    ConsoleAppender() = default;
    ConsoleAppender(const ConsoleAppender&) = delete;
    ConsoleAppender(ConsoleAppender&&) = delete;
    virtual ~ConsoleAppender() = default;

    ConsoleAppender& operator=(const ConsoleAppender&) = delete;
    ConsoleAppender& operator=(ConsoleAppender&&) = delete;

    // Implementation of Appender
    void AppendRecord(Record& record) override;
    void Flush() override;
};

} // namespace CppLogging

/*! \example console.cpp Console logger example */

#endif // CPPLOGGING_APPENDERS_CONSOLE_APPENDER_H
