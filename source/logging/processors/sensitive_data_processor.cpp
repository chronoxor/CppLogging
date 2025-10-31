#include "logging/processors/sensitive_data_processor.h"
#include "logging/logger.h"
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

// 链接OpenSSL库
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

namespace CppLogging {

SensitiveDataProcessor::SensitiveDataProcessor()
    : Processor()
    , _debug_mode(false)
    , _global_strategy(MaskingStrategy::PARTIAL)
{
    // 初始化内置敏感字段规则
    AddDefaultRules();
}

SensitiveDataProcessor::SensitiveDataProcessor(const std::shared_ptr<Layout>& layout)
    : Processor(layout)
    , _debug_mode(false)
    , _global_strategy(MaskingStrategy::PARTIAL)
{
    // 初始化内置敏感字段规则
    AddDefaultRules();
}

void SensitiveDataProcessor::AddDefaultRules()
{
    // 手机号规则（11位数字）
    AddSensitiveFieldRule("phone", R"(1[3-9]\d{9})", MaskingStrategy::PARTIAL, "***", 10);
    
    // 身份证号规则（15位或18位数字或最后一位为X）
    AddSensitiveFieldRule("id_card", R"(\d{15}(?:\d{2}[\dXx])?)", MaskingStrategy::ENCRYPT, "***", 20);
    
    // 银行卡号规则（16-19位数字）
    AddSensitiveFieldRule("bank_card", R"(\d{16,19})", MaskingStrategy::PARTIAL, "***", 15);
    
    // 邮箱规则
    AddSensitiveFieldRule("email", R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})
", MaskingStrategy::PARTIAL, "***", 5);
    
    // IP地址规则
    AddSensitiveFieldRule("ip_address", R"((?:\d{1,3}\.){3}\d{1,3})
", MaskingStrategy::PARTIAL, "***", 5);
}

bool SensitiveDataProcessor::ProcessRecord(Record& record)
{
    // 如果是调试模式且全局调试模式开启，不脱敏
    if (IsDebugMode(BuildContext(record)))
        return Processor::ProcessRecord(record);

    // 应用布局
    if (_layout)
        _layout->LayoutRecord(record);

    // 获取日志消息
    std::string message = record.message();

    // 构建脱敏上下文
    MaskingContext context = BuildContext(record);

    // 应用脱敏规则
    MaskingResult result = ApplyMaskingRules(message, context);

    // 更新日志消息
    if (result.modified)
        record.message() = result.masked_text;

    // 处理附加器
    for (auto& appender : _appenders)
        appender->AppendRecord(record);

    // 处理子处理器
    for (auto& processor : _processors)
        processor->ProcessRecord(record);

    return true;
}

void SensitiveDataProcessor::AddSensitiveFieldRule(const SensitiveFieldRule& rule)
{
    std::lock_guard<std::mutex> lock(_lock);
    _sensitive_rules.push_back(rule);
    _priority_index.push_back(_sensitive_rules.size() - 1);
    SortRulesByPriority();
}

void SensitiveDataProcessor::AddSensitiveFieldRule(const std::string& name, const std::string& pattern, MaskingStrategy strategy, const std::string& replacement, int priority)
{
    SensitiveFieldRule rule(name, pattern, strategy, replacement, priority);
    AddSensitiveFieldRule(rule);
}

void SensitiveDataProcessor::LoadRulesFromJSON(const std::string& json_content)
{
    try
    {
        nlohmann::json config = nlohmann::json::parse(json_content);

        if (config.contains("debug_mode"))
            _debug_mode = config["debug_mode"].get<bool>();

        if (config.contains("global_strategy"))
        {
            std::string strategy_str = config["global_strategy"].get<std::string>();
            if (strategy_str == "none")
                _global_strategy = MaskingStrategy::NONE;
            else if (strategy_str == "partial")
                _global_strategy = MaskingStrategy::PARTIAL;
            else if (strategy_str == "encrypt")
                _global_strategy = MaskingStrategy::ENCRYPT;
            else if (strategy_str == "hash")
                _global_strategy = MaskingStrategy::HASH;
            else if (strategy_str == "redact")
                _global_strategy = MaskingStrategy::REDACT;
        }

        if (config.contains("rules"))
        {
            auto rules = config["rules"].get<std::vector<nlohmann::json>>();
            for (const auto& rule_json : rules)
            {
                SensitiveFieldRule rule;
                
                if (rule_json.contains("name"))
                    rule.name = rule_json["name"].get<std::string>();
                
                if (rule_json.contains("pattern"))
                    rule.pattern = rule_json["pattern"].get<std::string>();
                
                if (rule_json.contains("strategy"))
                {
                    std::string strategy_str = rule_json["strategy"].get<std::string>();
                    if (strategy_str == "none")
                        rule.strategy = MaskingStrategy::NONE;
                    else if (strategy_str == "partial")
                        rule.strategy = MaskingStrategy::PARTIAL;
                    else if (strategy_str == "encrypt")
                        rule.strategy = MaskingStrategy::ENCRYPT;
                    else if (strategy_str == "hash")
                        rule.strategy = MaskingStrategy::HASH;
                    else if (strategy_str == "redact")
                        rule.strategy = MaskingStrategy::REDACT;
                }
                
                if (rule_json.contains("replacement"))
                    rule.replacement = rule_json["replacement"].get<std::string>();
                
                if (rule_json.contains("context"))
                    rule.context = rule_json["context"].get<std::vector<std::string>>();
                
                if (rule_json.contains("priority"))
                    rule.priority = rule_json["priority"].get<int>();
                
                // 编译正则表达式
                rule.regex = std::regex(rule.pattern);
                
                AddSensitiveFieldRule(rule);
            }
        }
    }
    catch (const std::exception& ex)
    {
        CppLogging::Logger::Error("Failed to load sensitive data rules from JSON: {}", ex.what());
    }
}

void SensitiveDataProcessor::LoadRulesFromFile(const std::string& file_path)
{
    try
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            CppLogging::Logger::Error("Failed to open sensitive data rules file: {}", file_path);
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        LoadRulesFromJSON(buffer.str());
    }
    catch (const std::exception& ex)
    {
        CppLogging::Logger::Error("Failed to load sensitive data rules from file: {}", ex.what());
    }
}

void SensitiveDataProcessor::RegisterCustomMaskingFunction(const std::string& name, std::function<std::string(const std::string&, const SensitiveFieldRule&)> func)
{
    std::lock_guard<std::mutex> lock(_lock);
    _custom_masking_functions[name] = func;
}

MaskingContext SensitiveDataProcessor::BuildContext(const Record& record) const
{
    MaskingContext context;
    
    // 设置日志级别
    switch (record.level())
    {
        case Level::NONE:
            context.log_level = "none";
            break;
        case Level::DEBUG:
            context.log_level = "debug";
            break;
        case Level::INFO:
            context.log_level = "info";
            break;
        case Level::WARN:
            context.log_level = "warn";
            break;
        case Level::ERROR:
            context.log_level = "error";
            break;
        case Level::FATAL:
            context.log_level = "fatal";
            break;
        default:
            context.log_level = "unknown";
            break;
    }
    
    // 设置记录器名称
    context.logger_name = record.logger();
    
    // 设置线程ID
    context.thread_id = std::to_string(record.thread());
    
    // 从缓冲区中提取模块信息（假设模块信息存储在缓冲区中）
    // 这里使用一个简单的示例，实际实现可能需要根据具体格式调整
    const void* buffer = record.buffer().data();
    size_t buffer_size = record.buffer().size();
    if (buffer_size > 0)
    {
        std::string buffer_str(static_cast<const char*>(buffer), buffer_size);
        size_t module_pos = buffer_str.find("module:");
        if (module_pos != std::string::npos)
        {
            size_t end_pos = buffer_str.find_first_of("|,;
", module_pos + 7);
            if (end_pos != std::string::npos)
                context.module = buffer_str.substr(module_pos + 7, end_pos - module_pos - 7);
            else
                context.module = buffer_str.substr(module_pos + 7);
        }
    }
    
    // 设置调试模式
    context.is_debug_mode = _debug_mode;
    
    return context;
}

MaskingResult SensitiveDataProcessor::ApplyMaskingRules(const std::string& text, const MaskingContext& context) const
{
    MaskingResult result;
    result.masked_text = text;
    result.fields_masked = 0;
    result.modified = false;

    std::lock_guard<std::mutex> lock(_lock);
    if (_sensitive_rules.empty())
        return result;

    // 按优先级顺序应用规则
    for (size_t index : _priority_index)
    {
        const SensitiveFieldRule& rule = _sensitive_rules[index];

        // 检查规则是否适用于当前上下文
        if (!RuleAppliesToContext(rule, context))
            continue;

        // 应用正则表达式匹配
        std::string::const_iterator search_start(result.masked_text.cbegin());
        std::smatch match;

        while (std::regex_search(search_start, result.masked_text.cend(), match, rule.regex))
        {
            // 脱敏匹配到的字段
            std::string masked_value = MaskField(match[0], rule);

            // 替换原始值
            result.masked_text.replace(match.position(), match.length(), masked_value);

            // 更新搜索起点
            search_start = result.masked_text.cbegin() + match.position() + masked_value.length();

            // 更新统计信息
            result.fields_masked++;
            result.modified = true;
        }
    }

    return result;
}

std::string SensitiveDataProcessor::MaskField(const std::string& value, const SensitiveFieldRule& rule) const
{
    // 如果策略是NONE，返回原始值
    if (rule.strategy == MaskingStrategy::NONE)
        return value;

    // 检查是否有自定义脱敏函数
    auto it = _custom_masking_functions.find(rule.name);
    if (it != _custom_masking_functions.end())
        return it->second(value, rule);

    // 应用内置脱敏策略
    switch (rule.strategy)
    {
        case MaskingStrategy::PARTIAL:
            return PartialMask(value, rule);
        case MaskingStrategy::ENCRYPT:
            return EncryptMask(value, rule);
        case MaskingStrategy::HASH:
            return HashMask(value, rule);
        case MaskingStrategy::REDACT:
            return RedactMask(value, rule);
        default:
            return value;
    }
}

std::string SensitiveDataProcessor::PartialMask(const std::string& value, const SensitiveFieldRule& rule) const
{
    if (value.empty())
        return value;

    // 根据不同类型的敏感数据应用不同的部分掩码策略
    if (rule.name == "phone")
    {
        // 手机号：保留前3位和后4位，中间用*代替
        if (value.length() >= 7)
            return value.substr(0, 3) + "****" + value.substr(value.length() - 4);
        else
            return std::string(value.length(), '*');
    }
    else if (rule.name == "id_card")
    {
        // 身份证号：保留前6位和后4位，中间用*代替
        if (value.length() >= 10)
            return value.substr(0, 6) + std::string(value.length() - 10, '*') + value.substr(value.length() - 4);
        else
            return std::string(value.length(), '*');
    }
    else if (rule.name == "bank_card")
    {
        // 银行卡号：保留前6位和后4位，中间用*代替
        if (value.length() >= 10)
            return value.substr(0, 6) + std::string(value.length() - 10, '*') + value.substr(value.length() - 4);
        else
            return std::string(value.length(), '*');
    }
    else if (rule.name == "email")
    {
        // 邮箱：保留用户名的前2位和域名，中间用*代替
        size_t at_pos = value.find('@');
        if (at_pos != std::string::npos && at_pos >= 2)
            return value.substr(0, 2) + "***" + value.substr(at_pos);
        else
            return value;
    }
    else if (rule.name == "ip_address")
    {
        // IP地址：保留前两段，后两段用*代替
        std::vector<std::string> parts;
        std::string part;
        std::istringstream iss(value);
        while (std::getline(iss, part, '.'))
            parts.push_back(part);

        if (parts.size() == 4)
            return parts[0] + "." + parts[1] + ".***.***";
        else
            return value;
    }
    else
    {
        // 默认部分掩码策略：保留前半部分，后半部分用*代替
        size_t half = value.length() / 2;
        return value.substr(0, half) + std::string(value.length() - half, '*');
    }
}

std::string SensitiveDataProcessor::EncryptMask(const std::string& value, const SensitiveFieldRule& rule) const
{
    if (value.empty())
        return value;

    // 使用AES-256-CBC加密
    // 注意：这里使用硬编码的密钥和IV，实际应用中应该从安全的地方获取
    // 并且每个敏感字段应该使用不同的密钥
    unsigned char key[] = "01234567890123456789012345678901"; // 32字节密钥
    unsigned char iv[] = "0123456789012345"; // 16字节IV

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return value;

    int len = 0;
    int ciphertext_len = 0;
    unsigned char* ciphertext = nullptr;

    // 分配加密缓冲区
    ciphertext = new unsigned char[value.length() + AES_BLOCK_SIZE];
    if (!ciphertext)
    {
        EVP_CIPHER_CTX_free(ctx);
        return value;
    }

    // 初始化加密操作
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1)
    {
        delete[] ciphertext;
        EVP_CIPHER_CTX_free(ctx);
        return value;
    }

    // 执行加密
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, reinterpret_cast<const unsigned char*>(value.c_str()), value.length()) != 1)
    {
        delete[] ciphertext;
        EVP_CIPHER_CTX_free(ctx);
        return value;
    }
    ciphertext_len = len;

    // 完成加密
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1)
    {
        delete[] ciphertext;
        EVP_CIPHER_CTX_free(ctx);
        return value;
    }
    ciphertext_len += len;

    // 将加密结果转换为十六进制字符串
    std::string result;
    result.reserve(ciphertext_len * 2);
    for (int i = 0; i < ciphertext_len; ++i)
    {
        char hex[3];
        sprintf_s(hex, sizeof(hex), "%02x", ciphertext[i]);
        result += hex;
    }

    // 清理资源
    delete[] ciphertext;
    EVP_CIPHER_CTX_free(ctx);

    return result;
}

std::string SensitiveDataProcessor::HashMask(const std::string& value, const SensitiveFieldRule& rule) const
{
    if (value.empty())
        return value;

    // 使用SHA-256哈希
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, value.c_str(), value.length());
    SHA256_Final(hash, &sha256);

    // 将哈希结果转换为十六进制字符串
    std::string result;
    result.reserve(SHA256_DIGEST_LENGTH * 2);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        char hex[3];
        sprintf_s(hex, sizeof(hex), "%02x", hash[i]);
        result += hex;
    }

    return result;
}

std::string SensitiveDataProcessor::RedactMask(const std::string& value, const SensitiveFieldRule& rule) const
{
    // 完全删除敏感数据，用指定的替换文本代替
    return rule.replacement;
}

bool SensitiveDataProcessor::IsDebugMode(const MaskingContext& context) const
{
    // 如果全局调试模式开启，或者上下文显示是调试模式，则不脱敏
    return context.is_debug_mode || (context.log_level == "debug");
}

bool SensitiveDataProcessor::RuleAppliesToContext(const SensitiveFieldRule& rule, const MaskingContext& context) const
{
    // 如果规则没有上下文条件，则适用于所有上下文
    if (rule.context.empty())
        return true;

    // 检查上下文条件
    for (const std::string& ctx : rule.context)
    {
        // 检查日志级别
        if (ctx == "level:debug" && context.log_level == "debug")
            return true;
        if (ctx == "level:info" && context.log_level == "info")
            return true;
        if (ctx == "level:warn" && context.log_level == "warn")
            return true;
        if (ctx == "level:error" && context.log_level == "error")
            return true;
        if (ctx == "level:fatal" && context.log_level == "fatal")
            return true;

        // 检查模块
        if (ctx.find("module:") == 0)
        {
            std::string module = ctx.substr(7);
            if (context.module == module)
                return true;
        }

        // 检查记录器名称
        if (ctx.find("logger:") == 0)
        {
            std::string logger = ctx.substr(7);
            if (context.logger_name == logger)
                return true;
        }
    }

    // 没有匹配的上下文条件
    return false;
}

void SensitiveDataProcessor::SortRulesByPriority()
{
    // 按优先级降序排序
    std::sort(_priority_index.begin(), _priority_index.end(), [this](size_t a, size_t b) {
        return _sensitive_rules[a].priority > _sensitive_rules[b].priority;
    });
}

} // namespace CppLogging