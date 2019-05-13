/*!
    \file async_wait_processor.cpp
    \brief Asynchronous wait logging processor implementation
    \author Ivan Shynkarenka
    \date 01.08.2016
    \copyright MIT License
*/

#include "logging/processors/async_wait_processor.h"

#include "errors/fatal.h"
#include "threads/thread.h"

#include <cassert>

namespace CppLogging {

AsyncWaitProcessor::AsyncWaitProcessor(const std::shared_ptr<Layout>& layout, bool auto_start, size_t capacity, size_t initial, const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup)
    : Processor(layout),
      _queue(capacity, initial),
      _on_thread_initialize(on_thread_initialize),
      _on_thread_clenup(on_thread_clenup)
{
    _started = false;

    // Start the logging processor
    if (auto_start)
        Start();
}

AsyncWaitProcessor::~AsyncWaitProcessor()
{
    // Stop the logging processor
    if (IsStarted())
        Stop();
}

bool AsyncWaitProcessor::Start()
{
    if (!Processor::Start())
        return false;

    // Start processing thread
    _thread = CppCommon::Thread::Start([this]() { ProcessThread(_on_thread_initialize, _on_thread_clenup); });

    return true;
}

bool AsyncWaitProcessor::Stop()
{
    if (!IsStarted())
        return false;

    // Thread local stop operation record
    thread_local Record stop;

    // Enqueue stop operation record
    stop.timestamp = 0;
    EnqueueRecord(stop);

    // Wait for processing thread
    _thread.join();

    return Processor::Stop();
}

bool AsyncWaitProcessor::ProcessRecord(Record& record)
{
    // Check if the logging processor started
    if (!IsStarted())
        return true;

    // Enqueue the given logger record
    return EnqueueRecord(record);
}

bool AsyncWaitProcessor::EnqueueRecord(Record& record)
{
    // Try to enqueue the given logger record
    return _queue.Enqueue(record);
}

void AsyncWaitProcessor::ProcessThread(const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup)
{
    // Call the thread initialize handler
    assert((on_thread_initialize) && "Thread initialize handler must be valid!");
    if (on_thread_initialize)
        on_thread_initialize();

    try
    {
        // Thread local logger records to process
        thread_local std::vector<Record> records;
        thread_local uint64_t previous = 0;

        // Reserve initial space for logging records
        records.reserve(_queue.capacity());

        while (_started)
        {
            // Dequeue the next logging record
            if (!_queue.Dequeue(records))
                return;

            // Current timestamp
            uint64_t current = 0;

            // Process all logging records
            for (auto& record : records)
            {
                // Handle stop operation record
                if (record.timestamp == 0)
                    return;

                // Handle flush operation record
                if (record.timestamp == 1)
                {
                    // Flush the logging processor
                    Processor::Flush();
                    continue;
                }

                // Process logging record
                Processor::ProcessRecord(record);

                // Find the latest record timestamp
                if (record.timestamp > current)
                    current = record.timestamp;
            }

            // Handle auto-flush period
            if (CppCommon::Timespan((int64_t)(current - previous)).seconds() > 1)
            {
                // Flush the logging processor
                Processor::Flush();

                // Update the previous timestamp
                previous = current;
            }
        }
    }
    catch (const std::exception& ex)
    {
        fatality(ex);
    }
    catch (...)
    {
        fatality("Asynchronous wait logging processor terminated!");
    }

    // Call the thread cleanup handler
    assert((on_thread_clenup) && "Thread cleanup handler must be valid!");
    if (on_thread_clenup)
        on_thread_clenup();
}

void AsyncWaitProcessor::Flush()
{
    // Check if the logging processor started
    if (!IsStarted())
        return;

    // Thread local flush operation record
    thread_local Record flush;

    // Enqueue flush operation record
    flush.timestamp = 1;
    EnqueueRecord(flush);
}

} // namespace CppLogging
