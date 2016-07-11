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
#include "threads/thread.h"
#include "time/timestamp.h"

#include <utility>

namespace CppLogging {

//! Logging record
/*!
    Encapsulates all required fields of a single logging record:
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
    std::pair<char*, uint8_t>  logger;
    //! Message of the logging record
    std::pair<char*, uint16_t> message;
    //! Buffer of the logging record
    std::pair<void*, uint32_t> buffer;

    Record() noexcept
        : timestamp(CppCommon::timestamp()),
          thread(CppCommon::Thread::CurrentThreadId()),
          level(Level::INFO),
          logger(nullptr, 0),
          message(nullptr, 0),
          buffer(nullptr, 0)
    {}
    Record(const Record&) noexcept = default;
    Record(Record&&) noexcept = default;
    ~Record() noexcept = default;

    Record& operator=(const Record&) noexcept = default;
    Record& operator=(Record&&) noexcept = default;
};

} // namespace CppLogging

#endif // CPPLOGGING_RECORD_H
