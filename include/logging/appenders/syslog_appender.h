/*!
    \file syslog_appender.h
    \brief Syslog appender definition
    \author Ivan Shynkarenka
    \date 28.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_SYSLOG_APPENDER_H
#define CPPLOGGING_APPENDERS_SYSLOG_APPENDER_H

#include "logging/appender.h"

namespace CppLogging {

//! Syslog appender
/*!
    Syslog appender forward logging record to the syslog() system call
    for Unix systems. Under Windows systems this appender does nothing!

    Thread-safe.
*/
class SyslogAppender : public Appender
{
public:
    SyslogAppender();
    SyslogAppender(const SyslogAppender&) = delete;
    SyslogAppender(SyslogAppender&&) = delete;
    virtual ~SyslogAppender();

    SyslogAppender& operator=(const SyslogAppender&) = delete;
    SyslogAppender& operator=(SyslogAppender&&) = delete;

    // Implementation of Appender
    void AppendRecord(Record& record) override;
};

} // namespace CppLogging

/*! \example syslog.cpp Syslog logger example */

#endif // CPPLOGGING_APPENDERS_SYSLOG_APPENDER_H
