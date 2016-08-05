/*!
    \file async_processor.h
    \brief Asynchronous logging processor definition
    \author Ivan Shynkarenka
    \date 01.08.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_PROCESSORS_ASYNC_PROCESSOR_H
#define CPPLOGGING_PROCESSORS_ASYNC_PROCESSOR_H

#include "logging/processor.h"

#include "threads/mpsc_ring_buffer.h"

namespace CppLogging {

//! Asynchronous logging processor
/*!
    Asynchronous logging processor stores the given logging record
    into thread-safe buffer and process it in the separate thread.

    Thread-safe.
*/
class AsyncProcessor : public Processor
{
public:
    //! Initialize asynchronous processor with the given overflow policy, capacity and concurrency
    /*!
         \param discard_on_overflow - Discard logging records on buffer overflow or block and wait (default is false)
         \param capacity - Buffer capacity capacity in bytes (must be a power of two) (default is 16 megabytes)
         \param concurrency - Hardware concurrency (default is std::thread::hardware_concurrency)
    */
    explicit AsyncProcessor(bool discard_on_overflow = false, size_t capacity = 16777216, size_t concurrency = std::thread::hardware_concurrency());
    AsyncProcessor(const AsyncProcessor&) = delete;
    AsyncProcessor(AsyncProcessor&&) = default;
    virtual ~AsyncProcessor();

    AsyncProcessor& operator=(const AsyncProcessor&) = delete;
    AsyncProcessor& operator=(AsyncProcessor&&) = default;

    // Implementation of Processor
    bool ProcessRecord(Record& record) override;
    void Flush() override;

    //! Initialize thread handler
    /*!
         This handler can be used to initialize priority or affinity of the logging thread.
    */
    virtual void OnThreadInitialize() {}
    //! Cleanup thread handler
    /*!
         This handler can be used to cleanup priority or affinity of the logging thread.
    */
    virtual void OnThreadCleanup() {}

private:
    bool _discard_on_overflow;
    CppCommon::MPSCRingBuffer _buffer;
    std::thread _thread;

    bool EnqueueRecord(bool discard_on_overflow, const void* chunk, size_t size);
    void ProcessBufferedRecords();
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_SYNC_PROCESSOR_H
