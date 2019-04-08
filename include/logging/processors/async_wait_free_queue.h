/*!
    \file async_wait_free_queue.h
    \brief Asynchronous wait-free logging ring queue definition
    \author Ivan Shynkarenka
    \date 04.08.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_PROCESSORS_ASYNC_WAIT_FREE_QUEUE_H
#define CPPLOGGING_PROCESSORS_ASYNC_WAIT_FREE_QUEUE_H

#include "logging/record.h"

#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <utility>

namespace CppLogging {

//! Asynchronous wait-free logging ring queue
/*!
    Multiple producers / multiple consumers wait-free ring queue use only atomic operations to provide thread-safe
    enqueue and dequeue operations. Ring queue size is limited to the capacity provided in the constructor.

    FIFO order is guaranteed!

    Thread-safe.

    C++ implementation of Dmitry Vyukov's non-intrusive lock free unbound MPSC queue
    http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
*/
template<typename T>
class AsyncWaitFreeQueue
{
public:
    //! Default class constructor
    /*!
        \param capacity - Ring queue capacity (must be a power of two)
    */
    explicit AsyncWaitFreeQueue(size_t capacity);
    AsyncWaitFreeQueue(const AsyncWaitFreeQueue&) = delete;
    AsyncWaitFreeQueue(AsyncWaitFreeQueue&&) = delete;
    ~AsyncWaitFreeQueue() { delete[] _buffer; }

    AsyncWaitFreeQueue& operator=(const AsyncWaitFreeQueue&) = delete;
    AsyncWaitFreeQueue& operator=(AsyncWaitFreeQueue&&) = delete;

    //! Check if the queue is not empty
    explicit operator bool() const noexcept { return !empty(); }

    //! Is ring queue empty?
    bool empty() const noexcept { return (size() == 0); }
    //! Get ring queue capacity
    size_t capacity() const noexcept { return _capacity; }
    //! Get ring queue size
    size_t size() const noexcept;

    //! Enqueue and swap the logging record into the ring queue (multiple producers threads method)
    /*!
        \param record - Logging record to enqueue and swap
        \return 'true' if the item was successfully enqueue, 'false' if the ring queue is full
    */
    bool Enqueue(Record& record);

    //! Dequeue and swap the logging record from the ring queue (multiple consumers threads method)
    /*!
        \param record - Logging record to dequeue and swap
        \return 'true' if the item was successfully dequeue, 'false' if the ring queue is empty
    */
    bool Dequeue(Record& record);

private:
    struct Node
    {
        std::atomic<size_t> sequence;
        Record value;
    };

    typedef char cache_line_pad[128];

    cache_line_pad _pad0;
    const size_t _capacity;
    const size_t _mask;
    Node* const _buffer;

    cache_line_pad _pad1;
    std::atomic<size_t> _head;
    cache_line_pad _pad2;
    std::atomic<size_t> _tail;
    cache_line_pad _pad3;
};

} // namespace CppLogging

#include "async_wait_free_queue.inl"

#endif // CPPLOGGING_PROCESSORS_ASYNC_WAIT_FREE_QUEUE_H
