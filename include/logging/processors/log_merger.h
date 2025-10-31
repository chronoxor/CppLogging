#ifndef CPPLOGGING_PROCESSORS_LOG_MERGER_H
#define CPPLOGGING_PROCESSORS_LOG_MERGER_H

#include "logging/processor.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>
#include <memory>
#include <functional>

namespace CppLogging {

// 日志合并配置
struct LogMergeConfig {
    std::chrono::milliseconds merge_window; // 合并窗口大小（默认1秒）
    double similarity_threshold; // 日志相似度阈值（0.0-1.0，默认0.8）
    size_t max_duplicates; // 最大重复次数（默认100）
    bool preserve_call_chain; // 是否保留调用链信息（默认true）
    bool enable_deduplication; // 是否启用重复日志删除（默认true）
    bool enable_merging; // 是否启用冗余信息合并（默认true）

    LogMergeConfig()
        : merge_window(std::chrono::milliseconds(1000))
        , similarity_threshold(0.8)
        , max_duplicates(100)
        , preserve_call_chain(true)
        , enable_deduplication(true)
        , enable_merging(true)
    {}
};

// 合并后的日志条目
struct MergedLogEntry {
    Record original_record; // 原始日志记录
    size_t duplicate_count; // 重复次数
    std::vector<std::string> duplicate_ids; // 重复日志的ID
    std::vector<std::string> merged_fields; // 合并的字段信息
    std::chrono::system_clock::time_point first_seen; // 首次出现时间
    std::chrono::system_clock::time_point last_seen; // 最后出现时间
    std::string call_chain_id; // 调用链ID

    MergedLogEntry(const Record& record)
        : original_record(record)
        , duplicate_count(1)
        , first_seen(std::chrono::system_clock::now())
        , last_seen(std::chrono::system_clock::now())
    {}
};

// 日志相似度计算器
class LogSimilarityCalculator {
public:
    virtual ~LogSimilarityCalculator() = default;

    // 计算两个日志消息的相似度（0.0-1.0）
    virtual double CalculateSimilarity(const std::string& msg1, const std::string& msg2) const = 0;
};

// 基于编辑距离的相似度计算器
class EditDistanceSimilarity : public LogSimilarityCalculator {
public:
    double CalculateSimilarity(const std::string& msg1, const std::string& msg2) const override;

private:
    // 计算Levenshtein编辑距离
    int CalculateEditDistance(const std::string& s1, const std::string& s2) const;
};

// 基于n-gram的相似度计算器
class NGramSimilarity : public LogSimilarityCalculator {
public:
    explicit NGramSimilarity(size_t n = 3) : _n(n) {}

    double CalculateSimilarity(const std::string& msg1, const std::string& msg2) const override;

private:
    size_t _n; // n-gram的n值

    // 提取n-gram特征
    std::unordered_map<std::string, int> ExtractNGramFeatures(const std::string& text) const;
};

// 日志合并处理器
class LogMerger : public Processor {
public:
    LogMerger();
    explicit LogMerger(const std::shared_ptr<Layout>& layout);
    ~LogMerger() = default;

    // 实现Processor接口
    bool ProcessRecord(Record& record) override;
    bool Flush() override;

    // 设置合并配置
    void SetMergeConfig(const LogMergeConfig& config) { _config = config; }
    LogMergeConfig GetMergeConfig() const { return _config; }

    // 设置相似度计算器
    void SetSimilarityCalculator(std::shared_ptr<LogSimilarityCalculator> calculator)
    { _similarity_calculator = calculator; }

    // 获取合并后的日志
    std::vector<MergedLogEntry> GetMergedLogs() const;

    // 注册自定义合并函数
    void RegisterCustomMergeFunction(std::function<bool(const Record&, const Record&)> func)
    { _custom_merge_function = func; }

private:
    // 合并配置
    LogMergeConfig _config;

    // 相似度计算器
    std::shared_ptr<LogSimilarityCalculator> _similarity_calculator;

    // 自定义合并函数
    std::function<bool(const Record&, const Record&)> _custom_merge_function;

    // 合并窗口内的日志
    std::unordered_map<std::string, MergedLogEntry> _merged_logs;
    mutable std::mutex _merged_logs_mutex;

    // 清理线程
    std::thread _cleanup_thread;
    std::atomic<bool> _cleanup_running;

    // 生成日志ID
    std::string GenerateLogID(const Record& record) const;

    // 提取调用链ID
    std::string ExtractCallChainID(const Record& record) const;

    // 检查两个日志是否相似
    bool AreLogsSimilar(const Record& record1, const Record& record2) const;

    // 合并两个日志
    bool MergeLogs(MergedLogEntry& target, const Record& source) const;

    // 清理过期的合并日志
    void CleanupExpiredLogs();

    // 清理线程函数
    void CleanupThread();

    // 生成合并后的日志消息
    std::string GenerateMergedMessage(const MergedLogEntry& entry) const;
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_LOG_MERGER_H