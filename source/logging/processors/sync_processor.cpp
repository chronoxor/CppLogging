/*!
    \file sync_processor.cpp
    \brief Synchronous logging processor implementation
    \author Ivan Shynkarenka
    \date 28.07.2016
    \copyright MIT License
*/

#include "logging/processors/sync_processor.h"

namespace CppLogging {

bool SyncProcessor::ProcessRecord(Record& record)
{
    CppCommon::Locker<CppCommon::CriticalSection> locker(_lock);

    // Process the given logging record under the critical section lock
    return Processor::ProcessRecord(record);
}

void SyncProcessor::Flush()
{
    CppCommon::Locker<CppCommon::CriticalSection> locker(_lock);

    // Flush under the critical section lock
    return Processor::Flush();
}

} // namespace CppLogging
