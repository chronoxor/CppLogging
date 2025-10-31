#include "logging/processors/consistent_processor.h"
#include "threads/thread.h"
#include "errors/fatal.h"
#include <cassert>
#include <chrono>

namespace CppLogging {

ConsistentProcessor::ConsistentProcessor()
    : Processor(nullptr),
      _node_id(0),
      _sequence_number(0),
      _max_out_of_order_window(100), // 默认100毫秒的乱序窗口
      _duplicate_detection_window(1000), // 默认1秒的重复检测窗口
      _running(false)
{
}

ConsistentProcessor::ConsistentProcessor(const std::shared_ptr<Layout>& layout)
    : Processor(layout),
      _node_id(0),
      _sequence_number(0),
      _max_out_of_order_window(100),
      _duplicate_detection_window(1000),
      _running(false)
{
}

ConsistentProcessor::~ConsistentProcessor()
{
    Stop();
}

bool ConsistentProcessor::Start()
{
    bool started = IsStarted();

    if (!Processor::Start())
        return false;

    if (!started)
    {
        _running = true;
        _processing_thread = CppCommon::Thread::Start([this]() { ProcessingThread(); });
    }

    return true;
}

bool ConsistentProcessor::Stop()
{
    if (IsStarted())
    {
        _running = false;
        _queue_condition.notify_all();

        if (_processing_thread.joinable())
            _processing_thread.join();
    }

    return Processor::Stop();
}

bool ConsistentProcessor::ProcessRecord(Record& record)
{
    // 检查是否已经启动
    if (!IsStarted())
        return true;

    // 为日志记录生成全局序列标识
    LogSequence sequence;
    sequence.node_id = _node_id;
    sequence.sequence_number = ++_sequence_number;
    sequence.timestamp = record.timestamp;
    sequence.record = std::make_shared<Record>(std::move(record));

    // 序列化序列信息到日志记录
    SerializeLogSequence(*sequence.record, sequence);

    // 将日志记录加入有序队列
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        _ordered_queue.push(sequence);
        _queue_condition.notify_one();
    }

    return true;
}

void ConsistentProcessor::Flush()
{
    // 检查是否已经启动
    if (!IsStarted())
        return;

    // 等待队列处理完成
    std::unique_lock<std::mutex> lock(_queue_mutex);
    _queue_condition.wait(lock, [this]() { return _ordered_queue.empty() || !_running; });

    // 刷新所有子处理器和appenders
    Processor::Flush();
}

void ConsistentProcessor::ProcessingThread()
{
    try
    {
        auto last_cleanup_time = std::chrono::steady_clock::now();

        while (_running)
        {
            std::unique_lock<std::mutex> lock(_queue_mutex);

            // 等待队列中有数据或停止信号
            _queue_condition.wait_for(lock, std::chrono::milliseconds(100),
                [this]() { return !_ordered_queue.empty() || !_running; });

            if (!_running)
                break;

            if (_ordered_queue.empty())
                continue;

            // 获取最早的日志记录
            LogSequence current = _ordered_queue.top();
            _ordered_queue.pop();

            lock.unlock();

            // 检查是否为重复记录
            if (IsDuplicate(current))
                continue;

            // 检查是否在乱序窗口内
            auto now = std::chrono::steady_clock::now();
            auto record_time = std::chrono::system_clock::from_time_t(current.timestamp / 1000000000);
            auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - record_time);

            if (time_diff.count() > _max_out_of_order_window)
            {
                // 日志记录已过期，直接处理
                ProcessOrderedRecord(current);
            }
            else
            {
                // 日志记录在乱序窗口内，暂时存储
                lock.lock();
                _received_sequences[current.node_id].emplace(current.sequence_number, current.timestamp);
                lock.unlock();

                // 检查是否可以处理连续的日志记录
                ProcessContinuousSequences();
            }

            // 定期清理重复检测缓存
            auto cleanup_diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_cleanup_time);
            if (cleanup_diff.count() > _duplicate_detection_window)
            {
                CleanupDuplicateCache();
                last_cleanup_time = now;
            }
        }

        // 处理队列中剩余的日志记录
        std::lock_guard<std::mutex> lock(_queue_mutex);
        while (!_ordered_queue.empty())
        {
            LogSequence current = _ordered_queue.top();
            _ordered_queue.pop();

            if (!IsDuplicate(current))
                ProcessOrderedRecord(current);
        }
    }
    catch (const std::exception& ex)
    {
        fatality(ex);
    }
    catch (...)
    {
        fatality("Consistent logging processor terminated!");
    }
}

void ConsistentProcessor::ProcessOrderedRecord(const LogSequence& sequence)
{
    // 反序列化日志记录
    Record record = *sequence.record;

    // 处理日志记录（应用filters、layouts、appenders等）
    Processor::ProcessRecord(record);
}

void ConsistentProcessor::ProcessContinuousSequences()
{
    std::lock_guard<std::mutex> lock(_queue_mutex);

    for (auto& [node_id, sequences] : _received_sequences)
    {
        uint64_t expected_sequence = _last_sequence[node_id] + 1;

        while (!sequences.empty())
        {
            auto [sequence_number, timestamp] = sequences.front();

            if (sequence_number == expected_sequence)
            {
                // 找到连续的序列号，处理该日志记录
                LogSequence sequence;
                sequence.node_id = node_id;
                sequence.sequence_number = sequence_number;
                sequence.timestamp = timestamp;

                // 从队列中移除
                sequences.pop();

                // 更新最后处理的序列号
                _last_sequence[node_id] = sequence_number;

                // 处理日志记录
                lock.unlock();
                ProcessOrderedRecord(sequence);
                lock.lock();

                expected_sequence++;
            }
            else if (sequence_number < expected_sequence)
            {
                // 序列号已经被处理过，可能是重复记录
                sequences.pop();
            }
            else
            {
                // 序列号不连续，等待后续记录
                break;
            }
        }
    }
}

void ConsistentProcessor::SerializeLogSequence(Record& record, const LogSequence& sequence)
{
    // 将序列信息存储在record的buffer中
    // 格式：node_id(8 bytes) + sequence_number(8 bytes) + timestamp(8 bytes)
    record.buffer.resize(record.buffer.size() + 24);
    std::memcpy(record.buffer.data() + record.buffer.size() - 24, &sequence.node_id, 8);
    std::memcpy(record.buffer.data() + record.buffer.size() - 16, &sequence.sequence_number, 8);
    std::memcpy(record.buffer.data() + record.buffer.size() - 8, &sequence.timestamp, 8);
}

LogSequence ConsistentProcessor::DeserializeLogSequence(const Record& record)
{
    LogSequence sequence;
    if (record.buffer.size() >= 24)
    {
        std::memcpy(&sequence.node_id, record.buffer.data() + record.buffer.size() - 24, 8);
        std::memcpy(&sequence.sequence_number, record.buffer.data() + record.buffer.size() - 16, 8);
        std::memcpy(&sequence.timestamp, record.buffer.data() + record.buffer.size() - 8, 8);
    }
    return sequence;
}

bool ConsistentProcessor::IsDuplicate(const LogSequence& sequence)
{
    std::lock_guard<std::mutex> lock(_queue_mutex);

    // 检查是否已经处理过该序列号
    if (_last_sequence[sequence.node_id] >= sequence.sequence_number)
        return true;

    // 检查重复检测缓存
    auto& node_cache = _duplicate_cache[sequence.node_id];
    if (node_cache.find(sequence.sequence_number) != node_cache.end())
        return true;

    // 将序列号加入重复检测缓存
    node_cache[sequence.sequence_number] = true;
    return false;
}

void ConsistentProcessor::CleanupDuplicateCache()
{
    std::lock_guard<std::mutex> lock(_queue_mutex);

    for (auto& [node_id, node_cache] : _duplicate_cache)
    {
        uint64_t last_sequence = _last_sequence[node_id];

        // 移除所有小于等于最后处理序列号的缓存条目
        auto it = node_cache.begin();
        while (it != node_cache.end())
        {
            if (it->first <= last_sequence)
                it = node_cache.erase(it);
            else
                ++it;
        }

        // 如果缓存为空，移除该节点的缓存
        if (node_cache.empty())
            _duplicate_cache.erase(node_id);
    }
}

} // namespace CppLogging