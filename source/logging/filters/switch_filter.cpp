/*!
    \file switch_filter.cpp
    \brief Switch filter implementation
    \author Ivan Shynkarenka
    \date 27.07.2016
    \copyright MIT License
*/

#include "logging/filters/switch_filter.h"

namespace CppLogging {

void SwitchFilter::Update(bool enabled)
{
    _enabled = enabled;
}

bool SwitchFilter::FilterRecord(Record& record)
{
    return _enabled;
}

} // namespace CppLogging
