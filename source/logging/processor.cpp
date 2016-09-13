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
		appender->Flush();

	// Flush all sub processors
	for (auto& processor : _processors)
		processor->Flush();
}

bool Processor::ProcessRecord(Record& record)
{
    // Filter the given logging record
    for (auto& filter : _filters)
        if (!filter->FilterRecord(record))
            return false;

    // Layout the given logging record
    for (auto& layout : _layouts)
        layout->LayoutRecord(record);

    // Append the given logging record
    for (auto& appender : _appenders)
        appender->AppendRecord(record);

    // Process the given logging record with sub processors
    for (auto& processor : _processors)
        if (!processor->ProcessRecord(record))
            return false;

    return true;
}

void Processor::Flush()
{
    // Flush all appenders
    for (auto& appender : _appenders)
        appender->Flush();

    // Flush all sub processors
    for (auto& processor : _processors)
        processor->Flush();
}

} // namespace CppLogging
