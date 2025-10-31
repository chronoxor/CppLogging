#ifndef CPPLOGGING_PROCESSORS_DYNAMIC_ROUTER_H
#define CPPLOGGING_PROCESSORS_DYNAMIC_ROUTER_H

#include "logging/processor.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace CppLogging {

class DynamicRouter : public Processor {
public:
    DynamicRouter();
    explicit DynamicRouter(const std::shared_ptr<Layout>& layout);

    // 实现Processor接口
    bool ProcessRecord(Record& record) override;

    // 添加路由映射
    void AddRoute(const std::string& target, const std::shared_ptr<Appender>& appender);
    void AddRoute(const std::string& target, const std::shared_ptr<Processor>& processor);

    // 移除路由映射
    void RemoveRoute(const std::string& target);

    // 清除所有路由
    void ClearRoutes();

private:
    std::unordered_map<std::string, std::vector<std::shared_ptr<Appender>>> _appender_routes;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Processor>>> _processor_routes;
    std::mutex _mutex;

    // 从Record中提取目标信息
    std::vector<std::string> ExtractTargets(const Record& record);
};

} // namespace CppLogging

#endif // CPPLOGGING_PROCESSORS_DYNAMIC_ROUTER_H