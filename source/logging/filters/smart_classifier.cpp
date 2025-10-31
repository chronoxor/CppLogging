#include "logging/filters/smart_classifier.h"
#include "filesystem/file.h"
#include "string/format.h"
#include <fstream>
#include <regex>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace CppLogging {

SmartClassifier::SmartClassifier()
    : Filter()
{
}

SmartClassifier::SmartClassifier(const std::string& config_file)
    : Filter(), _config_file(config_file)
{
    LoadRules(config_file);
}

bool SmartClassifier::FilterRecord(Record& record)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_rules.empty())
        return true;

    std::vector<ClassificationRule> matching_rules;

    // 按优先级降序排序规则
    std::vector<ClassificationRule> sorted_rules = _rules;
    std::sort(sorted_rules.begin(), sorted_rules.end(), [](const ClassificationRule& a, const ClassificationRule& b) {
        return a.priority > b.priority;
    });

    // 计算异常指纹（如果有）
    std::string exception_fingerprint;
    if (!record.buffer.empty())
        exception_fingerprint = CalculateExceptionFingerprint(record);

    // 匹配规则
    for (const auto& rule : sorted_rules)
    {
        // 级别匹配
        if (rule.level != Level::NONE && record.level < rule.level)
            continue;

        // 模块匹配
        if (!rule.modules.empty() && rule.modules.find(record.logger) == rule.modules.end())
            continue;

        // 关键词匹配
        if (!rule.keywords.empty())
        {
            bool keyword_matched = false;
            for (const auto& keyword : rule.keywords)
            {
                if (record.message.find(keyword) != std::string::npos)
                {
                    keyword_matched = true;
                    break;
                }
            }
            if (!keyword_matched)
                continue;
        }

        // 异常指纹匹配
        if (!rule.fingerprint.empty() && exception_fingerprint != rule.fingerprint)
            continue;

        matching_rules.push_back(rule);
    }

    // 应用匹配的规则
    if (!matching_rules.empty())
    {
        // 将所有匹配规则的目标合并并存储在record的buffer中
        std::unordered_set<std::string> all_targets;
        for (const auto& rule : matching_rules)
        {
            for (const auto& target : rule.targets)
                all_targets.insert(target);
        }

        if (!all_targets.empty())
        {
            std::string targets_str = fmt::format("||targets:{}", fmt::join(all_targets, ","));
            record.buffer.insert(record.buffer.end(), targets_str.begin(), targets_str.end());
        }
    }

    return true;
}

void SmartClassifier::LoadRules(const std::string& config_file)
{
    std::lock_guard<std::mutex> lock(_mutex);

    std::ifstream file(config_file);
    if (!file.is_open())
        return;

    json config;
    file >> config;

    _rules.clear();
    if (config.contains("rules"))
    {
        for (const auto& rule_json : config["rules"])
        {
            ClassificationRule rule;
            rule.priority = rule_json.value("priority", 0);
            rule.level = Level::FromString(rule_json.value("level", "NONE"));

            if (rule_json.contains("keywords"))
            {
                for (const auto& keyword : rule_json["keywords"])
                    rule.keywords.insert(keyword.get<std::string>());
            }

            if (rule_json.contains("modules"))
            {
                for (const auto& module : rule_json["modules"])
                    rule.modules.insert(module.get<std::string>());
            }

            rule.fingerprint = rule_json.value("fingerprint", "");

            if (rule_json.contains("targets"))
            {
                for (const auto& target : rule_json["targets"])
                    rule.targets.push_back(target.get<std::string>());
            }

            _rules.push_back(rule);
        }
    }

    IndexRules();
    _config_file = config_file;
}

void SmartClassifier::ReloadRules()
{
    if (!_config_file.empty())
        LoadRules(_config_file);
}

void SmartClassifier::AddRule(const ClassificationRule& rule)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _rules.push_back(rule);
    IndexRules();
}

void SmartClassifier::ClearRules()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _rules.clear();
    _keyword_index.clear();
    _module_index.clear();
}

void SmartClassifier::IndexRules()
{
    _keyword_index.clear();
    _module_index.clear();

    for (const auto& rule : _rules)
    {
        // 索引关键词
        for (const auto& keyword : rule.keywords)
            _keyword_index[keyword].push_back(rule);

        // 索引模块
        for (const auto& module : rule.modules)
            _module_index[module].push_back(rule);
    }
}

std::string SmartClassifier::CalculateExceptionFingerprint(const Record& record)
{
    // 简单的异常指纹计算实现
    // 实际应根据序列化的异常信息生成唯一指纹
    std::string fingerprint;
    if (record.buffer.size() >= 4)
    {
        // 使用前4个字节作为简单指纹
        fingerprint = fmt::format("{:02x}{:02x}{:02x}{:02x}",
            record.buffer[0], record.buffer[1], record.buffer[2], record.buffer[3]);
    }
    return fingerprint;
}

} // namespace CppLogging