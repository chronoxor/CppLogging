/*!
    \file debug_appender.h
    \brief Logging debug appender definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_DEBUG_APPENDER_H
#define CPPLOGGING_APPENDERS_DEBUG_APPENDER_H

#include "logging/appender.h"

namespace CppLogging {

//! Logging debug appender
/*!
    Logging debug appender prints the given logging record into
    the attached debugger if present.

    Thread-safe.
*/
class DebugAppender : public Appender
{
public:
    DebugAppender() = default;
    DebugAppender(const DebugAppender&) = delete;
    DebugAppender(DebugAppender&&) = default;
    ~DebugAppender() = default;

    DebugAppender& operator=(const DebugAppender&) = delete;
    DebugAppender& operator=(DebugAppender&&) = default;

    // Implementation of Appender
    void AppendRecord(Record& record) override;
    void Flush() override;
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDERS_DEBUG_APPENDER_H
