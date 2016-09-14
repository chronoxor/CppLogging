/*!
    \file rolling_file_appender.cpp
    \brief Rolling file appender definition
    \author Ivan Shynkarenka
    \date 12.09.2016
    \copyright MIT License
*/

#include "logging/appenders/rolling_file_appender.h"

#include "errors/exceptions.h"
#include "time/timezone.h"

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

const std::string RollingFileAppender::Impl::ARCHIVE_EXTENSION = "zip";

class TimePolicyImpl : public RollingFileAppender::Impl
{
    enum class PlaceholderType
    {
        String,
        UtcDateTime,
        UtcDate,
        UtcTime,
        UtcYear,
        UtcMonth,
        UtcDay,
        UtcHour,
        UtcMinute,
        UtcSecond,
        UtcTimezone,
        LocalDateTime,
        LocalDate,
        LocalTime,
        LocalYear,
        LocalMonth,
        LocalDay,
        LocalHour,
        LocalMinute,
        LocalSecond,
        LocalTimezone
    };

    struct Placeholder
    {
        PlaceholderType type;
        std::string value;

        Placeholder(PlaceholderType type) : type(type) {}
        Placeholder(PlaceholderType type, const std::string& value) : type(type), value(value) {}
    };

public:
    TimePolicyImpl(const CppCommon::Path& path, RollingFileAppender::TimeRollingPolicy policy, const std::string& pattern, bool archive, bool truncate, bool auto_flush)
        : RollingFileAppender::Impl(path, archive, truncate, auto_flush),
          _policy(policy), _pattern(pattern)
    {
        std::string placeholder;
        std::string subpattern;

        // Tokenize layout pattern
        bool read_placeholder = false;
        for (char ch : pattern)
        {
            // Start reading placeholder or pattern
            if (ch == '{')
            {
                if (read_placeholder)
                    AppendPattern(placeholder);
                else
                    AppendPattern(subpattern);
                placeholder.clear();
                subpattern.clear();
                read_placeholder = true;
            }
            // Stop reading placeholder or pattern
            else if (ch == '}')
            {
                if (read_placeholder)
                {
                    AppendPlaceholder(placeholder);
                    read_placeholder = false;
                }
                else
                    subpattern += ch;
            }
            // Continue reading placeholder or pattern
            else
            {
                if (read_placeholder)
                    placeholder += ch;
                else
                    subpattern += ch;
            }
        }

        // Addend last value of placeholder or pattern
        if (read_placeholder)
            AppendPattern(placeholder);
        else
            AppendPattern(subpattern);

        // Addend end of string character
        AppendPattern(std::string(1, '\0'));
    }

    virtual ~TimePolicyImpl()
    {
    }

private:
    RollingFileAppender::TimeRollingPolicy _policy;
    std::string _pattern;
    std::vector<Placeholder> _placeholders;

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
                CppCommon::File backup = PrepareFilePath(_backups);
                if (backup.IsFileExists())
                    CppCommon::File::Remove(backup);
                backup += "." + ARCHIVE_EXTENSION;
                if (backup.IsFileExists())
                    CppCommon::File::Remove(backup);

                // 1.4. Roll backup files
                for (size_t i = _backups - 1; i > 0; --i)
                {
                    CppCommon::File src = PrepareFilePath(i);
                    CppCommon::File dst = PrepareFilePath(i + 1);
                    if (src.IsFileExists())
                        CppCommon::File::Rename(src, dst);
                    src += "." + ARCHIVE_EXTENSION;
                    dst += "." + ARCHIVE_EXTENSION;
                    if (src.IsFileExists())
                        CppCommon::File::Rename(src, dst);
                }

                // 1.5. Backup the current file
                backup = PrepareFilePath(1);
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
            _file = PrepareFilePath();
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

    CppCommon::Path PrepareFilePath(const CppCommon::Timestamp& timestamp)
    {
        thread_local bool cache_initizlied = false;
        thread_local bool cache_time_required = false;
        thread_local bool cache_utc_required = false;
        thread_local bool cache_local_required = false;
        thread_local bool cache_timezone_required = false;
        thread_local uint64_t cache_seconds = 0;
        thread_local std::string cache_utc_datetime_str = "1970-01-01T01:01:01.000Z";
        thread_local std::string cache_utc_date_str = "1970-01-01";
        thread_local std::string cache_utc_time_str = "01:01:01.000Z";
        thread_local std::string cache_utc_year_str = "1970";
        thread_local std::string cache_utc_month_str = "01";
        thread_local std::string cache_utc_day_str = "01";
        thread_local std::string cache_utc_hour_str = "00";
        thread_local std::string cache_utc_minute_str = "00";
        thread_local std::string cache_utc_second_str = "00";
        thread_local std::string cache_utc_timezone_str = "Z";
        thread_local std::string cache_local_datetime_str = "1970-01-01T01:01:01.000+00:00";
        thread_local std::string cache_local_date_str = "1970-01-01";
        thread_local std::string cache_local_time_str = "01:01:01.000+00:00";
        thread_local std::string cache_local_year_str = "1970";
        thread_local std::string cache_local_month_str = "01";
        thread_local std::string cache_local_day_str = "01";
        thread_local std::string cache_local_hour_str = "00";
        thread_local std::string cache_local_minute_str = "00";
        thread_local std::string cache_local_second_str = "00";
        thread_local std::string cache_local_timezone_str = "+00:00";
        bool cache_update_datetime = false;

        // Update time cache
        if (cache_time_required || !cache_initizlied)
        {
            uint64_t seconds = timestamp.seconds();

            if (seconds != cache_seconds)
            {
                cache_seconds = seconds;

                // Update time zone cache values
                if (cache_timezone_required || !cache_initizlied)
                {
                    CppCommon::Timezone local;
                    ConvertTimezone(cache_local_timezone_str, local.total().minutes());
                    cache_update_datetime = true;
                }

                // Update UTC time cache values
                if (cache_utc_required || !cache_initizlied)
                {
                    CppCommon::UtcTime utc(timestamp);
                    ConvertNumber(cache_utc_year_str, utc.year(), 4);
                    ConvertNumber(cache_utc_month_str, utc.month(), 2);
                    ConvertNumber(cache_utc_day_str, utc.day(), 2);
                    ConvertNumber(cache_utc_hour_str, utc.hour(), 2);
                    ConvertNumber(cache_utc_minute_str, utc.minute(), 2);
                    ConvertNumber(cache_utc_second_str, utc.second(), 2);
                    cache_update_datetime = true;
                }

                // Update local time cache values
                if (cache_local_required || !cache_initizlied)
                {
                    CppCommon::LocalTime local(timestamp);
                    ConvertNumber(cache_local_year_str, local.year(), 4);
                    ConvertNumber(cache_local_month_str, local.month(), 2);
                    ConvertNumber(cache_local_day_str, local.day(), 2);
                    ConvertNumber(cache_local_hour_str, local.hour(), 2);
                    ConvertNumber(cache_local_minute_str, local.minute(), 2);
                    ConvertNumber(cache_local_second_str, local.second(), 2);
                    cache_update_datetime = true;
                }
            }
        }

        // Update date & time cache
        if (cache_update_datetime)
        {
            cache_utc_date_str = cache_utc_year_str;
            cache_utc_date_str += '-';
            cache_utc_date_str += cache_utc_month_str;
            cache_utc_date_str += '-';
            cache_utc_date_str += cache_utc_day_str;

            cache_utc_time_str = cache_utc_hour_str;
            cache_utc_time_str += ':';
            cache_utc_time_str += cache_utc_minute_str;
            cache_utc_time_str += ':';
            cache_utc_time_str += cache_utc_second_str;
            cache_utc_time_str += cache_utc_timezone_str;

            cache_utc_datetime_str = cache_utc_date_str;
            cache_utc_datetime_str += 'T';
            cache_utc_datetime_str += cache_utc_time_str;

            cache_local_date_str = cache_local_year_str;
            cache_local_date_str += '-';
            cache_local_date_str += cache_local_month_str;
            cache_local_date_str += '-';
            cache_local_date_str += cache_local_day_str;

            cache_local_time_str = cache_local_hour_str;
            cache_local_time_str += ':';
            cache_local_time_str += cache_local_minute_str;
            cache_local_time_str += ':';
            cache_local_time_str += cache_local_second_str;
            cache_local_time_str += cache_local_timezone_str;

            cache_local_datetime_str = cache_local_date_str;
            cache_local_datetime_str += 'T';
            cache_local_datetime_str += cache_local_time_str;

            cache_update_datetime = false;
        }

        cache_initizlied = true;

        std::string filename;

        // Iterate through all placeholders
        for (auto& placeholder : _placeholders)
        {
            switch (placeholder.type)
            {
                case PlaceholderType::String:
                {
                    // Output pattern string
                    filename.insert(filename.end(), placeholder.value.begin(), placeholder.value.end());
                    break;
                }
                case PlaceholderType::UtcDateTime:
                {
                    // Output UTC date & time string
                    filename.insert(filename.end(), cache_utc_datetime_str.begin(), cache_utc_datetime_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcDate:
                {
                    // Output UTC date string
                    filename.insert(filename.end(), cache_utc_date_str.begin(), cache_utc_date_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTime:
                {
                    // Output UTC time string
                    filename.insert(filename.end(), cache_utc_time_str.begin(), cache_utc_time_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcYear:
                {
                    // Output UTC year string
                    filename.insert(filename.end(), cache_utc_year_str.begin(), cache_utc_year_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMonth:
                {
                    // Output UTC month string
                    filename.insert(filename.end(), cache_utc_month_str.begin(), cache_utc_month_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcDay:
                {
                    // Output UTC day string
                    filename.insert(filename.end(), cache_utc_day_str.begin(), cache_utc_day_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcHour:
                {
                    // Output UTC hour string
                    filename.insert(filename.end(), cache_utc_hour_str.begin(), cache_utc_hour_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMinute:
                {
                    // Output UTC minute string
                    filename.insert(filename.end(), cache_utc_minute_str.begin(), cache_utc_minute_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcSecond:
                {
                    // Output UTC second string
                    filename.insert(filename.end(), cache_utc_second_str.begin(), cache_utc_second_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTimezone:
                {
                    // Output UTC time zone string
                    filename.insert(filename.end(), cache_utc_timezone_str.begin(), cache_utc_timezone_str.end());
                    break;
                }
                case PlaceholderType::LocalDateTime:
                {
                    // Output local date & time string
                    filename.insert(filename.end(), cache_local_datetime_str.begin(), cache_local_datetime_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    cache_timezone_required = true;
                    break;
                }
                case PlaceholderType::LocalDate:
                {
                    // Output local date string
                    filename.insert(filename.end(), cache_local_date_str.begin(), cache_local_date_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTime:
                {
                    // Output local time string
                    filename.insert(filename.end(), cache_local_time_str.begin(), cache_local_time_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    cache_timezone_required = true;
                    break;
                }
                case PlaceholderType::LocalYear:
                {
                    // Output local year string
                    filename.insert(filename.end(), cache_local_year_str.begin(), cache_local_year_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMonth:
                {
                    // Output local month string
                    filename.insert(filename.end(), cache_local_month_str.begin(), cache_local_month_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalDay:
                {
                    // Output local day string
                    filename.insert(filename.end(), cache_local_day_str.begin(), cache_local_day_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalHour:
                {
                    // Output local hour string
                    filename.insert(filename.end(), cache_local_hour_str.begin(), cache_local_hour_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMinute:
                {
                    // Output local minute string
                    filename.insert(filename.end(), cache_local_minute_str.begin(), cache_local_minute_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalSecond:
                {
                    // Output local second string
                    filename.insert(filename.end(), cache_local_second_str.begin(), cache_local_second_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTimezone:
                {
                    // Output local time zone string
                    filename.insert(filename.end(), cache_local_timezone_str.begin(), cache_local_timezone_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_timezone_required = true;
                    break;
                }
            }
        }

        return CppCommon::Path(_path / filename);
    }

    void AppendPattern(const std::string& pattern)
    {
        //  Skip empty pattern
        if (pattern.empty())
            return;

        // Insert or append pattern into placeholders collection
        if (_placeholders.empty() || (_placeholders[_placeholders.size() - 1].type != PlaceholderType::String))
            _placeholders.push_back(Placeholder(PlaceholderType::String, pattern));
        else
            _placeholders[_placeholders.size() - 1].value += pattern;
    }

    void AppendPlaceholder(const std::string& placeholder)
    {
        //  Skip empty placeholder
        if (placeholder.empty())
            return;

        if (placeholder == "UtcDateTime")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcDateTime));
        else if (placeholder == "UtcDate")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcDate));
        else if (placeholder == "UtcTime")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcTime));
        else if (placeholder == "UtcYear")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcYear));
        else if (placeholder == "UtcMonth")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcMonth));
        else if (placeholder == "UtcDay")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcDay));
        else if (placeholder == "UtcHour")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcHour));
        else if (placeholder == "UtcMinute")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcMinute));
        else if (placeholder == "UtcSecond")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcSecond));
        else if (placeholder == "UtcTimezone")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcTimezone));
        else if (placeholder == "LocalDateTime")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalDateTime));
        else if (placeholder == "LocalDate")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalDate));
        else if (placeholder == "LocalTime")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalTime));
        else if (placeholder == "LocalYear")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalYear));
        else if (placeholder == "LocalMonth")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalMonth));
        else if (placeholder == "LocalDay")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalDay));
        else if (placeholder == "LocalHour")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalHour));
        else if (placeholder == "LocalMinute")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalMinute));
        else if (placeholder == "LocalSecond")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalSecond));
        else if (placeholder == "LocalTimezone")
            _placeholders.push_back(Placeholder(PlaceholderType::LocalTimezone));
        else
            AppendPattern("{" + placeholder + "}");
    }

    static void ConvertNumber(std::string& output, int number, int digits)
    {
        // Prepare the output string
        output.clear();
        output.resize(digits, '0');

        // Calculate the output index
        size_t index = output.size() - 1;

        // Output digits
        while ((number >= 10) && (index != 0))
        {
            int a = number / 10;
            int b = number % 10;
            output[index--] = '0' + (char)b;
            number = a;
        }

        // Output the last digit
        output[index] = '0' + (char)number;
    }

    static void ConvertTimezone(std::string& output, int64_t offset)
    {
        // Prepare the output string
        output.clear();
        output.resize(6, '0');

        // Calculate the output index
        size_t index = output.size() - 1;

        // Output offset minutes
        int64_t minutes = offset % 60;
        if (minutes < 9)
        {
            output[index--] = '0' + (char)minutes;
            --index;
        }
        else
        {
            output[index--] = '0' + (char)(minutes % 10);
            minutes /= 10;
            output[index--] = '0' + (char)minutes;
        }

        // Output ':' separator
        output[index--] = ':';

        // Output offset hours
        int64_t hours = offset / 60;
        if (hours < 9)
        {
            output[index] = '0' + (char)hours;
        }
        else
        {
            output[index--] = '0' + (char)(hours % 10);
            hours /= 10;
            output[index] = '0' + (char)hours;
        }

        // Output minus prefix
        output[0] = (offset < 0) ? '-' : '+';
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
                CppCommon::File backup = PrepareFilePath(_backups);
                if (backup.IsFileExists())
                    CppCommon::File::Remove(backup);
                backup += "." + ARCHIVE_EXTENSION;
                if (backup.IsFileExists())
                    CppCommon::File::Remove(backup);

                // 1.4. Roll backup files
                for (size_t i = _backups - 1; i > 0; --i)
                {
                    CppCommon::File src = PrepareFilePath(i);
                    CppCommon::File dst = PrepareFilePath(i + 1);
                    if (src.IsFileExists())
                        CppCommon::File::Rename(src, dst);
                    src += "." + ARCHIVE_EXTENSION;
                    dst += "." + ARCHIVE_EXTENSION;
                    if (src.IsFileExists())
                        CppCommon::File::Rename(src, dst);
                }

                // 1.5. Backup the current file
                backup = PrepareFilePath(1);
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
            _file = PrepareFilePath();
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

    CppCommon::Path PrepareFilePath()
    {
        return CppCommon::Path(_path / (_filename + "." + _extension));
    }

    CppCommon::Path PrepareFilePath(size_t backup)
    {
        return CppCommon::Path(_path / (_filename + "." + std::to_string(backup) + "." + _extension));
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
