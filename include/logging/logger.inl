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
inline void Logger::Debug(const char* debug, const Args&... args)
{
#if defined(NDEBUG)
    // Log nothing in release mode...
#else
    Log(Level::DEBUG, debug, args...);
#endif
}

template <typename... Args>
inline void Logger::Info(const char* info, const Args&... args)
{
    Log(Level::INFO, info, args...);
}

template <typename... Args>
inline void Logger::Warn(const char* warn, const Args&... args)
{
    Log(Level::WARN, warn, args...);
}

template <typename... Args>
inline void Logger::Error(const char* error, const Args&... args)
{
    Log(Level::ERROR, error, args...);
}

template <typename... Args>
inline void Logger::Fatal(const char* fatal, const Args&... args)
{
    Log(Level::FATAL, fatal, args...);
}

inline void Logger::Flush()
{
    if (_sink)
        _sink->Flush();
}

} // namespace CppLogging
