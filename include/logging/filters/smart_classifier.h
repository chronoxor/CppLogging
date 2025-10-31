#ifndef CPPLOGGING_FILTERS_SMART_CLASSIFIER_H
#define CPPLOGGING_FILTERS_SMART_CLASSIFIER_H

#include "logging/filter.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <mutex>

namespace CppLogging {

class SmartClassifier : public Filter {
public:
    struct ClassificationRule {
        int priority;
        Level level;
        std::unordered_set<std::string> keywords;
        std::unordered_set<std::string> modules;
        std::string fingerprint;
        std::vector<std::string> targets;
    };

    SmartClassifier();
    explicit SmartClassifier(const std::string& config_file);

    bool FilterRecord(Record& record) override;
    void LoadRules(const std::string& config_file);
    void ReloadRules();
    void AddRule(const ClassificationRule& rule);
    void ClearRules();

private:
    std::vector<ClassificationRule> _rules;
    std::unordered_map<std::string, std::vector<ClassificationRule>> _keyword_index;
    std::unordered_map<std::string, std::vector<ClassificationRule>> _module_index;
    std::string _config_file;
    std::mutex _mutex;

    void IndexRules();
    std::string CalculateExceptionFingerprint(const Record& record);
};

} // namespace CppLogging

#endif // CPPLOGGING_FILTERS_SMART_CLASSIFIER_H