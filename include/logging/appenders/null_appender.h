/*!
    \file null_appender.h
    \brief Logging null appender definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_NULL_APPENDER_H
#define CPPLOGGING_APPENDERS_NULL_APPENDER_H

#include "logging/appender.h"

namespace CppLogging {

//! Logging null appender
/*!
    Logging null appender does nothing with a given logging record.

    Thread-safe.
*/
class NullAppender : public Appender
{
public:
    NullAppender() = default;
    NullAppender(const NullAppender&) = delete;
    NullAppender(NullAppender&&) = default;
    ~NullAppender() = default;

    NullAppender& operator=(const NullAppender&) = delete;
    NullAppender& operator=(NullAppender&&) = default;

    // Implementation of Appender
    void AppendRecord(Record& record) override {}
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDERS_NULL_APPENDER_H
