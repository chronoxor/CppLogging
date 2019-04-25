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
    // Shutdown working logger processors
    Shutdown();
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

    auto it = instance._working.find("");
    if (it != instance._working.end())
        return Logger(it->first, it->second);
    else
    {
        auto sink = std::make_shared<Processor>(std::make_shared<TextLayout>());
        sink->appenders().push_back(std::make_shared<ConsoleAppender>());
        instance._working[""] = sink;
        return Logger("", sink);
    }
}

Logger Config::CreateLogger(const std::string& name)
{
    Config& instance = GetInstance();

    CppCommon::Locker<CppCommon::CriticalSection> locker(instance._lock);

    auto it = instance._working.find(name);
    if (it != instance._working.end())
        return Logger(it->first, it->second);
    else
        return CreateLogger();
}

void Config::Setup()
{
    Config& instance = GetInstance();

    CppCommon::Locker<CppCommon::CriticalSection> locker(instance._lock);

    // Update working logger processors map
    std::swap(instance._working, instance._config);

    // Start all working logger processors
    for (auto& processor : instance._working)
        if (processor.second)
            processor.second->Start();
}

void Config::Shutdown()
{
    Config& instance = GetInstance();

    CppCommon::Locker<CppCommon::CriticalSection> locker(instance._lock);

    // Flush and stop all working logger processors
    for (auto& processor : instance._working)
    {
        if (processor.second)
        {
            processor.second->Flush();
            processor.second->Stop();
        }
    }

    // Clear working logger processors map
    instance._working.clear();
}

} // namespace CppLogging
