/*!
    \file config.cpp
    \brief Logger configuration implementation
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

#include "logging/config.h"

namespace CppLogging {

CppCommon::Mutex Config::_lock;
std::map<std::string, std::shared_ptr<Processor>> Config::_config;

void Config::ConfigLogger(const std::shared_ptr<Processor>& sink)
{
    CppCommon::Locker<CppCommon::Mutex> locker(_lock);

    _config[""] = sink;
}

void Config::ConfigLogger(const std::string& name, const std::shared_ptr<Processor>& sink)
{
    CppCommon::Locker<CppCommon::Mutex> locker(_lock);

    _config[name] = sink;
}

Logger Config::CreateLogger()
{
    CppCommon::Locker<CppCommon::Mutex> locker(_lock);

    auto it = _config.find("");
    if (it != _config.end())
        return Logger(it->first, it->second);
    else
    {
        auto sink = std::make_shared<Processor>();
        sink->layouts().push_back(std::make_shared<TextLayout>());
        sink->appenders().push_back(std::make_shared<ConsoleAppender>());
        _config[""] = sink;
        return Logger("", sink);
    }
}

Logger Config::CreateLogger(const std::string& name)
{
    CppCommon::Locker<CppCommon::Mutex> locker(_lock);

    auto it = _config.find(name);
    if (it != _config.end())
        return Logger(it->first, it->second);
    else
        return CreateLogger();
}

} // namespace CppLogging
