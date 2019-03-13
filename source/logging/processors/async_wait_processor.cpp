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

namespace CppLogging {

AsyncWaitProcessor::AsyncWaitProcessor(const std::shared_ptr<Layout>& layout, const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup)
    : Processor(layout)
{
    // Start processing thread
    _thread = CppCommon::Thread::Start([this, on_thread_initialize, on_thread_clenup]() { ProcessBufferedRecords(on_thread_initialize, on_thread_clenup); });
}

AsyncWaitProcessor::~AsyncWaitProcessor()
{
    // Thread local stop operation record
    thread_local Record stop;

    // Enqueue stop operation record
    stop.timestamp = 0;
    EnqueueRecord(stop);

    // Wait for processing thread
    _thread.join();
}

bool AsyncWaitProcessor::ProcessRecord(Record& record)
{
    // Enqueue the given logger record
    return EnqueueRecord(record);
}

bool AsyncWaitProcessor::EnqueueRecord(Record& record)
{
    // Try to enqueue the given logger record
    while (!_buffer.Enqueue(record))
        CppCommon::Thread::Yield();
    return true;
}

void AsyncWaitProcessor::ProcessBufferedRecords(const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup)
{
    // Call the thread initialize handler
    assert((on_thread_initialize) && "Thread initialize handler must be valid!");
    if (on_thread_initialize)
        on_thread_initialize();

    try
    {
        // Thread local logger record to process
        thread_local std::vector<Record> records;

        while (true)
        {
            // Dequeue the next logging record or yield if the buffer is empty
            if (!_buffer.Dequeue(records))
                return;

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
            }
        }
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
    // Thread local flush operation record
    thread_local Record flush;

    // Enqueue flush operation record
    flush.timestamp = 1;
    EnqueueRecord(flush);
}

} // namespace CppLogging
