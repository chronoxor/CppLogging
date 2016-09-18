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

void Logger::Log(Level level, const char* message, fmt::ArgList args)
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
    record.message = message;

    // Initialize format parameters of the logging record
    record.InitFormat(message, args);

    // Process the logging record
    if (_sink)
        _sink->ProcessRecord(record);
}

void Logger::Update()
{
    _sink = Config::CreateLogger(_name)._sink;
}

} // namespace CppLogging
