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

template <typename... T>
inline void Logger::Log(Level level, bool format, fmt::format_string<T...> message, T&&... args) const
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

    // Check for valid and started logging sink
    if (_sink && _sink->IsStarted())
    {
        // Filter the logging record
        if (!_sink->FilterRecord(record))
            return;

        // Format or serialize arguments list
        if (format)
            record.Format(message, std::forward<T>(args)...);
        else
            record.StoreFormat(message, std::forward<T>(args)...);

        // Process the logging record
        _sink->ProcessRecord(record);
    }
}

template <typename... T>
inline void Logger::Debug(fmt::format_string<T...> message, T&&... args) const
{
#if defined(NDEBUG)
    // Log nothing in release mode...
#else
    Log(Level::DEBUG, false, message, std::forward<T>(args)...);
#endif
}

template <typename... T>
inline void Logger::Info(fmt::format_string<T...> message, T&&... args) const
{
    Log(Level::INFO, false, message, std::forward<T>(args)...);
}

template <typename... T>
inline void Logger::Warn(fmt::format_string<T...> message, T&&... args) const
{
    Log(Level::WARN, false, message, std::forward<T>(args)...);
}

template <typename... T>
inline void Logger::Error(fmt::format_string<T...> message, T&&... args) const
{
    Log(Level::ERROR, false, message, std::forward<T>(args)...);
}

template <typename... T>
inline void Logger::Fatal(fmt::format_string<T...> message, T&&... args) const
{
    Log(Level::FATAL, false, message, std::forward<T>(args)...);
}

inline void Logger::Flush()
{
    if (_sink && _sink->IsStarted())
        _sink->Flush();
}

} // namespace CppLogging
