/*!
    \file record.h
    \brief Logging record definition
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_RECORD_H
#define CPPLOGGING_RECORD_H

#include "logging/level.h"
#include "memory/memory.h"
#include "string/format.h"
#include "threads/thread.h"
#include "time/timestamp.h"

#include <string>
#include <vector>
#include <utility>

namespace CppLogging {

//! Logging record
/*!
    Logging record encapsulates all required fields in a single instance:
    - timestamp
    - thread Id
    - level
    - logger
    - message
    - buffer

    Logging records are created inside Logger class and processed
    by logging appenders, filters and layouts.

    Not thread-safe.
*/
class Record
{
public:
    //! Timestamp of the logging record
    uint64_t timestamp;
    //! Thread Id of the logging record
    uint64_t thread;
    //! Level of the logging record
    Level level;
    //! Logger name of the logging record
    std::string logger;
    //! Message of the logging record
    std::string message;
    //! Buffer of the logging record
    std::vector<uint8_t> buffer;

    //! Record content after layout
    std::vector<uint8_t> raw;

    Record();
    Record(const Record&) = default;
    Record(Record&&) = default;
    ~Record() = default;

    Record& operator=(const Record&) = default;
    Record& operator=(Record&&) = default;

    //! Format message of the logging record
    template <typename... Args>
    void Format(const char* pattern, const Args&... args);

    //! Clear logging record
    void Clear();

    //! Swap two instances
    void swap(Record& record) noexcept;
    friend void swap(Record& record1, Record& record2) noexcept;
};

} // namespace CppLogging

#include "record.inl"

#endif // CPPLOGGING_RECORD_H
