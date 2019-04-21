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

    // Stop the logging processor
    if (IsStarted())
        Stop();
}

bool Processor::Start()
{
    if (IsStarted())
        return false;

    _started = true;
    return true;
}

bool Processor::Stop()
{
    if (!IsStarted())
        return false;

    _started = false;
    return true;
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
    // Check if the logging processor started
    if (!IsStarted())
        return true;

    // Filter the given logging record
    if (!FilterRecord(record))
        return true;

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
    // Check if the logging processor started
    if (!IsStarted())
        return;

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
