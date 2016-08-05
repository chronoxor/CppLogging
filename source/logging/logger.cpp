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

void Logger::Log(Level level, const char* message)
{
    // Create a new logging record
    Record record;
    record.level = level;
    record.logger = std::make_pair(_name.c_str(), (uint8_t)_name.size());
    record.message = std::make_pair(message, (uint16_t)std::strlen(message));

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
