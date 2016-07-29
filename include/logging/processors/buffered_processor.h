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
    fixed size buffer until Flush() method is invoked or buffer
    has not enough space.

    Not thread-safe.
*/
class BufferedProcessor : public Processor
{
public:
    //! Initialize buffered processor with given size limit and capacity values
    /*!
         \param limit - Buffer size limit in bytes (default is 100 megabytes)
         \param capacity - Buffer initial capacity in bytes (default is 10 megabytes)
    */
    explicit BufferedProcessor(size_t limit = 104857600, size_t capacity = 10485760)
        : _limit(limit),
          _buffer(capacity)
    {}
    BufferedProcessor(const BufferedProcessor&) = delete;
    BufferedProcessor(BufferedProcessor&&) = default;
    virtual ~BufferedProcessor() { Flush(); }

    BufferedProcessor& operator=(const BufferedProcessor&) = delete;
    BufferedProcessor& operator=(BufferedProcessor&&) = default;

    // Implementation of Processor
    bool ProcessRecord(Record& record) override;
    void Flush() override;

private:
    size_t _limit;
    std::vector<uint8_t> _buffer;

    void ProcessBufferedRecords();
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_BUFFERED_PROCESSOR_H
