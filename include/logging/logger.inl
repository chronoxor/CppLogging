/*!
    \file logger.inl
    \brief Logger interface inline implementation
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

namespace CppLogging {

inline Logger::Logger(const std::string& name, const std::shared_ptr<Processor>& sink) : _name(name), _sink(sink)
{
}

inline Logger::~Logger()
{
    Flush();
}

template <typename... Args>
inline void Logger::Log(Level level, bool format, const char* message, const Args&... args)
{
    // Thread local thread Id
    thread_local uint64_t thread = CppCommon::Thread::CurrentThreadId();
    // Thread local instance of the logging record
    thread_local Record record;

    // Clear the logging record
    record.Clear();

    // Fill necessary fields of the logging record
    record.timestamp = CppCommon::Timestamp::utc();
    record.thread = thread;
    record.level = level;
    record.logger = _name;

    // Filter the logging record
    if (_sink)
        if (!_sink->FilterRecord(record))
            return;

    // Format arguments list
    if (format)
        record.Format(message, args...);
    else
        record.message = message;

    // Process the logging record
    if (_sink)
        _sink->ProcessRecord(record);
}

inline void Logger::Debug(const std::string& debug)
{
#if defined(NDEBUG)
    // Log nothing in release mode...
#else
    Log(Level::DEBUG, false, debug.c_str());
#endif
}

template <typename... Args>
inline void Logger::Debug(const char* debug, const Args&... args)
{
#if defined(NDEBUG)
    // Log nothing in release mode...
#else
    Log(Level::DEBUG, true, debug, args...);
#endif
}

inline void Logger::Info(const std::string& info)
{
    Log(Level::INFO, false, info.c_str());
}

template <typename... Args>
inline void Logger::Info(const char* info, const Args&... args)
{
    Log(Level::INFO, true, info, args...);
}

inline void Logger::Warn(const std::string& warn)
{
    Log(Level::WARN, false, warn.c_str());
}

template <typename... Args>
inline void Logger::Warn(const char* warn, const Args&... args)
{
    Log(Level::WARN, true, warn, args...);
}

inline void Logger::Error(const std::string& error)
{
    Log(Level::ERROR, false, error.c_str());
}

template <typename... Args>
inline void Logger::Error(const char* error, const Args&... args)
{
    Log(Level::ERROR, true, error, args...);
}

inline void Logger::Fatal(const std::string& fatal)
{
    Log(Level::FATAL, false, fatal.c_str());
}

template <typename... Args>
inline void Logger::Fatal(const char* fatal, const Args&... args)
{
    Log(Level::FATAL, true, fatal, args...);
}

inline void Logger::Flush()
{
    if (_sink)
        _sink->Flush();
}

} // namespace CppLogging
