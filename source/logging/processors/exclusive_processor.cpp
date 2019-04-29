/*!
    \file exclusive_processor.cpp
    \brief Exclusive logging processor implementation
    \author Ivan Shynkarenka
    \date 11.04.2019
    \copyright MIT License
*/

#include "logging/processors/exclusive_processor.h"

namespace CppLogging {

bool ExclusiveProcessor::ProcessRecord(Record& record)
{
    // Check if the logging processor started
    if (!IsStarted())
        return true;

    // Filter the given logging record
    if (!FilterRecord(record))
        return true;

    // Layout the given logging record
    if (_layout && _layout->IsStarted())
        _layout->LayoutRecord(record);

    // Append the given logging record
    for (auto& appender : _appenders)
        if (appender && appender->IsStarted())
            appender->AppendRecord(record);

    // Process the given logging record with sub processors
    for (auto& processor : _processors)
        if (processor && processor->IsStarted() && !processor->ProcessRecord(record))
            return false;

    // Logging record was exclusively processed!
    return false;
}

} // namespace CppLogging
