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

#include "logging/processors/async_buffer.h"

#include <functional>

namespace CppLogging {

//! Asynchronous logging processor
/*!
    Asynchronous logging processor stores the given logging record
    into thread-safe buffer and process it in the separate thread.

    Please note that asynchronous logging processor moves the given
    logging record (ProcessRecord() method always returns false)
    into the buffer!

    Thread-safe.
*/
class AsyncProcessor : public Processor
{
public:
    //! Initialize asynchronous processor with the given overflow policy and buffer capacity
    /*!
         \param discard_on_overflow - Discard logging records on buffer overflow or block and wait (default is false)
         \param capacity - Buffer capacity in logging records (default is 4096)
         \param on_thread_initialize - Thread initialize handler can be used to initialize priority or affinity of the logging thread (default does nothing)
         \param on_thread_clenup - Thread initialize handler can be used to cleanup priority or affinity of the logging thread (default does nothing)
    */
    explicit AsyncProcessor(bool discard_on_overflow = false, size_t capacity = 4096, const std::function<void ()>& on_thread_initialize = [](){}, const std::function<void ()>& on_thread_clenup = [](){});
    AsyncProcessor(const AsyncProcessor&) = delete;
    AsyncProcessor(AsyncProcessor&&) = default;
    virtual ~AsyncProcessor();

    AsyncProcessor& operator=(const AsyncProcessor&) = delete;
    AsyncProcessor& operator=(AsyncProcessor&&) = default;

    // Implementation of Processor
    bool ProcessRecord(Record& record) override;
    void Flush() override;

private:
    bool _discard_on_overflow;
    AsyncBuffer _buffer;
    std::thread _thread;

    bool EnqueueRecord(bool discard_on_overflow, Record& record);
    void ProcessBufferedRecords(const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup);
};

} // namespace CppLogging

/*! \example async.cpp Asynchronous logger processor example */

#endif // CPPLOGGING_PROCESSORS_SYNC_PROCESSOR_H
