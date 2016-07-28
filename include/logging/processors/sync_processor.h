/*!
    \file sync_processor.h
    \brief Synchronous logging processor definition
    \author Ivan Shynkarenka
    \date 28.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_PROCESSORS_SYNC_PROCESSOR_H
#define CPPLOGGING_PROCESSORS_SYNC_PROCESSOR_H

#include "logging/processor.h"

#include "threads/mutex.h"

namespace CppLogging {

//! Synchronous logging processor
/*!
    Synchronous logging processor process the given logging record
    under with locking a mutex to avoid races in not thread-safe
    layouts, filters and appenders.

    Thread-safe.
*/
class SyncProcessor : public Processor
{
public:
    SyncProcessor() = default;
    SyncProcessor(const SyncProcessor&) = delete;
    SyncProcessor(SyncProcessor&&) = default;
    virtual ~SyncProcessor();

    SyncProcessor& operator=(const SyncProcessor&) = delete;
    SyncProcessor& operator=(SyncProcessor&&) = default;

    // Implementation of Processor
    bool ProcessRecord(Record& record) override;

private:
    CppCommon::Mutex _lock;
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_SYNC_PROCESSOR_H
