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

#include <atomic>
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
         \param positive - Positive filtration (default is true)
    */
    explicit MessageFilter(const std::regex& pattern, bool positive = true) : _positive(positive), _pattern(pattern) {}
    MessageFilter(const MessageFilter&) = delete;
    MessageFilter(MessageFilter&&) = delete;
    virtual ~MessageFilter() = default;

    MessageFilter& operator=(const MessageFilter&) = delete;
    MessageFilter& operator=(MessageFilter&&) = delete;

    //! Get the positive filtration flag
    bool positive() const noexcept { return _positive; }

    //! Get the message regular expression pattern
    const std::regex& pattern() const noexcept { return _pattern; }

    // Implementation of Filter
    bool FilterRecord(Record& record) override;

private:
    std::atomic<bool> _positive;
    std::regex _pattern;
};

} // namespace CppLogging

#endif // CPPLOGGING_FILTERS_MESSAGE_FILTER_H
