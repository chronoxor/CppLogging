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

#include <atomic>

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
    explicit LevelFilter(Level level) { Update(level); }
    //! Initialize level filter with a given level range
    /*!
         \param from - Level from value
         \param to - Level to value
    */
    explicit LevelFilter(Level from, Level to) { Update(from, to); }
    LevelFilter(const LevelFilter&) = delete;
    LevelFilter(LevelFilter&&) = default;
    virtual ~LevelFilter() = default;

    LevelFilter& operator=(const LevelFilter&) = delete;
    LevelFilter& operator=(LevelFilter&&) = default;

    //! Get Level from value
    Level from() const noexcept { return _from; }
    //! Get Level to value
    Level to() const noexcept { return _to; }

    //! Update level filter with a given level value
    /*!
         \param level - Level value
    */
    void Update(Level level);
    //! Update level filter with a given level range
    /*!
         \param from - Level from value
         \param to - Level to value
    */
    void Update(Level from, Level to);

    // Implementation of Filter
    bool FilterRecord(Record& record) override;

private:
    std::atomic<Level> _from;
    std::atomic<Level> _to;
};

} // namespace CppLogging

#endif // CPPLOGGING_FILTERS_LEVEL_FILTER_H
