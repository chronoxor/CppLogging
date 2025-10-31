#ifndef CPPLOGGING_PROCESSORS_SENSITIVE_DATA_PROCESSOR_H
#define CPPLOGGING_PROCESSORS_SENSITIVE_DATA_PROCESSOR_H

#include "logging/processor.h"
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

namespace CppLogging {

// 脱敏策略类型
enum class MaskingStrategy {
    NONE,           // 不脱敏
    PARTIAL,        // 部分掩码（如手机号：138****1234）
    ENCRYPT,        // 加密存储
    HASH,           // 哈希处理
    REDACT          // 完全删除
};

// 敏感字段规则
struct SensitiveFieldRule {
    std::string name;               // 字段名称
    std::string pattern;            // 正则表达式模式
    std::regex regex;               // 编译后的正则表达式
    MaskingStrategy strategy;       // 脱敏策略
    std::string replacement;        // 替换模板（如"***"）
    std::vector<std::string> context; // 上下文条件（如日志级别、模块等）
    int priority;                   // 优先级（值越高越优先）

    SensitiveFieldRule() : strategy(MaskingStrategy::PARTIAL), priority(0) {}
    SensitiveFieldRule(const std::string& n, const std::string& p, MaskingStrategy s, const std::string& r = "***", int prio = 0)
        : name(n), pattern(p), regex(p), strategy(s), replacement(r), priority(prio) {}
};

// 脱敏上下文
struct MaskingContext {
    std::string log_level;          // 日志级别
    std::string logger_name;        // 记录器名称
    std::string thread_id;          // 线程ID
    std::string module;             // 业务模块
    bool is_debug_mode;             // 是否为调试模式
};

// 脱敏结果
struct MaskingResult {
    std::string masked_text;        // 脱敏后的文本
    size_t fields_masked;           // 脱敏的字段数量
    bool modified;                  // 是否修改了文本
};

class SensitiveDataProcessor : public Processor {
public:
    SensitiveDataProcessor();
    explicit SensitiveDataProcessor(const std::shared_ptr<Layout>& layout);
    ~SensitiveDataProcessor() = default;

    // 实现Processor接口
    bool ProcessRecord(Record& record) override;

    // 添加敏感字段规则
    void AddSensitiveFieldRule(const SensitiveFieldRule& rule);
    void AddSensitiveFieldRule(const std::string& name, const std::string& pattern, MaskingStrategy strategy, const std::string& replacement = "***", int priority = 0);

    // 加载规则（从JSON配置）
    void LoadRulesFromJSON(const std::string& json_content);
    void LoadRulesFromFile(const std::string& file_path);

    // 设置调试模式（调试模式下不脱敏）
    void SetDebugMode(bool enabled) { _debug_mode = enabled; }

    // 设置全局脱敏策略
    void SetGlobalMaskingStrategy(MaskingStrategy strategy) { _global_strategy = strategy; }

    // 注册自定义脱敏函数
    void RegisterCustomMaskingFunction(const std::string& name, std::function<std::string(const std::string&, const SensitiveFieldRule&)> func);

private:
    // 敏感字段规则列表
    std::vector<SensitiveFieldRule> _sensitive_rules;
    // 按优先级排序的规则索引
    std::vector<size_t> _priority_index;

    // 自定义脱敏函数
    std::unordered_map<std::string, std::function<std::string(const std::string&, const SensitiveFieldRule&)>> _custom_masking_functions;

    // 全局设置
    bool _debug_mode;               // 调试模式（调试模式下不脱敏）
    MaskingStrategy _global_strategy; // 全局脱敏策略

    // 构建脱敏上下文
    MaskingContext BuildContext(const Record& record) const;

    // 应用脱敏规则
    MaskingResult ApplyMaskingRules(const std::string& text, const MaskingContext& context) const;

    // 根据策略脱敏字段
    std::string MaskField(const std::string& value, const SensitiveFieldRule& rule) const;

    // 内置脱敏策略实现
    std::string PartialMask(const std::string& value, const SensitiveFieldRule& rule) const;
    std::string EncryptMask(const std::string& value, const SensitiveFieldRule& rule) const;
    std::string HashMask(const std::string& value, const SensitiveFieldRule& rule) const;
    std::string RedactMask(const std::string& value, const SensitiveFieldRule& rule) const;

    // 辅助函数：检测是否为调试模式
    bool IsDebugMode(const MaskingContext& context) const;

    // 辅助函数：检查规则是否适用于当前上下文
    bool RuleAppliesToContext(const SensitiveFieldRule& rule, const MaskingContext& context) const;

    // 辅助函数：排序规则
    void SortRulesByPriority();
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_SENSITIVE_DATA_PROCESSOR_H