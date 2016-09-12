/*!
    \file config.cpp
    \brief Logger configuration implementation
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

#include "logging/config.h"

namespace CppLogging {

Config::~Config()
{
    CppCommon::Locker<CppCommon::CriticalSection> locker(_lock);

    for (auto& processor : _config)
        if (processor.second)
            processor.second->Flush();
}

void Config::ConfigLogger(const std::shared_ptr<Processor>& sink)
{
    Config& instance = GetInstance();

    CppCommon::Locker<CppCommon::CriticalSection> locker(instance._lock);

    instance._config[""] = sink;
}

void Config::ConfigLogger(const std::string& name, const std::shared_ptr<Processor>& sink)
{
    Config& instance = GetInstance();

    CppCommon::Locker<CppCommon::CriticalSection> locker(instance._lock);

    instance._config[name] = sink;
}

Logger Config::CreateLogger()
{
    Config& instance = GetInstance();

    CppCommon::Locker<CppCommon::CriticalSection> locker(instance._lock);

    auto it = instance._config.find("");
    if (it != instance._config.end())
        return Logger(it->first, it->second);
    else
    {
        auto sink = std::make_shared<Processor>();
        sink->layouts().push_back(std::make_shared<TextLayout>());
        sink->appenders().push_back(std::make_shared<ConsoleAppender>());
        instance._config[""] = sink;
        return Logger("", sink);
    }
}

Logger Config::CreateLogger(const std::string& name)
{
    Config& instance = GetInstance();

    CppCommon::Locker<CppCommon::CriticalSection> locker(instance._lock);

    auto it = instance._config.find(name);
    if (it != instance._config.end())
        return Logger(it->first, it->second);
    else
        return CreateLogger();
}

} // namespace CppLogging
