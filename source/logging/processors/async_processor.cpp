/*!
    \file async_processor.cpp
    \brief Asynchronous logging processor definition
    \author Ivan Shynkarenka
    \date 01.08.2016
    \copyright MIT License
*/

#include "logging/processors/async_processor.h"

#include "errors/fatal.h"
#include "threads/thread.h"

namespace CppLogging {

AsyncProcessor::AsyncProcessor(bool discard_on_overflow, size_t capacity, const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup)
    : _discard_on_overflow(discard_on_overflow),
      _buffer(capacity)
{
    // Start processing thread
    _thread = CppCommon::Thread::Start([this, on_thread_initialize, on_thread_clenup]() { ProcessBufferedRecords(on_thread_initialize, on_thread_clenup); });
}

AsyncProcessor::~AsyncProcessor()
{
    // Thread local stop operation record
    thread_local Record stop;

    // Enqueue stop operation record
    stop.timestamp = 0;
    EnqueueRecord(false, stop);

    // Wait for processing thread
    _thread.join();
}

bool AsyncProcessor::ProcessRecord(Record& record)
{
    // Enqueue the given logger record
    return EnqueueRecord(_discard_on_overflow, record);
}

bool AsyncProcessor::EnqueueRecord(bool discard_on_overflow, Record& record)
{
    // Try to enqueue the given logger record
    if (!_buffer.Enqueue(record))
    {
        // If the overflow policy is discard logging record, return immediately
        if (discard_on_overflow)
            return false;

        // If the overflow policy is blocking then yield if the buffer is full
        while (!_buffer.Enqueue(record))
            CppCommon::Thread::Yield();
    }

    return true;
}

void AsyncProcessor::ProcessBufferedRecords(const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup)
{
    // Call the thread initialize handler
    on_thread_initialize();

    try
    {
        // Thread local logger record to process
        thread_local Record record;

        while (true)
        {
            // Dequeue the next logging record or yield if the buffer is empty
            while (!_buffer.Dequeue(record))
                CppCommon::Thread::Yield();

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
        }
    }
    catch (...)
    {
        fatality("Asynchronous logging processor terminated!");
    }

    // Call the thread cleanup handler
    on_thread_clenup();
}

void AsyncProcessor::Flush()
{
    // Thread local flush operation record
    thread_local Record flush;

    // Enqueue flush operation record
    flush.timestamp = 1;
    EnqueueRecord(false, flush);
}

} // namespace CppLogging
