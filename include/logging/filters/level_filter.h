/*!
    \file level_filter.h
    \brief Level filter definition
    \author Ivan Shynkarenka
    \date 27.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_FILTERS_LEVEL_FILTER_H
#define CPPLOGGING_FILTERS_LEVEL_FILTER_H

#include "logging/filter.h"

namespace CppLogging {

//! Level filter
/*!
    Level filters out logging records which level filed is not matched
    to the given level value or range.

    Thread-safe.
*/
class LevelFilter : public Filter
{
public:
    //! Initialize level filter with a given level value
    /*!
         \param level - Level value
    */
    explicit LevelFilter(Level level) : _from(level), _to(level) {}
    //! Initialize level filter with a given level range
    /*!
         \param from - Level from value
         \param to - Level to value
    */
    explicit LevelFilter(Level from, Level to) : _from(from), _to(to) {}
    LevelFilter(const LevelFilter&) = delete;
    LevelFilter(LevelFilter&&) noexcept = default;
    virtual ~LevelFilter() = default;

    LevelFilter& operator=(const LevelFilter&) = delete;
    LevelFilter& operator=(LevelFilter&&) noexcept = default;

    // Implementation of Filter
    bool FilterRecord(Record& record) override;

private:
    Level _from;
    Level _to;
};

} // namespace CppLogging

#endif // CPPLOGGING_FILTERS_LEVEL_FILTER_H
