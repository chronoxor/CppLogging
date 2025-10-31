#include "logging/processors/log_merger.h"
#include "logging/logger.h"
#include <openssl/sha.h>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>

namespace CppLogging {

LogMerger::LogMerger()
    : Processor()
    , _config()
    , _similarity_calculator(std::make_shared<NGramSimilarity>(3))
    , _cleanup_running(false)
{
    // 启动清理线程
    _cleanup_running = true;
    _cleanup_thread = std::thread(&LogMerger::CleanupThread, this);
}

LogMerger::LogMerger(const std::shared_ptr<Layout>& layout)
    : Processor(layout)
    , _config()
    , _similarity_calculator(std::make_shared<NGramSimilarity>(3))
    , _cleanup_running(false)
{
    // 启动清理线程
    _cleanup_running = true;
    _cleanup_thread = std::thread(&LogMerger::CleanupThread, this);
}

bool LogMerger::ProcessRecord(Record& record)
{
    // 如果不启用合并和去重，直接处理
    if (!_config.enable_merging && !_config.enable_deduplication)
        return Processor::ProcessRecord(record);

    // 应用布局
    if (_layout)
        _layout->LayoutRecord(record);

    // 生成日志ID
    std::string log_id = GenerateLogID(record);

    // 提取调用链ID
    std::string call_chain_id = ExtractCallChainID(record);

    // 尝试合并日志
    bool merged = false;
    MergedLogEntry new_entry(record);
    new_entry.call_chain_id = call_chain_id;

    {
        std::lock_guard<std::mutex> lock(_merged_logs_mutex);

        // 查找相似的日志
        for (auto& [key, entry] : _merged_logs)
        {
            // 检查调用链ID是否相同（如果需要保留调用链）
            if (_config.preserve_call_chain && entry.call_chain_id != call_chain_id)
                continue;

            // 检查是否相似
            if (AreLogsSimilar(entry.original_record, record))
            {
                // 合并日志
                if (MergeLogs(entry, record))
                {
                    merged = true;
                    break;
                }
            }
        }

        // 如果没有找到相似的日志，添加新的合并条目
        if (!merged)
        {
            _merged_logs[log_id] = new_entry;
        }
    }

    // 如果启用了去重，只处理合并后的日志（在Flush时处理）
    if (_config.enable_deduplication)
        return true;

    // 否则，继续处理原始日志
    for (auto& appender : _appenders)
        appender->AppendRecord(record);

    for (auto& processor : _processors)
        processor->ProcessRecord(record);

    return true;
}

bool LogMerger::Flush()
{
    bool result = true;

    // 处理所有合并后的日志
    {
        std::lock_guard<std::mutex> lock(_merged_logs_mutex);

        for (auto& [key, entry] : _merged_logs)
        {
            // 生成合并后的日志消息
            std::string merged_message = GenerateMergedMessage(entry);
            Record merged_record = entry.original_record;
            merged_record.message() = merged_message;

            // 添加合并信息到缓冲区
            std::stringstream buffer;
            buffer << "merged: true";
            buffer << ", duplicates: " << entry.duplicate_count;
            buffer << ", first_seen: " << std::chrono::duration_cast<std::chrono::milliseconds>(entry.first_seen.time_since_epoch()).count();
            buffer << ", last_seen: " << std::chrono::duration_cast<std::chrono::milliseconds>(entry.last_seen.time_since_epoch()).count();
            if (!entry.call_chain_id.empty())
                buffer << ", call_chain: " << entry.call_chain_id;

            merged_record << buffer.str();

            // 处理合并后的日志
            for (auto& appender : _appenders)
                result &= appender->AppendRecord(merged_record);

            for (auto& processor : _processors)
                result &= processor->ProcessRecord(merged_record);
        }

        // 清空合并日志
        _merged_logs.clear();
    }

    // 清理过期日志
    CleanupExpiredLogs();

    return result && Processor::Flush();
}

double EditDistanceSimilarity::CalculateSimilarity(const std::string& msg1, const std::string& msg2) const
{
    if (msg1.empty() && msg2.empty())
        return 1.0;
    if (msg1.empty() || msg2.empty())
        return 0.0;

    int distance = CalculateEditDistance(msg1, msg2);
    int max_length = std::max(msg1.length(), msg2.length());
    return 1.0 - static_cast<double>(distance) / max_length;
}

int EditDistanceSimilarity::CalculateEditDistance(const std::string& s1, const std::string& s2) const
{
    int m = s1.length();
    int n = s2.length();

    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    for (int i = 0; i <= m; ++i)
        dp[i][0] = i;

    for (int j = 0; j <= n; ++j)
        dp[0][j] = j;

    for (int i = 1; i <= m; ++i)
    {
        for (int j = 1; j <= n; ++j)
        {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min(std::min(dp[i - 1][j] + 1, dp[i][j - 1] + 1), dp[i - 1][j - 1] + cost);
        }
    }

    return dp[m][n];
}

double NGramSimilarity::CalculateSimilarity(const std::string& msg1, const std::string& msg2) const
{
    if (msg1.empty() && msg2.empty())
        return 1.0;
    if (msg1.empty() || msg2.empty())
        return 0.0;

    // 提取n-gram特征
    auto features1 = ExtractNGramFeatures(msg1);
    auto features2 = ExtractNGramFeatures(msg2);

    // 计算交集和并集大小
    int intersection = 0;
    int union_size = 0;

    // 计算交集
    for (const auto& [gram, count] : features1)
    {
        if (features2.find(gram) != features2.end())
            intersection += std::min(count, features2[gram]);
    }

    // 计算并集
    for (const auto& [gram, count] : features1)
        union_size += count;

    for (const auto& [gram, count] : features2)
        union_size += count;

    union_size -= intersection;

    if (union_size == 0)
        return 0.0;

    return static_cast<double>(intersection) / union_size;
}

std::unordered_map<std::string, int> NGramSimilarity::ExtractNGramFeatures(const std::string& text) const
{
    std::unordered_map<std::string, int> features;

    if (text.length() < _n)
        return features;

    for (size_t i = 0; i <= text.length() - _n; ++i)
    {
        std::string gram = text.substr(i, _n);
        features[gram]++;
    }

    return features;
}

std::vector<MergedLogEntry> LogMerger::GetMergedLogs() const
{
    std::vector<MergedLogEntry> result;
    std::lock_guard<std::mutex> lock(_merged_logs_mutex);

    for (const auto& [key, entry] : _merged_logs)
        result.push_back(entry);

    return result;
}

std::string LogMerger::GenerateLogID(const Record& record) const
{
    // 使用SHA-256哈希生成唯一ID
    std::string message = record.message();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, message.c_str(), message.length());
    SHA256_Final(hash, &sha256);

    // 将哈希结果转换为十六进制字符串
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);

    return ss.str();
}

std::string LogMerger::ExtractCallChainID(const Record& record) const
{
    // 从缓冲区中提取调用链ID（假设调用链ID存储在缓冲区中）
    // 这里使用一个简单的示例，实际实现可能需要根据具体格式调整
    const void* buffer = record.buffer().data();
    size_t buffer_size = record.buffer().size();
    if (buffer_size > 0)
    {
        std::string buffer_str(static_cast<const char*>(buffer), buffer_size);
        size_t chain_pos = buffer_str.find("call_chain:");
        if (chain_pos != std::string::npos)
        {
            size_t end_pos = buffer_str.find_first_of("|,;
", chain_pos + 11);
            if (end_pos != std::string::npos)
                return buffer_str.substr(chain_pos + 11, end_pos - chain_pos - 11);
            else
                return buffer_str.substr(chain_pos + 11);
        }
    }
    return "";
}

bool LogMerger::AreLogsSimilar(const Record& record1, const Record& record2) const
{
    // 如果有自定义合并函数，使用自定义函数
    if (_custom_merge_function)
        return _custom_merge_function(record1, record2);

    // 检查日志级别是否相同
    if (record1.level() != record2.level())
        return false;

    // 检查记录器名称是否相同
    if (record1.logger() != record2.logger())
        return false;

    // 检查线程ID是否相同
    if (record1.thread() != record2.thread())
        return false;

    // 计算消息相似度
    double similarity = _similarity_calculator->CalculateSimilarity(record1.message(), record2.message());
    return similarity >= _config.similarity_threshold;
}

bool LogMerger::MergeLogs(MergedLogEntry& target, const Record& source) const
{
    // 检查重复次数是否超过最大值
    if (target.duplicate_count >= _config.max_duplicates)
        return false;

    // 更新重复次数
    target.duplicate_count++;

    // 更新最后出现时间
    target.last_seen = std::chrono::system_clock::now();

    // 生成源日志的ID并添加到重复ID列表
    std::string source_id = GenerateLogID(source);
    target.duplicate_ids.push_back(source_id);

    // 合并字段信息（这里使用一个简单的示例，实际实现可能需要更复杂的逻辑）
    const void* source_buffer = source.buffer().data();
    size_t source_buffer_size = source.buffer().size();
    if (source_buffer_size > 0)
    {
        std::string source_buffer_str(static_cast<const char*>(source_buffer), source_buffer_size);
        target.merged_fields.push_back(source_buffer_str);
    }

    return true;
}

void LogMerger::CleanupExpiredLogs()
{
    auto now = std::chrono::system_clock::now();
    std::lock_guard<std::mutex> lock(_merged_logs_mutex);

    for (auto it = _merged_logs.begin(); it != _merged_logs.end();)
    {
        // 检查日志是否过期
        auto age = now - it->second.last_seen;
        if (age > _config.merge_window)
            it = _merged_logs.erase(it);
        else
            ++it;
    }
}

void LogMerger::CleanupThread()
{
    while (_cleanup_running)
    {
        try
        {
            // 清理过期日志
            CleanupExpiredLogs();

            // 等待下一次清理
            std::this_thread::sleep_for(_config.merge_window / 2);
        }
        catch (const std::exception& ex)
        {
            // 记录清理错误
            CppLogging::Logger::Error("Log merger cleanup thread error: {}", ex.what());
        }
    }
}

std::string LogMerger::GenerateMergedMessage(const MergedLogEntry& entry) const
{
    std::stringstream ss;

    // 添加原始消息
    ss << entry.original_record.message();

    // 添加合并信息
    if (entry.duplicate_count > 1)
    {
        ss << " [MERGED: " << entry.duplicate_count << " times, ";
        ss << "first: " << std::chrono::duration_cast<std::chrono::milliseconds>(entry.first_seen.time_since_epoch()).count() << ", ";
        ss << "last: " << std::chrono::duration_cast<std::chrono::milliseconds>(entry.last_seen.time_since_epoch()).count() << "]";
    }

    return ss.str();
}

} // namespace CppLogging