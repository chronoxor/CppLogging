/*!
    \file rolling_file_appender.cpp
    \brief Rolling file appender definition
    \author Ivan Shynkarenka
    \date 12.09.2016
    \copyright MIT License
*/

#include "logging/appenders/rolling_file_appender.h"

#include "errors/exceptions.h"

#include <cassert>

namespace CppLogging {

RollingFileAppender::RollingFileAppender(const CppCommon::Path& path, const std::string& pattern, RollingPolicy policy, bool archive, bool truncate, bool auto_flush)
    : FileAppender(path, truncate, auto_flush), _path(path), _policy(policy), _pattern(pattern), _size(0), _backups(0), _archive(archive)
{
    assert(((policy != RollingPolicy::NONE) && (policy != RollingPolicy::SIZE)) && "Policy should be a valid time rolling policy!");
    if ((policy == RollingPolicy::NONE) || (policy == RollingPolicy::SIZE))
        throwex CppCommon::ArgumentException("Policy should be a valid time rolling policy!");
}

RollingFileAppender::RollingFileAppender(const CppCommon::Path& path, const std::string& filename, const std::string& extension, size_t size, size_t backups, bool archive, bool truncate, bool auto_flush)
    : FileAppender(path, truncate, auto_flush), _path(path), _policy(RollingPolicy::SIZE), _pattern(filename + ".{Backup}." + extension), _size(size), _backups(backups), _archive(archive)
{
    assert((size > 0) && "Size limit should be greater than zero!");
    if (size <= 0)
        throwex CppCommon::ArgumentException("Size limit should be greater than zero!");

    assert((backups > 0) && "Backups count should be greater than zero!");
    if (backups <= 0)
        throwex CppCommon::ArgumentException("Backups count should be greater than zero!");
}

bool RollingFileAppender::PrepareFile()
{
    return false;
}

} // namespace CppLogging
