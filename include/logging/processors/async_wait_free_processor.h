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

#include "logging/processors/async_wait_free_queue.h"

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
         \param auto_start - Auto-start the logging processor (default is true)
         \param capacity - Buffer capacity in logging records (default is 8192)
         \param discard - Discard logging records on buffer overflow or block and wait (default is false)
         \param on_thread_initialize - Thread initialize handler can be used to initialize priority or affinity of the logging thread (default does nothing)
         \param on_thread_clenup - Thread cleanup handler can be used to cleanup priority or affinity of the logging thread (default does nothing)
    */
    explicit AsyncWaitFreeProcessor(const std::shared_ptr<Layout>& layout, bool auto_start = true, size_t capacity = 8192, bool discard = false, const std::function<void ()>& on_thread_initialize = [](){}, const std::function<void ()>& on_thread_clenup = [](){});
    AsyncWaitFreeProcessor(const AsyncWaitFreeProcessor&) = delete;
    AsyncWaitFreeProcessor(AsyncWaitFreeProcessor&&) = delete;
    virtual ~AsyncWaitFreeProcessor();

    AsyncWaitFreeProcessor& operator=(const AsyncWaitFreeProcessor&) = delete;
    AsyncWaitFreeProcessor& operator=(AsyncWaitFreeProcessor&&) = delete;

    // Implementation of Processor
    bool Start() override;
    bool Stop() override;
    bool ProcessRecord(Record& record) override;
    void Flush() override;

private:
    bool _discard;
    AsyncWaitFreeQueue<Record> _queue;
    std::thread _thread;
    std::function<void ()> _on_thread_initialize;
    std::function<void ()> _on_thread_clenup;

    bool EnqueueRecord(bool discard, Record& record);
    void ProcessThread(const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup);
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_ASYNC_WAIT_FREE_PROCESSOR_H
