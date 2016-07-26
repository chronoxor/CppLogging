/*!
    \file ostream_appender.cpp
    \brief Logging std::ostream appender implementation
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#include "logging/appenders/ostream_appender.h"

namespace CppLogging {

void OstreamAppender::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.first == nullptr)
        return;

    _ostream.write((char*)record.raw.first, record.raw.second);
}

void OstreamAppender::Flush()
{
    _ostream.flush();
}

} // namespace CppLogging
