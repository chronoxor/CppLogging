#include "logging/processors/load_balanced_processor.h"
#include "logging/logger.h"
#include <cmath>
#include <algorithm>
#include <psapi.h>
#include <pdh.h>

// 链接PDH库
#pragma comment(lib, "pdh.lib")

namespace CppLogging {

LoadBalancedProcessor::LoadBalancedProcessor()
    : AsyncWaitProcessor()
    , _monitoring_running(false)
    , _monitoring_interval(std::chrono::milliseconds(5000)) // 5秒监控一次
    , _prediction_window_size(60) // 60个数据点（5分钟）
    , _cpu_load_threshold(0.8) // CPU使用率超过80%则调整
    , _io_load_threshold(0.8)  // IO使用率超过80%则调整
    , _buffer_usage_threshold(0.8) // 缓冲区使用率超过80%则调整
    , _min_thread_pool_size(2) // 线程池最小2个线程
    , _max_thread_pool_size(32) // 线程池最大32个线程
    , _min_buffer_capacity(1024 * 1024) // 缓冲区最小1MB
    , _max_buffer_capacity(64 * 1024 * 1024) // 缓冲区最大64MB
    , _prediction_threshold(0.1) // 预测变化超过10%则调整
    , _last_adjustment_time(0)
    , _adjustment_count(0)
{
}

LoadBalancedProcessor::LoadBalancedProcessor(const std::shared_ptr<Layout>& layout)
    : AsyncWaitProcessor(layout)
    , _monitoring_running(false)
    , _monitoring_interval(std::chrono::milliseconds(5000)) // 5秒监控一次
    , _prediction_window_size(60) // 60个数据点（5分钟）
    , _cpu_load_threshold(0.8) // CPU使用率超过80%则调整
    , _io_load_threshold(0.8)  // IO使用率超过80%则调整
    , _buffer_usage_threshold(0.8) // 缓冲区使用率超过80%则调整
    , _min_thread_pool_size(2) // 线程池最小2个线程
    , _max_thread_pool_size(32) // 线程池最大32个线程
    , _min_buffer_capacity(1024 * 1024) // 缓冲区最小1MB
    , _max_buffer_capacity(64 * 1024 * 1024) // 缓冲区最大64MB
    , _prediction_threshold(0.1) // 预测变化超过10%则调整
    , _last_adjustment_time(0)
    , _adjustment_count(0)
{
}

LoadBalancedProcessor::~LoadBalancedProcessor()
{
    Stop();
}

bool LoadBalancedProcessor::Start()
{
    if (!AsyncWaitProcessor::Start())
        return false;

    // 启动监控线程
    _monitoring_running = true;
    _monitoring_thread = std::thread(&LoadBalancedProcessor::MonitoringThread, this);

    return true;
}

bool LoadBalancedProcessor::Stop()
{
    // 停止监控线程
    _monitoring_running = false;
    if (_monitoring_thread.joinable())
        _monitoring_thread.join();

    return AsyncWaitProcessor::Stop();
}

void LoadBalancedProcessor::MonitoringThread()
{
    while (_monitoring_running)
    {
        try
        {
            // 收集负载指标
            LoadMetrics metrics = CollectLoadMetrics();

            // 添加到历史记录
            {n
                std::lock_guard<std::mutex> lock(_load_history_mutex);
                _load_history.push_back(metrics);
                // 保持历史记录大小在预测窗口内
                if (_load_history.size() > _prediction_window_size)
                    _load_history.erase(_load_history.begin());
            }

            // 足够的数据点时进行预测
            if (_load_history.size() >= _prediction_window_size)
            {
                LoadPrediction prediction = PredictLoadTrend();
                AdjustResources(prediction);
            }
        }
        catch (const std::exception& ex)
        {
            // 记录监控错误
            CppLogging::Logger::Error("Load balancing processor monitoring error: {}", ex.what());
        }

        // 等待下一次监控
        std::this_thread::sleep_for(_monitoring_interval);
    }
}

LoadMetrics LoadBalancedProcessor::CollectLoadMetrics()
{
    LoadMetrics metrics;
    metrics.timestamp = std::chrono::system_clock::now();

    // 获取CPU使用率
    static FILETIME prev_idle_time;
    static FILETIME prev_kernel_time;
    static FILETIME prev_user_time;
    static bool first_time = true;

    FILETIME idle_time, kernel_time, user_time;
    if (GetSystemTimes(&idle_time, &kernel_time, &user_time))
    {
        if (!first_time)
        {
            ULONGLONG idle = (ULONGLONG(idle_time.dwHighDateTime) << 32) | idle_time.dwLowDateTime;
            ULONGLONG kernel = (ULONGLONG(kernel_time.dwHighDateTime) << 32) | kernel_time.dwLowDateTime;
            ULONGLONG user = (ULONGLONG(user_time.dwHighDateTime) << 32) | user_time.dwLowDateTime;

            ULONGLONG prev_idle = (ULONGLONG(prev_idle_time.dwHighDateTime) << 32) | prev_idle_time.dwLowDateTime;
            ULONGLONG prev_kernel = (ULONGLONG(prev_kernel_time.dwHighDateTime) << 32) | prev_kernel_time.dwLowDateTime;
            ULONGLONG prev_user = (ULONGLONG(prev_user_time.dwHighDateTime) << 32) | prev_user_time.dwLowDateTime;

            ULONGLONG total = (kernel - prev_kernel) + (user - prev_user);
            ULONGLONG idle_diff = idle - prev_idle;

            if (total > 0)
                metrics.cpu_usage = 1.0 - (double(idle_diff) / total);
            else
                metrics.cpu_usage = 0.0;
        }

        prev_idle_time = idle_time;
        prev_kernel_time = kernel_time;
        prev_user_time = user_time;
        first_time = false;
    }
    else
    {
        metrics.cpu_usage = 0.0;
    }

    // 获取IO使用率（磁盘使用率作为近似）
    PDH_HQUERY query = nullptr;
    PDH_HCOUNTER counter = nullptr;
    DWORD counter_type = 0;
    PDH_FMT_COUNTERVALUE counter_value;

    if (PdhOpenQuery(nullptr, 0, &query) == ERROR_SUCCESS)
    {
        if (PdhAddCounterA(query, "\\PhysicalDisk(_Total)\\% Disk Time", 0, &counter) == ERROR_SUCCESS)
        {
            if (PdhCollectQueryData(query) == ERROR_SUCCESS)
            {
                if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, &counter_type, &counter_value) == ERROR_SUCCESS)
                {
                    metrics.io_usage = counter_value.doubleValue / 100.0;
                    // 限制在0.0-1.0范围内
                    metrics.io_usage = std::max(0.0, std::min(1.0, metrics.io_usage));
                }
            }
        }

        PdhCloseQuery(query);
    }
    else
    {
        metrics.io_usage = 0.0;
    }

    // 获取队列大小
    metrics.queue_size = _queue.size();

    // 获取缓冲区使用率
    size_t buffer_size = _buffer.capacity();
    if (buffer_size > 0)
        metrics.buffer_usage = double(_buffer.size()) / buffer_size;
    else
        metrics.buffer_usage = 0.0;

    // 获取日志速率（条/秒）
    static uint64_t prev_log_count = 0;
    static std::chrono::system_clock::time_point prev_time = std::chrono::system_clock::now();

    uint64_t current_log_count = _processed_records;
    auto current_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - prev_time).count();

    if (duration > 0)
    {
        metrics.log_rate = (current_log_count - prev_log_count) / duration;
        prev_log_count = current_log_count;
        prev_time = current_time;
    }
    else
    {
        metrics.log_rate = 0;
    }

    return metrics;
}

LoadPrediction LoadBalancedProcessor::PredictLoadTrend()
{
    LoadPrediction prediction;
    prediction.predicted_cpu = 0.0;
    prediction.predicted_io = 0.0;
    prediction.predicted_log_rate = 0;
    prediction.is_peak = false;

    std::lock_guard<std::mutex> lock(_load_history_mutex);
    if (_load_history.size() < _prediction_window_size)
        return prediction;

    // 使用简单的线性回归模型预测负载趋势
    auto predict_trend = [](const std::vector<double>& values) -> double {
        size_t n = values.size();
        if (n < 2)
            return 0.0;

        double sum_x = 0.0;
        double sum_y = 0.0;
        double sum_xy = 0.0;
        double sum_x2 = 0.0;

        for (size_t i = 0; i < n; ++i)
        {
            double x = static_cast<double>(i);
            double y = values[i];

            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }

        double denominator = n * sum_x2 - sum_x * sum_x;
        if (denominator == 0.0)
            return 0.0;

        double slope = (n * sum_xy - sum_x * sum_y) / denominator;
        return slope;
    };

    // 提取CPU、IO和日志速率的历史数据
    std::vector<double> cpu_values;
    std::vector<double> io_values;
    std::vector<uint64_t> log_rate_values;

    for (const auto& metrics : _load_history)
    {
        cpu_values.push_back(metrics.cpu_usage);
        io_values.push_back(metrics.io_usage);
        log_rate_values.push_back(metrics.log_rate);
    }

    // 计算指数移动平均值（EMA）以平滑数据
    double ema_cpu = CalculateEMA(cpu_values);
    double ema_io = CalculateEMA(io_values);
    double ema_log_rate = CalculateEMA(std::vector<double>(log_rate_values.begin(), log_rate_values.end()));

    // 预测未来趋势
    double cpu_trend = predict_trend(cpu_values);
    double io_trend = predict_trend(io_values);
    double log_rate_trend = predict_trend(std::vector<double>(log_rate_values.begin(), log_rate_values.end()));

    // 预测未来值（简单线性外推）
    prediction.predicted_cpu = ema_cpu + cpu_trend * 2; // 预测未来2个时间点（10秒后）
    prediction.predicted_io = ema_io + io_trend * 2;
    prediction.predicted_log_rate = static_cast<uint64_t>(ema_log_rate + log_rate_trend * 2);

    // 限制在合理范围内
    prediction.predicted_cpu = std::max(0.0, std::min(1.0, prediction.predicted_cpu));
    prediction.predicted_io = std::max(0.0, std::min(1.0, prediction.predicted_io));
    prediction.predicted_log_rate = std::max(static_cast<uint64_t>(0), prediction.predicted_log_rate);

    // 判断是否为峰值
    double current_cpu = cpu_values.back();
    double current_io = io_values.back();
    uint64_t current_log_rate = log_rate_values.back();

    // 如果预测值比当前值高很多，则认为是峰值
    if (prediction.predicted_cpu > current_cpu * (1 + _prediction_threshold) &&
        prediction.predicted_io > current_io * (1 + _prediction_threshold) &&
        prediction.predicted_log_rate > current_log_rate * (1 + _prediction_threshold))
    {
        prediction.is_peak = true;
    }

    return prediction;
}

void LoadBalancedProcessor::AdjustResources(const LoadPrediction& prediction)
{
    // 检查调整频率（1分钟内不超过2次）
    size_t current_time = GetCurrentTimeSeconds();
    if (current_time - _last_adjustment_time < 60)
    {
        if (_adjustment_count >= 2)
            return; // 超过调整次数限制
    }
    else
    {
        // 重置调整计数
        _adjustment_count = 0;
    }

    bool adjusted = false;
    size_t new_thread_pool_size = _thread_pool.size();
    size_t new_buffer_capacity = _buffer.capacity();

    // 根据预测结果调整线程池大小
    if (prediction.predicted_cpu > _cpu_load_threshold || prediction.predicted_io > _io_load_threshold || prediction.is_peak)
    {
        // 负载过高，增加线程池大小
        if (new_thread_pool_size < _max_thread_pool_size)
        {
            new_thread_pool_size = std::min(new_thread_pool_size * 2, _max_thread_pool_size);
            adjusted = true;
        }

        // 增加缓冲区容量
        if (new_buffer_capacity < _max_buffer_capacity)
        {
            new_buffer_capacity = std::min(new_buffer_capacity * 2, _max_buffer_capacity);
            adjusted = true;
        }
    }
    else if (prediction.predicted_cpu < _cpu_load_threshold * 0.5 && prediction.predicted_io < _io_load_threshold * 0.5)
    {
        // 负载过低，减少线程池大小
        if (new_thread_pool_size > _min_thread_pool_size)
        {
            new_thread_pool_size = std::max(new_thread_pool_size / 2, _min_thread_pool_size);
            adjusted = true;
        }

        // 减少缓冲区容量
        if (new_buffer_capacity > _min_buffer_capacity)
        {
            new_buffer_capacity = std::max(new_buffer_capacity / 2, _min_buffer_capacity);
            adjusted = true;
        }
    }

    // 应用资源调整
    if (adjusted)
    {
        if (new_thread_pool_size != _thread_pool.size())
        {
            // 调整线程池大小
            _thread_pool.resize(new_thread_pool_size);
            CppLogging::Logger::Info("Load balancing: Adjusted thread pool size from {} to {}", _thread_pool.size(), new_thread_pool_size);
        }

        if (new_buffer_capacity != _buffer.capacity())
        {
            // 调整缓冲区容量
            _buffer.reserve(new_buffer_capacity);
            CppLogging::Logger::Info("Load balancing: Adjusted buffer capacity from {} to {}", _buffer.capacity(), new_buffer_capacity);
        }

        // 更新调整时间和计数
        _last_adjustment_time = current_time;
        _adjustment_count++;
    }
}

size_t LoadBalancedProcessor::GetCurrentTimeSeconds() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

double LoadBalancedProcessor::CalculateEMA(const std::vector<double>& values, double alpha) const
{
    if (values.empty())
        return 0.0;

    double ema = values[0];
    for (size_t i = 1; i < values.size(); ++i)
    {
        ema = alpha * values[i] + (1 - alpha) * ema;
    }
    return ema;
}

} // namespace CppLogging