/*!
    \file processor.cpp
    \brief Logging processor interface implementation
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#include "logging/processor.h"

namespace CppLogging {

Processor::~Processor()
{
    // Flush all appenders
    for (auto& appender : _appenders)
        if (appender)
            appender->Flush();

    // Flush all sub processors
    for (auto& processor : _processors)
        if (processor)
            processor->Flush();
}

bool Processor::FilterRecord(Record& record)
{
    // Filter the given logging record
    for (auto& filter : _filters)
        if (filter && !filter->FilterRecord(record))
            return false;

    return true;
}

bool Processor::ProcessRecord(Record& record)
{
    // Filter the given logging record
    if (!FilterRecord(record))
        return false;

    // Layout the given logging record
    if (_layout)
        _layout->LayoutRecord(record);

    // Append the given logging record
    for (auto& appender : _appenders)
        if (appender)
            appender->AppendRecord(record);

    // Process the given logging record with sub processors
    for (auto& processor : _processors)
        if (processor && !processor->ProcessRecord(record))
            return false;

    return true;
}

void Processor::Flush()
{
    // Flush all appenders
    for (auto& appender : _appenders)
        if (appender)
            appender->Flush();

    // Flush all sub processors
    for (auto& processor : _processors)
        if (processor)
            processor->Flush();
}

} // namespace CppLogging
