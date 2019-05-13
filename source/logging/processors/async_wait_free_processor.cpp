/*!
    \file async_wait_free_processor.cpp
    \brief Asynchronous wait-free logging processor implementation
    \author Ivan Shynkarenka
    \date 01.08.2016
    \copyright MIT License
*/

#include "logging/processors/async_wait_free_processor.h"

#include "errors/fatal.h"
#include "threads/thread.h"

#include <cassert>

namespace CppLogging {

AsyncWaitFreeProcessor::AsyncWaitFreeProcessor(const std::shared_ptr<Layout>& layout, bool auto_start, size_t capacity, bool discard, const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup)
    : Processor(layout),
      _discard(discard),
      _queue(capacity),
      _on_thread_initialize(on_thread_initialize),
      _on_thread_clenup(on_thread_clenup)
{
    _started = false;

    // Start the logging processor
    if (auto_start)
        Start();
}

AsyncWaitFreeProcessor::~AsyncWaitFreeProcessor()
{
    // Stop the logging processor
    if (IsStarted())
        Stop();
}

bool AsyncWaitFreeProcessor::Start()
{
    if (!Processor::Start())
        return false;

    // Start processing thread
    _thread = CppCommon::Thread::Start([this]() { ProcessThread(_on_thread_initialize, _on_thread_clenup); });

    return true;
}

bool AsyncWaitFreeProcessor::Stop()
{
    if (!IsStarted())
        return false;

    // Thread local stop operation record
    thread_local Record stop;

    // Enqueue stop operation record
    stop.timestamp = 0;
    EnqueueRecord(false, stop);

    // Wait for processing thread
    _thread.join();

    return Processor::Stop();
}

bool AsyncWaitFreeProcessor::ProcessRecord(Record& record)
{
    // Check if the logging processor started
    if (!IsStarted())
        return true;

    // Enqueue the given logger record
    return EnqueueRecord(_discard, record);
}

bool AsyncWaitFreeProcessor::EnqueueRecord(bool discard, Record& record)
{
    // Try to enqueue the given logger record
    if (!_queue.Enqueue(record))
    {
        // If the overflow policy is discard logging record, return immediately
        if (discard)
            return false;

        // If the overflow policy is blocking then yield if the queue is full
        while (!_queue.Enqueue(record))
            CppCommon::Thread::Yield();
    }

    return true;
}

void AsyncWaitFreeProcessor::ProcessThread(const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup)
{
    // Call the thread initialize handler
    assert((on_thread_initialize) && "Thread initialize handler must be valid!");
    if (on_thread_initialize)
        on_thread_initialize();

    try
    {
        // Thread local logger record to process
        thread_local Record record;
        thread_local uint64_t previous = CppCommon::Timestamp::utc();

        while (_started)
        {
            // Try to dequeue the next logging record
            bool empty = !_queue.Dequeue(record);

            // Current timestamp
            uint64_t current;

            if (!empty)
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

                // Update the current timestamp
                current = record.timestamp;
            }
            else
            {
                // Update the current timestamp
                current = CppCommon::Timestamp::utc();
            }

            // Handle auto-flush period
            if (CppCommon::Timespan((int64_t)(current - previous)).seconds() > 1)
            {
                // Flush the logging processor
                Processor::Flush();

                // Update the previous timestamp
                previous = current;
            }

            // Sleep for a while if the queue was empty
            if (empty)
                CppCommon::Thread::Sleep(100);
        }
    }
    catch (const std::exception& ex)
    {
        fatality(ex);
    }
    catch (...)
    {
        fatality("Asynchronous wait-free logging processor terminated!");
    }

    // Call the thread cleanup handler
    assert((on_thread_clenup) && "Thread cleanup handler must be valid!");
    if (on_thread_clenup)
        on_thread_clenup();
}

void AsyncWaitFreeProcessor::Flush()
{
    // Check if the logging processor started
    if (!IsStarted())
        return;

    // Thread local flush operation record
    thread_local Record flush;

    // Enqueue flush operation record
    flush.timestamp = 1;
    EnqueueRecord(false, flush);
}

} // namespace CppLogging
