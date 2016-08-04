/*!
    \file ostream_appender.cpp
    \brief Output stream (std::ostream) appender implementation
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#include "logging/appenders/ostream_appender.h"

namespace CppLogging {

void OstreamAppender::AppendRecord(Record& record)
{
    // Skip logging records without layout
    if (record.raw.empty())
        return;

    // Append logging record content
    _ostream.write((char*)record.raw.data(), record.raw.size() - 1);
}

void OstreamAppender::Flush()
{
    // Flush stream
    _ostream.flush();
}

} // namespace CppLogging
