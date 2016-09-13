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

const std::string RollingFileAppender::ARCHIVE_EXTENSION = ".zip";

RollingFileAppender::RollingFileAppender(const CppCommon::Path& path, const std::string& pattern, RollingPolicy policy, bool archive, bool truncate, bool auto_flush)
    : FileAppender(path, truncate, auto_flush), _path(path), _policy(policy), _pattern(pattern), _filename(), _extension(), _size(0), _backups(0), _written(0), _archive(archive)
{
    assert((policy != RollingPolicy::SIZE) && "Policy should be a valid time rolling policy!");
    if (policy == RollingPolicy::SIZE)
        throwex CppCommon::ArgumentException("Policy should be a valid time rolling policy!");
}

RollingFileAppender::RollingFileAppender(const CppCommon::Path& path, const std::string& filename, const std::string& extension, size_t size, size_t backups, bool archive, bool truncate, bool auto_flush)
    : FileAppender(path, truncate, auto_flush), _path(path), _policy(RollingPolicy::SIZE), _pattern(), _filename(filename), _extension(extension), _size(size), _backups(backups), _written(0), _archive(archive)
{
    assert((size > 0) && "Size limit should be greater than zero!");
    if (size <= 0)
        throwex CppCommon::ArgumentException("Size limit should be greater than zero!");

    assert((backups > 0) && "Backups count should be greater than zero!");
    if (backups <= 0)
        throwex CppCommon::ArgumentException("Backups count should be greater than zero!");
}

void RollingFileAppender::AppendRecord(Record& record)
{
    FileAppender::AppendRecord(record);

    // Update written bytes counter
    _written += record.raw.size() - 1;
}

bool RollingFileAppender::PrepareFile(size_t size)
{
    try
    {
        if (_policy == RollingPolicy::SIZE)
        {
            // 1. Check if the file is already opened for writing
            if (_file.IsFileWriteOpened())
            {
                // 1.1. Check file size limit
                if ((_written + size) < _size)
                    return true;

                // 1.2. Flush & close the file
                _file.Flush();
                _file.Close();

                // 1.3. Delete the last backup and archive if exists
                CppCommon::File backup = _path / (_filename + "." + std::to_string(_backups) + _extension);
                if (backup.IsFileExists())
                    CppCommon::File::Remove(backup);
                backup += ARCHIVE_EXTENSION;
                if (backup.IsFileExists())
                    CppCommon::File::Remove(backup);

                // 1.4. Roll backup files
                for (size_t i = _backups - 1; i > 0; --i)
                {
                    CppCommon::File src = _path / (_filename + "." + std::to_string(i) + _extension);
                    CppCommon::File dst = _path / (_filename + "." + std::to_string(i + 1) + _extension);
                    if (src.IsFileExists())
                        CppCommon::File::Rename(src, dst);
                    src += ARCHIVE_EXTENSION;
                    dst += ARCHIVE_EXTENSION;
                    if (src.IsFileExists())
                        CppCommon::File::Rename(src, dst);
                }

                // 1.5. Backup the current file
                backup = _path / (_filename + "." + std::to_string(1) + _extension);
                CppCommon::File::Rename(_file, backup);

                // 1.6. Archive the current backup
                if (_archive)
                    ArchiveFile(backup);
            }

            // 2. Check retry timestamp if 100ms elapsed after the last attempt
            if ((CppCommon::Timestamp::nano() - _retry).milliseconds() < 100)
                return false;

            // 3. If the file is opened for reading close it
            if (_file.IsFileReadOpened())
                _file.Close();

            // 4. Open the file for writing
            _file = _path / (_filename + _extension);
            _file.OpenOrCreate(false, true, _truncate);
            _written = 0;

            // 5. Reset the the retry timestamp
            _retry = CppCommon::Timestamp::nano();
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (CppCommon::FileSystemException&)
    {
        // In case of any IO error reset the retry timestamp and return false!
        _retry = CppCommon::Timestamp::nano();
        return false;
    }
}

void RollingFileAppender::ArchiveFile(const CppCommon::File& file)
{

}

} // namespace CppLogging
