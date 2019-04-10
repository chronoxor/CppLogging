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
         \param positive - Positive filtration (default is true)
    */
    explicit LevelFilter(Level level, bool positive = true) { Update(level, positive); }
    //! Initialize level filter with a given level range
    /*!
         \param from - Level from value
         \param to - Level to value
         \param positive - Positive filtration (default is true)
    */
    explicit LevelFilter(Level from, Level to, bool positive = true) { Update(from, to, positive); }
    LevelFilter(const LevelFilter&) = delete;
    LevelFilter(LevelFilter&&) = delete;
    virtual ~LevelFilter() = default;

    LevelFilter& operator=(const LevelFilter&) = delete;
    LevelFilter& operator=(LevelFilter&&) = delete;

    //! Get the positive filtration flag
    bool positive() const noexcept { return _positive; }

    //! Get Level from value
    Level from() const noexcept { return _from; }
    //! Get Level to value
    Level to() const noexcept { return _to; }

    //! Update level filter with a given level value
    /*!
         \param level - Level value
         \param positive - Positive filtration (default is true)
    */
    void Update(Level level, bool positive = true);
    //! Update level filter with a given level range
    /*!
         \param from - Level from value
         \param to - Level to value
         \param positive - Positive filtration (default is true)
    */
    void Update(Level from, Level to, bool positive = true);

    // Implementation of Filter
    bool FilterRecord(Record& record) override;

private:
    std::atomic<bool> _positive;
    std::atomic<Level> _from;
    std::atomic<Level> _to;
};

} // namespace CppLogging

#endif // CPPLOGGING_FILTERS_LEVEL_FILTER_H
