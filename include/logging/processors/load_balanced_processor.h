#ifndef CPPLOGGING_PROCESSORS_LOAD_BALANCED_PROCESSOR_H
#define CPPLOGGING_PROCESSORS_LOAD_BALANCED_PROCESSOR_H

#include "logging/processors/async_wait_processor.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>

namespace CppLogging {

struct LoadMetrics {
    double cpu_usage; // CPU使用率 (0.0 - 1.0)
    double io_usage;  // IO使用率 (0.0 - 1.0)
    size_t queue_size; // 队列大小
    size_t buffer_usage; // 缓冲区使用率 (0.0 - 1.0)
    uint64_t log_rate; // 日志速率 (条/秒)
    std::chrono::system_clock::time_point timestamp; // 时间戳
};

struct LoadPrediction {
    double predicted_cpu; // 预测的CPU使用率 (0.0 - 1.0)
    double predicted_io;  // 预测的IO使用率 (0.0 - 1.0)
    uint64_t predicted_log_rate; // 预测的日志速率 (条/秒)
    bool is_peak; // 是否为峰值
};

class LoadBalancedProcessor : public AsyncWaitProcessor {
public:
    LoadBalancedProcessor();
    explicit LoadBalancedProcessor(const std::shared_ptr<Layout>& layout);
    ~LoadBalancedProcessor();

    // 实现Processor接口
    bool Start() override;
    bool Stop() override;

    // 设置负载阈值
    void SetCPULoadThreshold(double threshold) { _cpu_load_threshold = threshold; }
    void SetIOLoadThreshold(double threshold) { _io_load_threshold = threshold; }
    void SetBufferUsageThreshold(double threshold) { _buffer_usage_threshold = threshold; }

    // 设置资源调整参数
    void SetMinThreadPoolSize(size_t min_size) { _min_thread_pool_size = min_size; }
    void SetMaxThreadPoolSize(size_t max_size) { _max_thread_pool_size = max_size; }
    void SetMinBufferCapacity(size_t min_capacity) { _min_buffer_capacity = min_capacity; }
    void SetMaxBufferCapacity(size_t max_capacity) { _max_buffer_capacity = max_capacity; }

    // 设置预测参数
    void SetPredictionWindowSize(size_t window_size) { _prediction_window_size = window_size; }
    void SetPredictionThreshold(double threshold) { _prediction_threshold = threshold; }

private:
    // 负载监控线程
    std::thread _monitoring_thread;
    std::atomic<bool> _monitoring_running;
    std::chrono::milliseconds _monitoring_interval; // 监控间隔

    // 负载指标历史记录
    std::vector<LoadMetrics> _load_history;
    std::mutex _load_history_mutex;
    size_t _prediction_window_size; // 预测窗口大小

    // 负载阈值
    double _cpu_load_threshold; // CPU负载阈值 (超过该值则调整资源)
    double _io_load_threshold;  // IO负载阈值
    double _buffer_usage_threshold; // 缓冲区使用率阈值

    // 资源配置范围
    size_t _min_thread_pool_size; // 线程池最小大小
    size_t _max_thread_pool_size; // 线程池最大大小
    size_t _min_buffer_capacity; // 缓冲区最小容量
    size_t _max_buffer_capacity; // 缓冲区最大容量

    // 预测参数
    double _prediction_threshold; // 预测阈值
    std::atomic<size_t> _last_adjustment_time; // 上次调整时间 (秒)
    std::atomic<size_t> _adjustment_count; // 1分钟内的调整次数

    // 监控线程函数
    void MonitoringThread();

    // 收集负载指标
    LoadMetrics CollectLoadMetrics();

    // 预测负载趋势
    LoadPrediction PredictLoadTrend();

    // 调整资源配置
    void AdjustResources(const LoadPrediction& prediction);

    // 获取当前时间（秒）
    size_t GetCurrentTimeSeconds() const;

    // 计算指数移动平均值
    double CalculateEMA(const std::vector<double>& values, double alpha = 0.3) const;
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_LOAD_BALANCED_PROCESSOR_H