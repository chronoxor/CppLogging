/*!
    \file logger.cpp
    \brief Logger interface implementation
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

#include "logging/logger.h"

namespace CppLogging {

Logger::Logger(const std::string& name) : _name(name)
{

}

void Logger::Log(Record& record)
{
    _sink->ProcessRecord(record);
}

void Logger::Log(Level level, const std::string& message)
{
    Record record;
    record.level = level;
    record.logger = std::make_pair(_name.c_str(), (uint8_t)_name.size());
    record.message = std::make_pair(message.c_str(), (uint16_t)message.size());
    Log(record);
}

void Logger::Debug(const std::string& debug)
{
#if defined(NDEBUG)
    // Log nothing in release mode...
#else
    Log(Level::DEBUG, debug);
#endif
}

void Logger::Info(const std::string& info)
{
    Log(Level::INFO, info);
}

void Logger::Warn(const std::string& warn)
{
    Log(Level::WARN, warn);
}

void Logger::Error(const std::string& error)
{
    Log(Level::ERROR, error);
}

void Logger::Fatal(const std::string& fatal)
{
    Log(Level::FATAL, fatal);
}

void Logger::Update()
{

}

} // namespace CppLogging
