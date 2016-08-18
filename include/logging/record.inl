/*!
    \file record.inl
    \brief Logging record inline implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

namespace CppLogging {

inline Record::Record()
    : timestamp(CppCommon::Timestamp::utc()),
      thread(CppCommon::Thread::CurrentThreadId()),
      level(Level::INFO)
{
    logger.reserve(32);
    message.reserve(512);
    buffer.reserve(1024);
    raw.reserve(512);
}

inline void swap(Record& record1, Record& record2) noexcept
{
    using std::swap;
    swap(record1.timestamp, record2.timestamp);
    swap(record1.thread, record2.thread);
    swap(record1.level, record2.level);
    swap(record1.logger, record2.logger);
    swap(record1.message, record2.message);
    swap(record1.buffer, record2.buffer);
    swap(record1.raw, record2.raw);
}

} // namespace CppLogging
