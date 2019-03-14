/*!
    \file async_wait_free_processor.h
    \brief Asynchronous wait-free logging processor definition
    \author Ivan Shynkarenka
    \date 01.08.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_PROCESSORS_ASYNC_WAIT_FREE_PROCESSOR_H
#define CPPLOGGING_PROCESSORS_ASYNC_WAIT_FREE_PROCESSOR_H

#include "logging/processor.h"

#include "logging/processors/async_wait_free_buffer.h"

#include <cassert>
#include <functional>

namespace CppLogging {

//! Asynchronous wait-free logging processor
/*!
    Asynchronous wait-free logging processor stores the given logging
    record into thread-safe buffer and process it in the separate thread.

    This processor use fixed size async buffer which can overflow.

    Please note that asynchronous logging processor moves the given
    logging record (ProcessRecord() method always returns false)
    into the buffer!

    Thread-safe.
*/
class AsyncWaitFreeProcessor : public Processor
{
public:
    //! Initialize asynchronous processor with a given layout interface, overflow policy and buffer capacity
    /*!
         \param layout - Logging layout interface
         \param discard_on_overflow - Discard logging records on buffer overflow or block and wait (default is false)
         \param capacity - Buffer capacity in logging records (default is 8192)
         \param on_thread_initialize - Thread initialize handler can be used to initialize priority or affinity of the logging thread (default does nothing)
         \param on_thread_clenup - Thread cleanup handler can be used to cleanup priority or affinity of the logging thread (default does nothing)
    */
    explicit AsyncWaitFreeProcessor(const std::shared_ptr<Layout>& layout, bool discard_on_overflow = false, size_t capacity = 8192, const std::function<void ()>& on_thread_initialize = [](){}, const std::function<void ()>& on_thread_clenup = [](){});
    AsyncWaitFreeProcessor(const AsyncWaitFreeProcessor&) = delete;
    AsyncWaitFreeProcessor(AsyncWaitFreeProcessor&&) = default;
    virtual ~AsyncWaitFreeProcessor();

    AsyncWaitFreeProcessor& operator=(const AsyncWaitFreeProcessor&) = delete;
    AsyncWaitFreeProcessor& operator=(AsyncWaitFreeProcessor&&) = default;

    // Implementation of Processor
    bool ProcessRecord(Record& record) override;
    void Flush() override;

private:
    bool _discard_on_overflow;
    AsyncWaitFreeBuffer _buffer;
    std::thread _thread;

    bool EnqueueRecord(bool discard_on_overflow, Record& record);
    void ProcessBufferedRecords(const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup);
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_ASYNC_WAIT_FREE_PROCESSOR_H
