/*!
    \file buffered_processor.h
    \brief Buffered logging processor definition
    \author Ivan Shynkarenka
    \date 28.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_PROCESSORS_BUFFERED_PROCESSOR_H
#define CPPLOGGING_PROCESSORS_BUFFERED_PROCESSOR_H

#include "logging/processor.h"

namespace CppLogging {

//! Buffered logging processor
/*!
    Buffered logging processor stores all logging records in the
    limited size buffer until Flush() method is invoked or buffer
    has not enough space.

    Please note that buffered logging processor moves the given
    logging record (ProcessRecord() method always returns false)
    into the buffer!

    Not thread-safe.
*/
class BufferedProcessor : public Processor
{
public:
    //! Initialize buffered processor with the given limit and capacity
    /*!
         \param limit - Buffer limit in logging records (default is 65536)
         \param capacity - Buffer initial capacity in logging records (default is 4096)
    */
    explicit BufferedProcessor(size_t limit = 65536, size_t capacity = 4096) : _limit(limit)
    { _buffer.reserve(capacity); }
    BufferedProcessor(const BufferedProcessor&) = delete;
    BufferedProcessor(BufferedProcessor&&) noexcept = default;
    virtual ~BufferedProcessor() = default;

    BufferedProcessor& operator=(const BufferedProcessor&) = delete;
    BufferedProcessor& operator=(BufferedProcessor&&) noexcept = default;

    // Implementation of Processor
    bool ProcessRecord(Record& record) override;
    void Flush() override;

private:
    size_t _limit;
    std::vector<Record> _buffer;

    void ProcessBufferedRecords();
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_BUFFERED_PROCESSOR_H
