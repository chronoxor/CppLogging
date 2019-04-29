/*!
    \file appender.h
    \brief Logging appender interface definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDER_H
#define CPPLOGGING_APPENDER_H

#include "logging/element.h"
#include "logging/record.h"

namespace CppLogging {

//! Logging appender interface
/*!
    Logging appender takes an instance of a single logging record
    and store it content in some storage or show it in console.

    \see NullAppender
    \see ConsoleAppender
    \see DebugAppender
    \see ErrorAppender
    \see MemoryAppender
    \see OstreamAppender
    \see FileAppender
    \see RollingFileAppender
    \see SysLogAppender
*/
class Appender : public Element
{
public:
    //! Append the given logging record
    /*!
         \param record - Logging record
    */
    virtual void AppendRecord(Record& record) = 0;

    //! Flush the logging appender
    virtual void Flush() {}
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDER_H
