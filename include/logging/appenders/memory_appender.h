/*!
    \file memory_appender.h
    \brief Memory appender definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_MEMORY_APPENDER_H
#define CPPLOGGING_APPENDERS_MEMORY_APPENDER_H

#include "logging/appender.h"

namespace CppLogging {

//! Memory appender
/*!
    Memory appender collects all given logging records into
    growing memory buffer with the given initial capacity.

    Not thread-safe.
*/
class MemoryAppender : public Appender
{
public:
    //! Initialize the appender with a given capacity
    /*!
         \param capacity - Memory buffer capacity (default is 0)
    */
    explicit MemoryAppender(size_t capacity = 0) : _buffer(capacity) {}
    MemoryAppender(const MemoryAppender&) = delete;
    MemoryAppender(MemoryAppender&&) = delete;
    virtual ~MemoryAppender() = default;

    MemoryAppender& operator=(const MemoryAppender&) = delete;
    MemoryAppender& operator=(MemoryAppender&&) = delete;

    //! Get memory buffer
    std::vector<uint8_t>& buffer() noexcept { return _buffer; }
    //! Get constant memory buffer
    const std::vector<uint8_t>& buffer() const noexcept { return _buffer; }

    // Implementation of Appender
    void AppendRecord(Record& record) override;

private:
    std::vector<uint8_t> _buffer;
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDERS_MEMORY_APPENDER_H
