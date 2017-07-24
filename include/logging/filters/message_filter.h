/*!
    \file message_filter.h
    \brief Message filter definition
    \author Ivan Shynkarenka
    \date 27.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_FILTERS_MESSAGE_FILTER_H
#define CPPLOGGING_FILTERS_MESSAGE_FILTER_H

#include "logging/filter.h"

#include <regex>

namespace CppLogging {

//! Message filter
/*!
    Message filters out logging records which message field is not matched
    to the given regular expression pattern.

    Thread-safe.
*/
class MessageFilter : public Filter
{
public:
    //! Initialize message filter with a given regular expression pattern
    /*!
         \param pattern - Regular expression pattern
    */
    explicit MessageFilter(const std::regex& pattern) : _pattern(pattern) {}
    MessageFilter(const MessageFilter&) = delete;
    MessageFilter(MessageFilter&&) noexcept = default;
    virtual ~MessageFilter() = default;

    MessageFilter& operator=(const MessageFilter&) = delete;
    MessageFilter& operator=(MessageFilter&&) noexcept = default;

    // Implementation of Filter
    bool FilterRecord(Record& record) override;

private:
    std::regex _pattern;
};

} // namespace CppLogging

#endif // CPPLOGGING_FILTERS_MESSAGE_FILTER_H
