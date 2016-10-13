/*!
    \file async_buffer.h
    \brief Asynchronous logging buffer definition
    \author Ivan Shynkarenka
    \date 04.08.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_PROCESSORS_ASYNC_BUFFER_H
#define CPPLOGGING_PROCESSORS_ASYNC_BUFFER_H

#include "logging/record.h"

#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <utility>

namespace CppLogging {

//! Asynchronous logging buffer
/*!
    Multiple producers / multiple consumers wait-free ring buffer use only atomic operations to provide thread-safe
    enqueue and dequeue operations. Ring buffer size is limited to the capacity provided in the constructor.

    FIFO order is guaranteed!

    C++ implementation of Dmitry Vyukov's non-intrusive lock free unbound MPSC queue
    http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue
*/
class AsyncBuffer
{
public:
    //! Default class constructor
    /*!
        \param capacity - Ring buffer capacity (must be a power of two)
    */
    explicit AsyncBuffer(size_t capacity);
    AsyncBuffer(const AsyncBuffer&) = delete;
    AsyncBuffer(AsyncBuffer&&) = default;
    ~AsyncBuffer() { delete[] _buffer; }

    AsyncBuffer& operator=(const AsyncBuffer&) = delete;
    AsyncBuffer& operator=(AsyncBuffer&&) = default;

    //! Get ring buffer capacity
    size_t capacity() const noexcept { return _capacity; }
    //! Get ring buffer size
    size_t size() const noexcept;

    //! Enqueue and swap the logging record into the ring buffer (multiple producers threads method)
    /*!
        \param record - Logging record to enqueue and swap
        \return 'true' if the item was successfully enqueue, 'false' if the ring buffer is full
    */
    bool Enqueue(Record& record);

    //! Dequeue and swap the logging record from the ring buffer (multiple consumers threads method)
    /*!
        \param record - Logging record to dequeue and swap
        \return 'true' if the item was successfully dequeue, 'false' if the ring buffer is empty
    */
    bool Dequeue(Record& record);

private:
    struct Node
    {
        Record value;
        std::atomic<size_t> sequence;
    };

    typedef char cache_line_pad[64];

    cache_line_pad _pad0;
    const size_t _capacity;
    const size_t _mask;
    Node* const _buffer;

    cache_line_pad _pad1;
    std::atomic<size_t> _head;

    cache_line_pad _pad2;
    std::atomic<size_t> _tail;
};

} // namespace CppLogging

#include "async_buffer.inl"

#endif // CPPLOGGING_PROCESSORS_ASYNC_BUFFER_H
