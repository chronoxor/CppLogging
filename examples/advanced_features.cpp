#include "logging/config.h"
#include "logging/logger.h"
#include "logging/processors/smart_classifier.h"
#include "logging/processors/dynamic_router.h"
#include "logging/processors/consistent_processor.h"
#include "logging/processors/load_balanced_processor.h"
#include "logging/processors/sensitive_data_processor.h"
#include "logging/processors/log_merger.h"
#include "logging/appenders/console_appender.h"
#include "logging/appenders/file_appender.h"
#include "logging/appenders/rolling_file_appender.h"
#include "logging/layouts/pattern_layout.h"

#include <iostream>
#include <thread>
#include <chrono>

using namespace CppLogging;

int main(int argc, char** argv)
{
    // 初始化日志配置
    auto config = std::make_shared<Config>();

    // 创建布局
    auto pattern_layout = std::make_shared<PatternLayout>("%d{%Y-%m-%d %H:%M:%S.%f} [%l] %t %n - %m");

    // 创建附加器
    auto console_appender = std::make_shared<ConsoleAppender>(pattern_layout);
    auto file_appender = std::make_shared<FileAppender>(pattern_layout, "logs/advanced_features.log");
    auto rolling_file_appender = std::make_shared<RollingFileAppender>(pattern_layout, "logs/rolling.log", "yyyy-MM-dd");

    // 创建智能分类器
    auto classifier = std::make_shared<SmartClassifier>(pattern_layout);

    // 加载分类规则
    std::string classification_rules = R"({
        "rules": [
            {
                "name": "payment_error",
                "pattern": "payment.*error|Error.*payment",
                "level": "ERROR",
                "targets": ["alerting", "encrypted_archive"],
                "priority": 10
            },
            {
                "name": "user_activity",
                "pattern": "user.*login|user.*logout|user.*register",
                "level": "INFO",
                "targets": ["analytics"],
                "priority": 5
            },
            {
                "name": "system_metrics",
                "pattern": "cpu.*usage|memory.*usage|disk.*usage",
                "level": "DEBUG",
                "targets": ["monitoring"],
                "priority": 3
            }
        ]
    })";
    classifier->LoadRulesFromJSON(classification_rules);

    // 创建动态路由器
    auto router = std::make_shared<DynamicRouter>(pattern_layout);

    // 注册路由目标
    router->AddRoute("alerting", console_appender);
    router->AddRoute("analytics", file_appender);
    router->AddRoute("monitoring", rolling_file_appender);

    // 创建分布式一致性处理器
    auto consistent_processor = std::make_shared<ConsistentProcessor>(pattern_layout, "node1", 10);

    // 创建负载均衡处理器
    auto load_balanced_processor = std::make_shared<LoadBalancedProcessor>(pattern_layout);

    // 配置负载均衡处理器
    load_balanced_processor->SetCPULoadThreshold(0.8);
    load_balanced_processor->SetIOLoadThreshold(0.8);
    load_balanced_processor->SetBufferUsageThreshold(0.8);
    load_balanced_processor->SetMinThreadPoolSize(2);
    load_balanced_processor->SetMaxThreadPoolSize(16);
    load_balanced_processor->SetMinBufferCapacity(1024 * 1024);
    load_balanced_processor->SetMaxBufferCapacity(32 * 1024 * 1024);

    // 创建敏感数据处理器
    auto sensitive_processor = std::make_shared<SensitiveDataProcessor>(pattern_layout);

    // 加载敏感数据规则
    std::string sensitive_rules = R"({
        "debug_mode": false,
        "global_strategy": "partial",
        "rules": [
            {
                "name": "phone",
                "pattern": "1[3-9]\\d{9}",
                "strategy": "partial",
                "replacement": "***",
                "priority": 10
            },
            {
                "name": "id_card",
                "pattern": "\\d{15}(?:\\d{2}[\\dXx])?",
                "strategy": "encrypt",
                "replacement": "***",
                "priority": 20
            },
            {
                "name": "bank_card",
                "pattern": "\\d{16,19}",
                "strategy": "partial",
                "replacement": "***",
                "priority": 15
            },
            {
                "name": "email",
                "pattern": "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}",
                "strategy": "partial",
                "replacement": "***",
                "priority": 5
            }
        ]
    })";
    sensitive_processor->LoadRulesFromJSON(sensitive_rules);

    // 创建日志合并器
    auto log_merger = std::make_shared<LogMerger>(pattern_layout);

    // 配置日志合并器
    LogMergeConfig merge_config;
    merge_config.merge_window = std::chrono::milliseconds(2000);
    merge_config.similarity_threshold = 0.8;
    merge_config.max_duplicates = 100;
    merge_config.preserve_call_chain = true;
    log_merger->SetMergeConfig(merge_config);

    // 构建处理器链
    classifier->AddAppender(router);
    router->AddProcessor(consistent_processor);
    consistent_processor->AddProcessor(load_balanced_processor);
    load_balanced_processor->AddProcessor(sensitive_processor);
    sensitive_processor->AddProcessor(log_merger);
    log_merger->AddAppender(console_appender);

    // 配置日志记录器
    config->RegisterAppender("console", console_appender);
    config->RegisterAppender("file", file_appender);
    config->RegisterAppender("rolling", rolling_file_appender);
    config->RegisterProcessor("classifier", classifier);
    config->RegisterProcessor("router", router);
    config->RegisterProcessor("consistent", consistent_processor);
    config->RegisterProcessor("load_balanced", load_balanced_processor);
    config->RegisterProcessor("sensitive", sensitive_processor);
    config->RegisterProcessor("merger", log_merger);
    config->SetDefaultAppender("console");
    config->SetDefaultProcessor("classifier");

    // 初始化日志系统
    Logger::Config(config);

    // 获取日志记录器
    auto logger = Logger::Get("advanced_features");

    // 创建多个线程生成日志
    auto log_generator = [&logger](int thread_id) {
        for (int i = 0; i < 100; ++i)
        {
            // 生成不同类型的日志
            if (i % 10 == 0)
            {
                // 支付错误日志（应该被路由到alerting和encrypted_archive）
                logger.Error("Payment error: transaction failed for user 12345, amount $100.00, card 1234-5678-9012-3456");
            }
            else if (i % 10 == 1)
            {
                // 用户活动日志（应该被路由到analytics）
                logger.Info("User login: username john_doe, ip 192.168.1.100, phone 138-0013-8000");
            }
            else if (i % 10 == 2)
            {
                // 系统指标日志（应该被路由到monitoring）
                logger.Debug("System metrics: cpu usage 45%, memory usage 68%, disk usage 72%");
            }
            else if (i % 10 == 3)
            {
                // 包含敏感数据的日志（应该被脱敏）
                logger.Info("User profile: name John Doe, id card 110101199001011234, email john.doe@example.com");
            }
            else
            {
                // 普通日志
                logger.Info("Normal log message from thread {}, iteration {}", thread_id, i);
            }

            // 等待随机时间
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100));
        }
    };

    // 启动多个日志生成线程
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i)
        threads.emplace_back(log_generator, i);

    // 等待所有线程完成
    for (auto& thread : threads)
        thread.join();

    // 刷新所有日志处理器
    Logger::Flush();

    // 关闭日志系统
    Logger::Shutdown();

    return 0;
}