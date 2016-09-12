/*!
    \file logger.cpp
    \brief Logger interface implementation
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

#include "logging/logger.h"

#include "logging/config.h"

namespace CppLogging {

Logger::Logger() : _name(), _sink(Config::CreateLogger()._sink)
{
}

Logger::Logger(const std::string& name) : _name(name), _sink(Config::CreateLogger(name)._sink)
{
}

Logger::Logger(const std::string& name, const std::shared_ptr<Processor>& sink) : _name(name), _sink(sink)
{
}

Logger::~Logger()
{
    Flush();
}

void Logger::Log(Level level, const char* message)
{
    // Thread local instance of the logging record
    thread_local Record record;

    // Fill necessary fields of the logging record
    record.timestamp = CppCommon::Timestamp::utc();
    record.level = level;
    record.logger = _name;
    record.message = message;

    // Clear buffer filed and raw field of the logging record
    record.buffer.clear();
    record.raw.clear();

    // Process the logging record
    if (_sink)
        _sink->ProcessRecord(record);
}

void Logger::Debug(const char* debug)
{
#if defined(NDEBUG)
    // Log nothing in release mode...
#else
    Log(Level::DEBUG, debug);
#endif
}

void Logger::Info(const char* info)
{
    Log(Level::INFO, info);
}

void Logger::Warn(const char* warn)
{
    Log(Level::WARN, warn);
}

void Logger::Error(const char* error)
{
    Log(Level::ERROR, error);
}

void Logger::Fatal(const char* fatal)
{
    Log(Level::FATAL, fatal);
}

void Logger::Flush()
{
    if (_sink)
        _sink->Flush();
}

void Logger::Update()
{
    _sink = Config::CreateLogger(_name)._sink;
}

} // namespace CppLogging
