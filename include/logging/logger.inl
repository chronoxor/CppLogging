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

    // Format or serialize arguments list
    if (format)
        record.Format(message, args...);
    else
        record.Serialize(message, args...);

#if !defined(NDEBUG)
    // Validate format message in debug mode
    assert(record.Validate() && "Invalid format message!");
#endif

    // Process the logging record
    if (_sink)
        _sink->ProcessRecord(record);
}

inline void Logger::Debug(const std::string& debug)
{
#if defined(NDEBUG)
    // Log nothing in release mode...
#else
    Log(Level::DEBUG, debug.c_str());
#endif
}

template <typename... Args>
inline void Logger::Debug(const char* debug, const Args&... args)
{
#if defined(NDEBUG)
    // Log nothing in release mode...
#else
    Log(Level::DEBUG, false, debug, args...);
#endif
}

inline void Logger::Info(const std::string& info)
{
    Log(Level::INFO, info.c_str());
}

template <typename... Args>
inline void Logger::Info(const char* info, const Args&... args)
{
    Log(Level::INFO, false, info, args...);
}

inline void Logger::Warn(const std::string& warn)
{
    Log(Level::WARN, warn.c_str());
}

template <typename... Args>
inline void Logger::Warn(const char* warn, const Args&... args)
{
    Log(Level::WARN, false, warn, args...);
}

inline void Logger::Error(const std::string& error)
{
    Log(Level::ERROR, error.c_str());
}

template <typename... Args>
inline void Logger::Error(const char* error, const Args&... args)
{
    Log(Level::ERROR, false, error, args...);
}

inline void Logger::Fatal(const std::string& fatal)
{
    Log(Level::FATAL, fatal.c_str());
}

template <typename... Args>
inline void Logger::Fatal(const char* fatal, const Args&... args)
{
    Log(Level::FATAL, false, fatal, args...);
}

inline void Logger::Flush()
{
    if (_sink)
        _sink->Flush();
}

} // namespace CppLogging
