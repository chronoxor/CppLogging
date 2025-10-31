#include "logging/processors/dynamic_router.h"
#include <algorithm>
#include <sstream>

namespace CppLogging {

DynamicRouter::DynamicRouter()
    : Processor(nullptr)
{
}

DynamicRouter::DynamicRouter(const std::shared_ptr<Layout>& layout)
    : Processor(layout)
{
}

bool DynamicRouter::ProcessRecord(Record& record)
{
    // 先处理自己的filters
    if (!FilterRecord(record))
        return false;

    // 应用自己的layout
    if (_layout)
        _layout->LayoutRecord(record);

    // 提取目标信息
    std::vector<std::string> targets = ExtractTargets(record);

    // 路由到目标appenders和processors
    {
        std::lock_guard<std::mutex> lock(_mutex);

        for (const auto& target : targets)
        {
            // 路由到appenders
            auto appender_it = _appender_routes.find(target);
            if (appender_it != _appender_routes.end())
            {
                for (const auto& appender : appender_it->second)
                {
                    if (appender->IsStarted())
                        appender->AppendRecord(record);
                }
            }

            // 路由到processors
            auto processor_it = _processor_routes.find(target);
            if (processor_it != _processor_routes.end())
            {
                for (const auto& processor : processor_it->second)
                {
                    if (processor->IsStarted())
                        processor->ProcessRecord(record);
                }
            }
        }
    }

    // 处理自己的appenders
    for (auto& appender : _appenders)
    {
        if (appender->IsStarted())
            appender->AppendRecord(record);
    }

    // 处理自己的processors
    for (auto& processor : _processors)
    {
        if (processor->IsStarted())
            processor->ProcessRecord(record);
    }

    return true;
}

void DynamicRouter::AddRoute(const std::string& target, const std::shared_ptr<Appender>& appender)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _appender_routes[target].push_back(appender);
}

void DynamicRouter::AddRoute(const std::string& target, const std::shared_ptr<Processor>& processor)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _processor_routes[target].push_back(processor);
}

void DynamicRouter::RemoveRoute(const std::string& target)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _appender_routes.erase(target);
    _processor_routes.erase(target);
}

void DynamicRouter::ClearRoutes()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _appender_routes.clear();
    _processor_routes.clear();
}

std::vector<std::string> DynamicRouter::ExtractTargets(const Record& record)
{
    std::vector<std::string> targets;

    // 从record的buffer中提取目标信息
    // 这里假设targets是由SmartClassifier以逗号分隔的字符串形式存储在buffer的末尾
    // 实际实现中应该有更可靠的序列化/反序列化机制
    if (!record.buffer.empty())
    {
        // 简单的实现：假设最后一个字节是目标字符串的长度
        // 实际应用中应该使用更健壮的格式
        size_t buffer_size = record.buffer.size();
        if (buffer_size > 0)
        {
            // 尝试读取目标信息（这里使用简单的方式，实际应根据SmartClassifier的存储格式调整）
            std::string buffer_content(record.buffer.begin(), record.buffer.end());
            
            // 假设目标信息是用特殊分隔符开头的，比如"||targets:"
            size_t target_pos = buffer_content.find("||targets:");
            if (target_pos != std::string::npos)
            {
                std::string targets_str = buffer_content.substr(target_pos + 10);
                std::istringstream iss(targets_str);
                std::string target;
                while (std::getline(iss, target, ','))
                {
                    if (!target.empty())
                        targets.push_back(target);
                }
            }
        }
    }

    return targets;
}

} // namespace CppLogging