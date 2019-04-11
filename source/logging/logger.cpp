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

Logger::Logger() : _sink(Config::CreateLogger()._sink)
{
}

Logger::Logger(const std::string& name) : _name(name), _sink(Config::CreateLogger(name)._sink)
{
}

void Logger::Update()
{
    _sink = Config::CreateLogger(_name)._sink;
}

} // namespace CppLogging
