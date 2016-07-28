/*!
    \file level_filter.cpp
    \brief Level filter implementation
    \author Ivan Shynkarenka
    \date 27.07.2016
    \copyright MIT License
*/

#include "logging/filters/level_filter.h"

namespace CppLogging {

bool LevelFilter::FilterRecord(Record& record)
{
    return ((record.level >= _from) && (record.level <= _to));
}

} // namespace CppLogging
