/*!
    \file rolling_file_appender.cpp
    \brief Rolling file appender definition
    \author Ivan Shynkarenka
    \date 12.09.2016
    \copyright MIT License
*/

#include "logging/appenders/rolling_file_appender.h"

#include "errors/fatal.h"
#include "string/format.h"
#include "threads/thread.h"
#include "threads/wait_queue.h"
#include "time/timezone.h"
#include "utility/countof.h"
#include "utility/resource.h"

#include "minizip/zip.h"
#if defined(_WIN32) || defined(_WIN64)
#include "minizip/iowin32.h"
#endif

#include <atomic>
#include <cassert>

namespace CppLogging {

//! @cond INTERNALS

class RollingFileAppender::Impl
{
public:
    static const std::string ARCHIVE_EXTENSION;

    Impl(RollingFileAppender& appender, const CppCommon::Path& path, bool archive, bool truncate, bool auto_flush, bool auto_start)
        : _appender(appender), _path(path), _archive(archive), _truncate(truncate), _auto_flush(auto_flush)
    {
        // Start the rolling file appender
        if (auto_start)
            Start();
    }

    virtual ~Impl()
    {
        // Stop the rolling file appender
        if (IsStarted())
            Stop();
    }

    virtual bool IsStarted() const noexcept { return _started; }

    virtual bool Start()
    {
        if (IsStarted())
            return false;

        if (_archive)
            ArchivationStart();

        _started = true;
        return true;
    }

    virtual bool Stop()
    {
        if (!IsStarted())
            return false;

        CloseFile();

        if (_archive)
            ArchivationStop();

        _started = false;
        return true;
    }

    virtual void AppendRecord(Record& record) = 0;
    virtual void Flush() = 0;

protected:
    RollingFileAppender& _appender;
    CppCommon::Path _path;
    bool _archive;
    bool _truncate;
    bool _auto_flush;

    std::atomic<bool> _started{false};
    CppCommon::Timestamp _retry{0};
    CppCommon::File _file;
    size_t _written{0};

    bool CloseFile()
    {
        try
        {
            // Check if the file is already opened for writing
            if (_file.IsFileWriteOpened())
            {
                // Flush & close the file
                _file.Flush();
                _file.Close();

                // Archive the file
                if (_archive)
                    ArchiveQueue(_file);
            }
            return true;
        }
        catch (const CppCommon::FileSystemException&) { return false; }
    }

    std::thread _archive_thread;
    CppCommon::WaitQueue<CppCommon::Path> _archive_queue;

    virtual void ArchiveQueue(const CppCommon::Path& path)
    {
        _archive_queue.Enqueue(path);
    }

    virtual void ArchiveFile(const CppCommon::Path& path, const CppCommon::Path& filename)
    {
        CppCommon::File file(path);

        // Create a new zip archive
        zipFile zf;
#if defined(_WIN32) || defined(_WIN64)
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64W(&ffunc);
        zf = zipOpen2_64((file + ".zip").wstring().c_str(), APPEND_STATUS_CREATE, nullptr, &ffunc);
#else
        zf = zipOpen64((file + ".zip").string().c_str(), APPEND_STATUS_CREATE);
#endif
        if (zf == nullptr)
            throwex CppCommon::FileSystemException("Cannot create a new zip archive!").Attach(file);

        // Smart resource cleaner pattern
        auto zip = CppCommon::resource(zf, [](zipFile handle) { zipClose(handle, nullptr); });

        // Open a new file in zip archive
        int result = zipOpenNewFileInZip64(zf, filename.empty() ? file.filename().string().c_str() : filename.string().c_str(), nullptr, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 1);
        if (result != ZIP_OK)
            throwex CppCommon::FileSystemException("Cannot open a new file in zip archive!").Attach(file);

        // Smart resource cleaner pattern
        auto zip_file = CppCommon::resource(zf, [](zipFile handle) { zipCloseFileInZip(handle); });

        CppCommon::File source(file);
        uint8_t buffer[16384];
        size_t size;

        // Open the source file for reading
        source.Open(true, false);

        // Write data into the zip file
        do
        {
            size = source.Read(buffer, CppCommon::countof(buffer));
            if (size > 0)
            {
                result = zipWriteInFileInZip(zf, buffer, (unsigned)size);
                if (result != ZIP_OK)
                    throwex CppCommon::FileSystemException("Cannot write into the zip file!").Attach(file);
            }
        } while (size > 0);

        // Close the source file
        source.Close();

        // Close the file in zip archive
        result = zipCloseFileInZip(zf);
        if (result != ZIP_OK)
            throwex CppCommon::FileSystemException("Cannot close a file in zip archive!").Attach(file);
        zip_file.release();

        // Close zip archive
        result = zipClose(zf, nullptr);
        if (result != ZIP_OK)
            throwex CppCommon::FileSystemException("Cannot close a zip archive!").Attach(file);
        zip.release();

        // Remove the source file
        CppCommon::File::Remove(source);
    }

    void ArchivationStart()
    {
        // Start archivation thread
        _archive_thread = CppCommon::Thread::Start([this]() { ArchivationThread(); });
    }

    void ArchivationStop()
    {
        // Stop archivation thread
        _archive_queue.Close();
        _archive_thread.join();
    }

    void ArchivationThread()
    {
        // Call initialize archivation thread handler
        _appender.onArchiveThreadInitialize();

        try
        {
            CppCommon::Path path;
            while (_archive_queue.Dequeue(path))
                ArchiveFile(path, "");
        }
        catch (const std::exception& ex)
        {
            fatality(ex);
        }
        catch (...)
        {
            fatality("Archivation thread terminated!");
        }

        // Call cleanup archivation thread handler
        _appender.onArchiveThreadCleanup();
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

        explicit Placeholder(PlaceholderType t) : type(t) {}
        Placeholder(PlaceholderType t, const std::string& v) : type(t), value(v) {}
    };

public:
    TimePolicyImpl(RollingFileAppender& appender, const CppCommon::Path& path, TimeRollingPolicy policy, const std::string& pattern, bool archive, bool truncate, bool auto_flush, bool auto_start)
        : RollingFileAppender::Impl(appender, path, archive, truncate, auto_flush, auto_start),
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

        // Calculate rolling delay
        switch (policy)
        {
            case TimeRollingPolicy::SECOND:
                _rolldelay = 1000000000ull;
                break;
            case TimeRollingPolicy::MINUTE:
                _rolldelay = 60 * 1000000000ull;
                break;
            case TimeRollingPolicy::HOUR:
                _rolldelay = 60 * 60 * 1000000000ull;
                break;
            default:
                _rolldelay = 24 * 60 * 60 * 1000000000ull;
                break;
        }
    }

    virtual ~TimePolicyImpl() = default;

    TimeRollingPolicy policy() const
    {
        return _policy;
    }

    void AppendRecord(Record& record) override
    {
        // Skip logging records without layout
        if (record.raw.empty())
            return;

        size_t size = record.raw.size() - 1;

        if (PrepareFile(record.timestamp))
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
            catch (const CppCommon::FileSystemException&)
            {
                // Try to close the opened file in case of any IO error
                try
                {
                    _file.Close();
                }
                catch (const CppCommon::FileSystemException&) {}
            }
        }
    }

    void Flush() override
    {
        if (PrepareFile(CppCommon::Timestamp::utc()))
        {
            // Try to flush the opened file
            try
            {
                _file.Flush();
            }
            catch (const CppCommon::FileSystemException&)
            {
                // Try to close the opened file in case of any IO error
                try
                {
                    _file.Close();
                }
                catch (const CppCommon::FileSystemException&) {}
            }
        }
    }

private:
    TimeRollingPolicy _policy;
    std::string _pattern;
    std::vector<Placeholder> _placeholders;
    CppCommon::Timestamp _rollstamp{0};
    CppCommon::Timespan _rolldelay{0};
    bool _first{true};

    bool PrepareFile(uint64_t timestamp)
    {
        try
        {
            // 1. Check if the file is already opened for writing
            if (_file.IsFileWriteOpened())
            {
                // 1.1. Check the rolling timestamp
                if (timestamp < (_rollstamp + _rolldelay))
                    return true;

                // 1.2. Flush & close the file
                _file.Flush();
                _file.Close();

                // 1.3. Archive the file
                if (_archive)
                    ArchiveQueue(_file);
            }

            // 2. Check retry timestamp if 100ms elapsed after the last attempt
            if ((CppCommon::Timestamp::utc() - _retry).milliseconds() < 100)
                return false;

            uint64_t rollstamp = timestamp;

            // 3. Truncate rolling timestamp according to the time rolling policy
            switch (_policy)
            {
                case TimeRollingPolicy::SECOND:
                    rollstamp = (rollstamp / 1000000000ull) * 1000000000ull;
                    break;
                case TimeRollingPolicy::MINUTE:
                    rollstamp = (rollstamp / (60 * 1000000000ull)) * (60 * 1000000000ull);
                    break;
                case TimeRollingPolicy::HOUR:
                    rollstamp = (rollstamp / (60 * 60 * 1000000000ull)) * (60 * 60 * 1000000000ull);
                    break;
                default:
                    rollstamp = (rollstamp / (24 * 60 * 60 * 1000000000ull)) * (24 * 60 * 60 * 1000000000ull);
                    break;
            }

            // 4. Reset the flag for the first rolling file
            if (_first)
                _first = false;
            else
                timestamp = rollstamp;

            // 5. If the file is opened for reading close it
            if (_file.IsFileReadOpened())
                _file.Close();

            // 6. Prepare the actual rolling file path
            _file = PrepareFilePath(CppCommon::Timestamp(timestamp));

            // 7. Create the parent directory tree
            CppCommon::Directory::CreateTree(_file.parent());

            // 8. Open or create the rolling file
            _file.OpenOrCreate(false, true, _truncate);

            // 9. Reset the written bytes counter
            _written = 0;

            // 10. Reset the retry timestamp
            _retry = 0;

            // 11. Reset the rolling timestamp
            _rollstamp = rollstamp;

            return true;
        }
        catch (const CppCommon::FileSystemException&)
        {
            // In case of any IO error reset the retry timestamp and return false!
            _retry = CppCommon::Timestamp::utc();
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
        thread_local char cache_utc_datetime_str[] = "1970-01-01T010101Z";
        thread_local char cache_utc_date_str[] = "1970-01-01";
        thread_local char cache_utc_time_str[] = "010101Z";
        thread_local char cache_utc_year_str[] = "1970";
        thread_local char cache_utc_month_str[] = "01";
        thread_local char cache_utc_day_str[] = "01";
        thread_local char cache_utc_hour_str[] = "00";
        thread_local char cache_utc_minute_str[] = "00";
        thread_local char cache_utc_second_str[] = "00";
        thread_local char cache_utc_timezone_str[] = "Z";
        thread_local char cache_local_datetime_str[] = "1970-01-01T010101+0000";
        thread_local char cache_local_date_str[] = "1970-01-01";
        thread_local char cache_local_time_str[] = "010101+0000";
        thread_local char cache_local_year_str[] = "1970";
        thread_local char cache_local_month_str[] = "01";
        thread_local char cache_local_day_str[] = "01";
        thread_local char cache_local_hour_str[] = "00";
        thread_local char cache_local_minute_str[] = "00";
        thread_local char cache_local_second_str[] = "00";
        thread_local char cache_local_timezone_str[] = "+0000";
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
                    ConvertTimezone(cache_local_timezone_str, local.total().minutes(), 5);
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
            char* buffer = cache_utc_date_str;
            std::memcpy(buffer, cache_utc_year_str, CppCommon::countof(cache_utc_year_str) - 1);
            buffer += CppCommon::countof(cache_utc_year_str) - 1;
            *buffer++ = '-';
            std::memcpy(buffer, cache_utc_month_str, CppCommon::countof(cache_utc_month_str) - 1);
            buffer += CppCommon::countof(cache_utc_month_str) - 1;
            *buffer++ = '-';
            std::memcpy(buffer, cache_utc_day_str, CppCommon::countof(cache_utc_day_str) - 1);
            buffer += CppCommon::countof(cache_utc_day_str) - 1;

            buffer = cache_utc_time_str;
            std::memcpy(buffer, cache_utc_hour_str, CppCommon::countof(cache_utc_hour_str) - 1);
            buffer += CppCommon::countof(cache_utc_hour_str) - 1;
            std::memcpy(buffer, cache_utc_minute_str, CppCommon::countof(cache_utc_minute_str) - 1);
            buffer += CppCommon::countof(cache_utc_minute_str) - 1;
            std::memcpy(buffer, cache_utc_second_str, CppCommon::countof(cache_utc_second_str) - 1);
            buffer += CppCommon::countof(cache_utc_second_str) - 1;
            std::memcpy(buffer, cache_utc_timezone_str, CppCommon::countof(cache_utc_timezone_str) - 1);
            buffer += CppCommon::countof(cache_utc_timezone_str) - 1;

            buffer = cache_utc_datetime_str;
            std::memcpy(buffer, cache_utc_date_str, CppCommon::countof(cache_utc_date_str) - 1);
            buffer += CppCommon::countof(cache_utc_date_str) - 1;
            *buffer++ = 'T';
            std::memcpy(buffer, cache_utc_time_str, CppCommon::countof(cache_utc_time_str) - 1);
            buffer += CppCommon::countof(cache_utc_time_str) - 1;

            buffer = cache_local_date_str;
            std::memcpy(buffer, cache_local_year_str, CppCommon::countof(cache_local_year_str) - 1);
            buffer += CppCommon::countof(cache_local_year_str) - 1;
            *buffer++ = '-';
            std::memcpy(buffer, cache_local_month_str, CppCommon::countof(cache_local_month_str) - 1);
            buffer += CppCommon::countof(cache_local_month_str) - 1;
            *buffer++ = '-';
            std::memcpy(buffer, cache_local_day_str, CppCommon::countof(cache_local_day_str) - 1);
            buffer += CppCommon::countof(cache_local_day_str) - 1;

            buffer = cache_local_time_str;
            std::memcpy(buffer, cache_local_hour_str, CppCommon::countof(cache_local_hour_str) - 1);
            buffer += CppCommon::countof(cache_local_hour_str) - 1;
            std::memcpy(buffer, cache_local_minute_str, CppCommon::countof(cache_local_minute_str) - 1);
            buffer += CppCommon::countof(cache_local_minute_str) - 1;
            std::memcpy(buffer, cache_local_second_str, CppCommon::countof(cache_local_second_str) - 1);
            buffer += CppCommon::countof(cache_local_second_str) - 1;
            std::memcpy(buffer, cache_local_timezone_str, CppCommon::countof(cache_local_timezone_str) - 1);
            buffer += CppCommon::countof(cache_local_timezone_str) - 1;

            buffer = cache_local_datetime_str;
            std::memcpy(buffer, cache_local_date_str, CppCommon::countof(cache_local_date_str) - 1);
            buffer += CppCommon::countof(cache_local_date_str) - 1;
            *buffer++ = 'T';
            std::memcpy(buffer, cache_local_time_str, CppCommon::countof(cache_local_time_str) - 1);
            buffer += CppCommon::countof(cache_local_time_str) - 1;

            cache_update_datetime = false;
        }

        cache_initizlied = true;

        std::string filename;

        // Iterate through all placeholders
        for (const auto& placeholder : _placeholders)
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
                    filename.insert(filename.end(), std::begin(cache_utc_datetime_str), std::end(cache_utc_datetime_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcDate:
                {
                    // Output UTC date string
                    filename.insert(filename.end(), std::begin(cache_utc_date_str), std::end(cache_utc_date_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTime:
                {
                    // Output UTC time string
                    filename.insert(filename.end(), std::begin(cache_utc_time_str), std::end(cache_utc_time_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcYear:
                {
                    // Output UTC year string
                    filename.insert(filename.end(), std::begin(cache_utc_year_str), std::end(cache_utc_year_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMonth:
                {
                    // Output UTC month string
                    filename.insert(filename.end(), std::begin(cache_utc_month_str), std::end(cache_utc_month_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcDay:
                {
                    // Output UTC day string
                    filename.insert(filename.end(), std::begin(cache_utc_day_str), std::end(cache_utc_day_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcHour:
                {
                    // Output UTC hour string
                    filename.insert(filename.end(), std::begin(cache_utc_hour_str), std::end(cache_utc_hour_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMinute:
                {
                    // Output UTC minute string
                    filename.insert(filename.end(), std::begin(cache_utc_minute_str), std::end(cache_utc_minute_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcSecond:
                {
                    // Output UTC second string
                    filename.insert(filename.end(), std::begin(cache_utc_second_str), std::end(cache_utc_second_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTimezone:
                {
                    // Output UTC time zone string
                    filename.insert(filename.end(), std::begin(cache_utc_timezone_str), std::end(cache_utc_timezone_str) - 1);
                    break;
                }
                case PlaceholderType::LocalDateTime:
                {
                    // Output local date & time string
                    filename.insert(filename.end(), std::begin(cache_local_datetime_str), std::end(cache_local_datetime_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    cache_timezone_required = true;
                    break;
                }
                case PlaceholderType::LocalDate:
                {
                    // Output local date string
                    filename.insert(filename.end(), std::begin(cache_local_date_str), std::end(cache_local_date_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTime:
                {
                    // Output local time string
                    filename.insert(filename.end(), std::begin(cache_local_time_str), std::end(cache_local_time_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    cache_timezone_required = true;
                    break;
                }
                case PlaceholderType::LocalYear:
                {
                    // Output local year string
                    filename.insert(filename.end(), std::begin(cache_local_year_str), std::end(cache_local_year_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMonth:
                {
                    // Output local month string
                    filename.insert(filename.end(), std::begin(cache_local_month_str), std::end(cache_local_month_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalDay:
                {
                    // Output local day string
                    filename.insert(filename.end(), std::begin(cache_local_day_str), std::end(cache_local_day_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalHour:
                {
                    // Output local hour string
                    filename.insert(filename.end(), std::begin(cache_local_hour_str), std::end(cache_local_hour_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMinute:
                {
                    // Output local minute string
                    filename.insert(filename.end(), std::begin(cache_local_minute_str), std::end(cache_local_minute_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalSecond:
                {
                    // Output local second string
                    filename.insert(filename.end(), std::begin(cache_local_second_str), std::end(cache_local_second_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTimezone:
                {
                    // Output local time zone string
                    filename.insert(filename.end(), std::begin(cache_local_timezone_str), std::end(cache_local_timezone_str) - 1);
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
            _placeholders.emplace_back(PlaceholderType::String, pattern);
        else
            _placeholders[_placeholders.size() - 1].value += pattern;
    }

    void AppendPlaceholder(const std::string& placeholder)
    {
        //  Skip empty placeholder
        if (placeholder.empty())
            return;

        if (placeholder == "UtcDateTime")
            _placeholders.emplace_back(PlaceholderType::UtcDateTime);
        else if (placeholder == "UtcDate")
            _placeholders.emplace_back(PlaceholderType::UtcDate);
        else if (placeholder == "UtcTime")
            _placeholders.emplace_back(PlaceholderType::UtcTime);
        else if (placeholder == "UtcYear")
            _placeholders.emplace_back(PlaceholderType::UtcYear);
        else if (placeholder == "UtcMonth")
            _placeholders.emplace_back(PlaceholderType::UtcMonth);
        else if (placeholder == "UtcDay")
            _placeholders.emplace_back(PlaceholderType::UtcDay);
        else if (placeholder == "UtcHour")
            _placeholders.emplace_back(PlaceholderType::UtcHour);
        else if (placeholder == "UtcMinute")
            _placeholders.emplace_back(PlaceholderType::UtcMinute);
        else if (placeholder == "UtcSecond")
            _placeholders.emplace_back(PlaceholderType::UtcSecond);
        else if (placeholder == "UtcTimezone")
            _placeholders.emplace_back(PlaceholderType::UtcTimezone);
        else if (placeholder == "LocalDateTime")
            _placeholders.emplace_back(PlaceholderType::LocalDateTime);
        else if (placeholder == "LocalDate")
            _placeholders.emplace_back(PlaceholderType::LocalDate);
        else if (placeholder == "LocalTime")
            _placeholders.emplace_back(PlaceholderType::LocalTime);
        else if (placeholder == "LocalYear")
            _placeholders.emplace_back(PlaceholderType::LocalYear);
        else if (placeholder == "LocalMonth")
            _placeholders.emplace_back(PlaceholderType::LocalMonth);
        else if (placeholder == "LocalDay")
            _placeholders.emplace_back(PlaceholderType::LocalDay);
        else if (placeholder == "LocalHour")
            _placeholders.emplace_back(PlaceholderType::LocalHour);
        else if (placeholder == "LocalMinute")
            _placeholders.emplace_back(PlaceholderType::LocalMinute);
        else if (placeholder == "LocalSecond")
            _placeholders.emplace_back(PlaceholderType::LocalSecond);
        else if (placeholder == "LocalTimezone")
            _placeholders.emplace_back(PlaceholderType::LocalTimezone);
        else
            AppendPattern("{" + placeholder + "}");
    }

    static void ConvertNumber(char* output, int number, size_t size)
    {
        // Prepare the output string
        std::memset(output, '0', size);

        // Calculate the output index
        size_t index = size - 1;

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

    static void ConvertTimezone(char* output, int64_t offset, size_t size)
    {
        // Prepare the output string
        std::memset(output, '0', size);

        // Calculate the output index
        size_t index = size - 1;

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
    SizePolicyImpl(RollingFileAppender& appender, const CppCommon::Path& path, const std::string& filename, const std::string& extension, size_t size, size_t backups, bool archive, bool truncate, bool auto_flush, bool auto_start)
        : RollingFileAppender::Impl(appender, path, archive, truncate, auto_flush, auto_start),
          _filename(filename), _extension(extension), _size(size), _backups(backups)
    {
        assert((size > 0) && "Size limit should be greater than zero!");
        if (size <= 0)
            throwex CppCommon::ArgumentException("Size limit should be greater than zero!");

        assert((backups > 0) && "Backups count should be greater than zero!");
        if (backups <= 0)
            throwex CppCommon::ArgumentException("Backups count should be greater than zero!");
    }

    virtual ~SizePolicyImpl() = default;

    void AppendRecord(Record& record) override
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
            catch (const CppCommon::FileSystemException&)
            {
                // Try to close the opened file in case of any IO error
                try
                {
                    _file.Close();
                }
                catch (const CppCommon::FileSystemException&) {}
            }
        }
    }

    void Flush() override
    {
        if (PrepareFile(0))
        {
            // Try to flush the opened file
            try
            {
                _file.Flush();
            }
            catch (const CppCommon::FileSystemException&)
            {
                // Try to close the opened file in case of any IO error
                try
                {
                    _file.Close();
                }
                catch (const CppCommon::FileSystemException&) {}
            }
        }
    }

private:
    std::string _filename;
    std::string _extension;
    size_t _size;
    size_t _backups;

    bool PrepareFile(size_t size)
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

                // 1.3. Archive or roll the current backup
                if (_archive)
                    ArchiveQueue(_file);
                else
                    RollBackup(_file);
            }

            // 2. Check retry timestamp if 100ms elapsed after the last attempt
            if ((CppCommon::Timestamp::utc() - _retry).milliseconds() < 100)
                return false;

            // 3. If the file is opened for reading close it
            if (_file.IsFileReadOpened())
                _file.Close();

            // 4. Prepare the actual rolling file path
            _file = PrepareFilePath();

            // 5. Create the parent directory tree
            CppCommon::Directory::CreateTree(_file.parent());

            // 6. Open or create the rolling file
            _file.OpenOrCreate(false, true, _truncate);

            // 7. Reset the written bytes counter
            _written = 0;

            // 8. Reset the retry timestamp
            _retry = 0;

            return true;
        }
        catch (const CppCommon::FileSystemException&)
        {
            // In case of any IO error reset the retry timestamp and return false!
            _retry = CppCommon::Timestamp::utc();
            return false;
        }
    }

    void ArchiveQueue(const CppCommon::Path& path) override
    {
        // Create unique file name
        CppCommon::File unique = CppCommon::File(path).ReplaceFilename(CppCommon::File::unique());
        CppCommon::File::Rename(path, unique);

        _archive_queue.Enqueue(unique);
    }

    void ArchiveFile(const CppCommon::Path& path, const CppCommon::Path& filename) override
    {
        // Roll backup
        CppCommon::File backup = RollBackup(path);

        // Archive backup
        Impl::ArchiveFile(backup, PrepareFilePath());
    }

    CppCommon::File RollBackup(const CppCommon::Path& path)
    {
        // Delete the last backup and archive if exists
        CppCommon::File backup = PrepareFilePath(_backups);
        if (backup.IsFileExists())
            CppCommon::File::Remove(backup);
        backup += "." + ARCHIVE_EXTENSION;
        if (backup.IsFileExists())
            CppCommon::File::Remove(backup);

        // Roll backup files
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

        // Backup the current file
        backup = PrepareFilePath(1);
        CppCommon::File::Rename(path, backup);
        return backup;
    }

    CppCommon::Path PrepareFilePath()
    {
        return CppCommon::Path(_path / "{}.{}"_format(_filename, _extension));
    }

    CppCommon::Path PrepareFilePath(size_t backup)
    {
        return CppCommon::Path(_path / "{}.{}.{}"_format(_filename, backup, _extension));
    }
};

//! @endcond

RollingFileAppender::RollingFileAppender(const CppCommon::Path& path, TimeRollingPolicy policy, const std::string& pattern, bool archive, bool truncate, bool auto_flush, bool auto_start)
    : _pimpl(std::make_unique<TimePolicyImpl>(*this, path, policy, pattern, archive, truncate, auto_flush, auto_start))
{
}

RollingFileAppender::RollingFileAppender(const CppCommon::Path& path, const std::string& filename, const std::string& extension, size_t size, size_t backups, bool archive, bool truncate, bool auto_flush, bool auto_start)
    : _pimpl(std::make_unique<SizePolicyImpl>(*this, path, filename, extension, size, backups, archive, truncate, auto_flush, auto_start))
{
}

RollingFileAppender::~RollingFileAppender()
{
}

bool RollingFileAppender::IsStarted() const noexcept
{
    return _pimpl->IsStarted();
}

bool RollingFileAppender::Start()
{
    return _pimpl->Start();
}

bool RollingFileAppender::Stop()
{
    return _pimpl->Stop();
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
