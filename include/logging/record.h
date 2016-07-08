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
    Record() = default;
    Record(const Record&) = default;
    Record(Record&&) = default;
    ~Record() = default;

    Record& operator=(const Record&) = default;
    Record& operator=(Record&&) = default;

    //! Get the timestamp of the logging record
    uint64_t timestamp() const { return _timestamp; }
    //! Get the parent thread Id of the logging record
    uint64_t thread() const { return _thread; }
    //! Get the level of the logging record
    Level level() const { return _level; }
    //! Get the parent logger of the logging record
    std::pair<char*, size_t> logger() const { return _logger; }
    //! Get the message of the logging record
    std::pair<char*, size_t> message() const { return _message; }
    //! Get the buffer of the logging record
    std::pair<void*, size_t> buffer() const { return _buffer; }

    //! Get the logging record size
    size_t size() const { return sizeof(uint64_t) + sizeof(uint64_t) + sizeof(Level) + _logger.second + _message.second + _buffer.second; }

    //! Set the timestamp of the logging record
    /*!
         \param timestamp - Timestamp of the logging record
    */
    void SetTimestamp(uint64_t timestamp) { _timestamp = timestamp; }
    //! Set the parent thread Id of the logging record
    /*!
         \param thread - Parent thread Id of the logging record
    */
    void SetThread(uint64_t thread) { _thread = thread; }
    //! Set the level of the logging record
    /*!
         \param level - Level of the logging record
    */
    void SetLevel(Level level) { _level = level; }
    //! Set the parent logger of the logging record
    /*!
         \param logger - Parent logger of the logging record
    */
    void SetLogger(const std::pair<char*, size_t>& logger) { _logger = logger; }
    //! Set the message of the logging record
    /*!
         \param message - Message of the logging record
    */
    void SetMessage(const std::pair<char*, size_t>& message) { _message = message; }
    //! Set the buffer of the logging record
    /*!
         \param buffer - Buffer of the logging record
    */
    void SetBuffer(const std::pair<void*, size_t>& buffer) { _buffer = buffer; }

private:
    uint64_t _timestamp;
    uint64_t _thread;
    Level _level;
    std::pair<char*, size_t> _logger;
    std::pair<char*, size_t> _message;
    std::pair<void*, size_t> _buffer;
};

} // namespace CppLogging

#endif // CPPLOGGING_RECORD_H
