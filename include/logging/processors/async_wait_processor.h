/*!
    \file async_wait_processor.h
    \brief Asynchronous wait logging processor definition
    \author Ivan Shynkarenka
    \date 01.08.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_PROCESSORS_ASYNC_WAIT_PROCESSOR_H
#define CPPLOGGING_PROCESSORS_ASYNC_WAIT_PROCESSOR_H

#include "logging/processor.h"

#include "threads/wait_batcher.h"

#include <functional>

namespace CppLogging {

//! Asynchronous wait logging processor
/*!
    Asynchronous wait logging processor stores the given logging
    record into thread-safe buffer and process it in the separate
    thread.

    This processor use dynamic size async buffer which cannot overflow,
    buy might lead to out of memory error.

    Please note that asynchronous logging processor moves the given
    logging record (ProcessRecord() method always returns false)
    into the buffer!

    Thread-safe.
*/
class AsyncWaitProcessor : public Processor
{
public:
    //! Initialize asynchronous processor with a given layout interface
    /*!
         \param layout - Logging layout interface
         \param auto_start - Auto-start the logging processor (default is true)
         \param capacity - Buffer capacity in logging records (0 for unlimited capacity, default is 8192)
         \param initial - Buffer initial capacity in logging records (default is 8192)
         \param on_thread_initialize - Thread initialize handler can be used to initialize priority or affinity of the logging thread (default does nothing)
         \param on_thread_clenup - Thread cleanup handler can be used to cleanup priority or affinity of the logging thread (default does nothing)
    */
    explicit AsyncWaitProcessor(const std::shared_ptr<Layout>& layout, bool auto_start = true, size_t capacity = 8192, size_t initial = 8192, const std::function<void ()>& on_thread_initialize = [](){}, const std::function<void ()>& on_thread_clenup = [](){});
    AsyncWaitProcessor(const AsyncWaitProcessor&) = delete;
    AsyncWaitProcessor(AsyncWaitProcessor&&) = delete;
    virtual ~AsyncWaitProcessor();

    AsyncWaitProcessor& operator=(const AsyncWaitProcessor&) = delete;
    AsyncWaitProcessor& operator=(AsyncWaitProcessor&&) = delete;

    // Implementation of Processor
    bool Start() override;
    bool Stop() override;
    bool ProcessRecord(Record& record) override;
    void Flush() override;

private:
    CppCommon::WaitBatcher<Record> _queue;
    std::thread _thread;
    std::function<void ()> _on_thread_initialize;
    std::function<void ()> _on_thread_clenup;

    bool EnqueueRecord(Record& record);
    void ProcessThread(const std::function<void ()>& on_thread_initialize, const std::function<void ()>& on_thread_clenup);
};

} // namespace CppLogging

/*! \example async.cpp Asynchronous logger processor example */

#endif // CPPLOGGING_PROCESSORS_ASYNC_WAIT_PROCESSOR_H
