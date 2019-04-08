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

#include "threads/critical_section.h"

namespace CppLogging {

//! Synchronous logging processor
/*!
    Synchronous logging processor process the given logging record
    under the critical section to avoid races in not thread-safe
    layouts, filters and appenders.

    Thread-safe.
*/
class SyncProcessor : public Processor
{
public:
    //! Initialize synchronous logging processor with a given layout interface
    /*!
         \param layout - Logging layout interface
    */
    explicit SyncProcessor(const std::shared_ptr<Layout>& layout) : Processor(layout) {}
    SyncProcessor(const SyncProcessor&) = delete;
    SyncProcessor(SyncProcessor&&) = delete;
    virtual ~SyncProcessor() = default;

    SyncProcessor& operator=(const SyncProcessor&) = delete;
    SyncProcessor& operator=(SyncProcessor&&) = delete;

    // Implementation of Processor
    bool ProcessRecord(Record& record) override;
    void Flush() override;

private:
    CppCommon::CriticalSection _lock;
};

} // namespace CppLogging

/*! \example sync.cpp Synchronous logger processor example */

#endif // CPPLOGGING_PROCESSORS_SYNC_PROCESSOR_H
