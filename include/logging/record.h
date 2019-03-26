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

#include <cassert>
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

    //! Is the record contains stored format message and its arguments
    bool IsFormatStored() const noexcept { return !buffer.empty(); }

    //! Format message and its arguments
    template <typename... Args>
    Record& Format(std::string_view pattern, const Args&... args);

    //! Store format message and its arguments
    template <typename... Args>
    Record& StoreFormat(std::string_view pattern, const Args&... args);

    //! Store custom format message and its arguments
    template <typename... Args>
    Record& StoreCustomFormat(std::string_view pattern, const Args&... args);

    //! Restore format message and its arguments
    std::string RestoreFormat() { return RestoreFormat(message, buffer, 0, buffer.size()); }

    //! Clear logging record
    void Clear();

    //! Swap two instances
    void swap(Record& record) noexcept;
    friend void swap(Record& record1, Record& record2) noexcept;

private:
    //! Restore format of the custom data type
    static std::string RestoreFormat(std::string_view pattern, const std::vector<uint8_t> buffer, size_t offset, size_t size);
};

} // namespace CppLogging

#include "record.inl"

#endif // CPPLOGGING_RECORD_H
