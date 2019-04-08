/*!
    \file error_appender.h
    \brief Error (stderr) appender definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_ERROR_APPENDER_H
#define CPPLOGGING_APPENDERS_ERROR_APPENDER_H

#include "logging/appender.h"

namespace CppLogging {

//! Error (stderr) appender
/*!
    Error appender prints the given logging record into
    the console or system error stream (stderr).

    Thread-safe.
*/
class ErrorAppender : public Appender
{
public:
    ErrorAppender() = default;
    ErrorAppender(const ErrorAppender&) = delete;
    ErrorAppender(ErrorAppender&&) = delete;
    virtual ~ErrorAppender() = default;

    ErrorAppender& operator=(const ErrorAppender&) = delete;
    ErrorAppender& operator=(ErrorAppender&&) = delete;

    // Implementation of Appender
    void AppendRecord(Record& record) override;
    void Flush() override;
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDERS_ERROR_APPENDER_H
