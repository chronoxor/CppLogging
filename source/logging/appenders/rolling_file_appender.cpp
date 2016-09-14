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

class RollingFileAppender::Impl
{
public:
    static const std::string ARCHIVE_EXTENSION;

    Impl(const CppCommon::Path& path, bool archive, bool truncate, bool auto_flush)
        : _path(path), _archive(archive), _truncate(truncate), _auto_flush(auto_flush),
          _retry(0), _file(), _written(0)
    {
    }

    virtual ~Impl()
    {
    }

    void AppendRecord(Record& record)
    {
        // Skip logging records without layout
        if (record.raw.empty())
            return;

        size_t size = record.raw.size() - 1;
        if (PrepareFile(size))
        {
            // Try to write logging record content into the opened file
            try
            {
                _file.Write(record.raw.data(), size);
                _written += size;

                // Perform auto-flush if enabled
                if (_auto_flush)
                    _file.Flush();
            }
            catch (CppCommon::FileSystemException&)
            {
                // Try to close the opened file in case of any IO error
                try
                {
                    _file.Close();
                }
                catch (CppCommon::FileSystemException&) {}
            }
        }
    }

    void Flush()
    {
        if (PrepareFile(0))
        {
            // Try to flush the opened file
            try
            {
                _file.Flush();
            }
            catch (CppCommon::FileSystemException&)
            {
                // Try to close the opened file in case of any IO error
                try
                {
                    _file.Close();
                }
                catch (CppCommon::FileSystemException&) {}
            }
        }
    }

protected:
    CppCommon::Path _path;
    bool _archive;
    bool _truncate;
    bool _auto_flush;

    CppCommon::Timestamp _retry;
    CppCommon::File _file;
    size_t _written;

    virtual bool PrepareFile(size_t size) = 0;

    void ArchiveFile(const CppCommon::File& file)
    {
    }
};

const std::string RollingFileAppender::Impl::ARCHIVE_EXTENSION = ".zip";

class TimePolicyImpl : public RollingFileAppender::Impl
{
public:
    TimePolicyImpl(const CppCommon::Path& path, RollingFileAppender::TimeRollingPolicy policy, const std::string& pattern, bool archive, bool truncate, bool auto_flush)
        : RollingFileAppender::Impl(path, archive, truncate, auto_flush),
          _policy(policy), _pattern(pattern)
    {
    }

    virtual ~TimePolicyImpl()
    {
    }

private:
    RollingFileAppender::TimeRollingPolicy _policy;
    std::string _pattern;

    bool PrepareFile(size_t size) override
    {
        return false;
    }
};

class SizePolicyImpl : public RollingFileAppender::Impl
{
public:
    SizePolicyImpl(const CppCommon::Path& path, const std::string& filename, const std::string& extension, size_t size, size_t backups, bool archive, bool truncate, bool auto_flush)
        : RollingFileAppender::Impl(path, archive, truncate, auto_flush),
          _filename(filename), _extension(extension), _size(size), _backups(backups)
    {
        assert((size > 0) && "Size limit should be greater than zero!");
        if (size <= 0)
            throwex CppCommon::ArgumentException("Size limit should be greater than zero!");

        assert((backups > 0) && "Backups count should be greater than zero!");
        if (backups <= 0)
            throwex CppCommon::ArgumentException("Backups count should be greater than zero!");
    }

    virtual ~SizePolicyImpl()
    {
    }

private:
    std::string _filename;
    std::string _extension;
    size_t _size;
    size_t _backups;

    bool PrepareFile(size_t size) override
    {
        try
        {
            // 1. Check if the file is already opened for writing
            if (_file.IsFileWriteOpened())
            {
                // 1.1. Check file size limit
                if ((_written + size) <= _size)
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

            // 5. Reset the written bytes counter
            _written = 0;

            // 6. Reset the the retry timestamp
            _retry = 0;

            return true;
        }
        catch (CppCommon::FileSystemException&)
        {
            // In case of any IO error reset the retry timestamp and return false!
            _retry = CppCommon::Timestamp::nano();
            return false;
        }
    }
};

RollingFileAppender::RollingFileAppender(const CppCommon::Path& path, TimeRollingPolicy policy, const std::string& pattern, bool archive, bool truncate, bool auto_flush)
    : _pimpl(std::make_unique<TimePolicyImpl>(path, policy, pattern, archive, truncate, auto_flush))
{
}

RollingFileAppender::RollingFileAppender(const CppCommon::Path& path, const std::string& filename, const std::string& extension, size_t size, size_t backups, bool archive, bool truncate, bool auto_flush)
    : _pimpl(std::make_unique<SizePolicyImpl>(path, filename, extension, size, backups, archive, truncate, auto_flush))
{
}

RollingFileAppender::RollingFileAppender(RollingFileAppender&& appender) : _pimpl(std::move(appender._pimpl))
{
}

RollingFileAppender::~RollingFileAppender()
{
}

RollingFileAppender& RollingFileAppender::operator=(RollingFileAppender&& appender)
{
    _pimpl = std::move(appender._pimpl);
    return *this;
}

void RollingFileAppender::AppendRecord(Record& record)
{
    _pimpl->AppendRecord(record);
}

void RollingFileAppender::Flush()
{
    _pimpl->Flush();
}

} // namespace CppLogging
