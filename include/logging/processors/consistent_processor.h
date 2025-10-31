#ifndef CPPLOGGING_PROCESSORS_CONSISTENT_PROCESSOR_H
#define CPPLOGGING_PROCESSORS_CONSISTENT_PROCESSOR_H

#include "logging/processor.h"
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace CppLogging {

struct LogSequence {
    uint64_t node_id;
    uint64_t sequence_number;
    uint64_t timestamp;
    std::shared_ptr<Record> record;

    // 比较运算符，用于排序
    bool operator<(const LogSequence& other) const {
        // 首先按时间戳排序
        if (timestamp != other.timestamp)
            return timestamp > other.timestamp;
        // 时间戳相同则按节点ID排序
        if (node_id != other.node_id)
            return node_id > other.node_id;
        // 最后按序列号排序
        return sequence_number > other.sequence_number;
    }
};

class ConsistentProcessor : public Processor {
public:
    ConsistentProcessor();
    explicit ConsistentProcessor(const std::shared_ptr<Layout>& layout);
    ~ConsistentProcessor();

    // 实现Processor接口
    bool Start() override;
    bool Stop() override;
    bool ProcessRecord(Record& record) override;
    void Flush() override;

    // 设置节点ID
    void SetNodeId(uint64_t node_id) { _node_id = node_id; }

    // 设置最大允许的乱序时间窗口（毫秒）
    void SetMaxOutOfOrderWindow(int64_t window_ms) { _max_out_of_order_window = window_ms; }

    // 设置重复检测的时间窗口（毫秒）
    void SetDuplicateDetectionWindow(int64_t window_ms) { _duplicate_detection_window = window_ms; }

private:
    uint64_t _node_id; // 当前节点ID
    std::atomic<uint64_t> _sequence_number; // 本地序列号
    int64_t _max_out_of_order_window; // 最大允许的乱序时间窗口（毫秒）
    int64_t _duplicate_detection_window; // 重复检测的时间窗口（毫秒）

    std::priority_queue<LogSequence> _ordered_queue; // 有序队列
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, uint64_t>> _last_sequence; // 每个节点的最后序列号
    std::unordered_map<uint64_t, std::queue<std::pair<uint64_t, uint64_t>>> _received_sequences; // 已接收的序列号
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, bool>> _duplicate_cache; // 重复检测缓存

    std::thread _processing_thread; // 处理线程
    std::mutex _queue_mutex; // 队列互斥锁
    std::condition_variable _queue_condition; // 队列条件变量
    std::atomic<bool> _running; // 运行状态

    // 处理线程函数
    void ProcessingThread();

    // 序列化日志序列到Record
    void SerializeLogSequence(Record& record, const LogSequence& sequence);

    // 从Record反序列化日志序列
    LogSequence DeserializeLogSequence(const Record& record);

    // 检查是否为重复记录
    bool IsDuplicate(const LogSequence& sequence);

    // 清理过期的重复检测缓存
    void CleanupDuplicateCache();
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_CONSISTENT_PROCESSOR_H