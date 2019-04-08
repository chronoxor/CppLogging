/*!
    \file debug_appender.h
    \brief Debug appender definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_DEBUG_APPENDER_H
#define CPPLOGGING_APPENDERS_DEBUG_APPENDER_H

#include "logging/appender.h"

namespace CppLogging {

//! Debug appender
/*!
    Debug appender prints the given logging record into
    the attached debugger if present for Windows system.
    Under Unix systems this appender prints the given
    logging record into the system error stream (stderr).

    Thread-safe.
*/
class DebugAppender : public Appender
{
public:
    DebugAppender() = default;
    DebugAppender(const DebugAppender&) = delete;
    DebugAppender(DebugAppender&&) = delete;
    virtual ~DebugAppender() = default;

    DebugAppender& operator=(const DebugAppender&) = delete;
    DebugAppender& operator=(DebugAppender&&) = delete;

    // Implementation of Appender
    void AppendRecord(Record& record) override;
    void Flush() override;
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDERS_DEBUG_APPENDER_H
