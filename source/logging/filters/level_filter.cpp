/*!
    \file level_filter.cpp
    \brief Level filter implementation
    \author Ivan Shynkarenka
    \date 27.07.2016
    \copyright MIT License
*/

#include "logging/filters/level_filter.h"

namespace CppLogging {

void LevelFilter::Update(Level level, bool positive)
{
    _positive = positive;
    _from = Level::NONE;
    _to = level;
}

void LevelFilter::Update(Level from, Level to, bool positive)
{
    _positive = positive;
    if (from <= to)
    {
        _from = from;
        _to = to;
    }
    else
    {
        _from = to;
        _to = from;
    }
}

bool LevelFilter::FilterRecord(Record& record)
{
    if (_positive)
        return ((record.level >= _from) && (record.level <= _to));
    else
        return ((record.level < _from) || (record.level > _to));
}

} // namespace CppLogging
