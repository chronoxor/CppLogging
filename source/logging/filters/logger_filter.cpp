/*!
    \file logger_filter.cpp
    \brief Logging logger filter implementation
    \author Ivan Shynkarenka
    \date 27.07.2016
    \copyright MIT License
*/

#include "logging/filters/logger_filter.h"

namespace CppLogging {

bool LoggerFilter::FilterRecord(Record& record)
{
    return std::regex_match(record.logger.first, record.logger.first + record.logger.second, _pattern);
}

} // namespace CppLogging
