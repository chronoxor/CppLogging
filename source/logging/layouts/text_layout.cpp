/*!
    \file text_layout.cpp
    \brief Text layout implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/layouts/text_layout.h"

#include "time/timezone.h"

#include <cstring>
#include <vector>

namespace CppLogging {

class TextLayout::Impl
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
        LocalTimezone,
        Millisecond,
        Microsecond,
        Nanosecond,
        Thread,
        Level,
        Logger,
        Message
    };

    struct Placeholder
    {
        PlaceholderType type;
        std::string value;

        Placeholder(PlaceholderType type) : type(type) {}
        Placeholder(PlaceholderType type, const std::string& value) : type(type), value(value) {}
    };

public:
    Impl(const std::string& pattern) : _buffer(1024)
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

    ~Impl()
    {
    }

    std::pair<void*, size_t> LayoutRecord(Record& record)
    {
        static bool cache_initizlied = false;
        static bool cache_time_required = false;
        static bool cache_utc_required = false;
        static bool cache_local_required = false;
        static bool cache_timezone_required = false;
        static bool cache_millisecond_required = false;
        static bool cache_microsecond_required = false;
        static bool cache_nanosecond_required = false;
        static uint64_t cache_seconds = 0;
        static int cache_millisecond = 0;
        static int cache_microsecond = 0;
        static int cache_nanosecond = 0;
        static std::string cache_utc_datetime_str = "1970-01-01T01:01:01.000Z";
        static std::string cache_utc_date_str = "1970-01-01";
        static std::string cache_utc_time_str = "01:01:01.000Z";
        static std::string cache_utc_year_str = "1970";
        static std::string cache_utc_month_str = "01";
        static std::string cache_utc_day_str = "01";
        static std::string cache_utc_hour_str = "00";
        static std::string cache_utc_minute_str = "00";
        static std::string cache_utc_second_str = "00";
        static std::string cache_local_datetime_str = "1970-01-01T01:01:01.000+00:00";
        static std::string cache_local_date_str = "1970-01-01";
        static std::string cache_local_time_str = "01:01:01.000+00:00";
        static std::string cache_local_year_str = "1970";
        static std::string cache_local_month_str = "01";
        static std::string cache_local_day_str = "01";
        static std::string cache_local_hour_str = "00";
        static std::string cache_local_minute_str = "00";
        static std::string cache_local_second_str = "00";
        static std::string cache_timezone_str = "+00:00";
        static std::string cache_millisecond_str = "000";
        static std::string cache_microsecond_str = "000";
        static std::string cache_nanosecond_str = "000";
        static bool cache_thread_required = false;
        static uint64_t cache_thread = 0;
        static std::string cache_thread_str = "0x00000000";
        static bool cache_level_required = false;
        static Level cache_level = Level::FATAL;
        static std::string cache_level_str = "FATAL";
        bool cache_update_datetime = false;

        // Update time cache
        if (cache_time_required || !cache_initizlied)
        {
            CppCommon::Timestamp timestamp(record.timestamp);
            uint64_t seconds = timestamp.seconds();
            int millisecond = timestamp.milliseconds() % 1000;
            int microsecond = timestamp.microseconds() % 1000;
            int nanosecond = timestamp.nanoseconds() % 1000;

            if (nanosecond != cache_nanosecond)
            {
                cache_nanosecond = nanosecond;

                // Update nanosecond cache values
                if (cache_nanosecond_required || !cache_initizlied)
                    ConvertNumber(cache_nanosecond_str, nanosecond, 3);
            }

            if (microsecond != cache_microsecond)
            {
                cache_microsecond = microsecond;

                // Update microsecond cache values
                if (cache_microsecond_required || !cache_initizlied)
                    ConvertNumber(cache_microsecond_str, microsecond, 3);
            }

            if (millisecond != cache_millisecond)
            {
                cache_millisecond = millisecond;

                // Update millisecond cache values
                if (cache_millisecond_required || !cache_initizlied)
                {
                    ConvertNumber(cache_millisecond_str, millisecond, 3);
                    cache_update_datetime = true;
                }
            }

            if (seconds != cache_seconds)
            {
                cache_seconds = seconds;

                // Update time zone cache values
                if (cache_timezone_required || !cache_initizlied)
                {
                    CppCommon::Timezone local;
                    ConvertTimezone(cache_timezone_str, local.total().minutes());
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
            cache_utc_time_str += '.';
            cache_utc_time_str += cache_millisecond_str;
            cache_utc_time_str += 'Z';

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
            cache_local_time_str += '.';
            cache_local_time_str += cache_millisecond_str;
            cache_local_time_str += cache_timezone_str;

            cache_local_datetime_str = cache_local_date_str;
            cache_local_datetime_str += 'T';
            cache_local_datetime_str += cache_local_time_str;

            cache_update_datetime = false;
        }

        // Update thread cache
        if (cache_thread_required || !cache_initizlied)
        {
            if (record.thread != cache_thread)
            {
                cache_thread = record.thread;
                ConvertThread(cache_thread_str, cache_thread);
            }
        }

        // Update level cache
        if (cache_level_required || !cache_initizlied)
        {
            if (record.level != cache_level)
            {
                cache_level = record.level;
                ConvertLevel(cache_level_str, cache_level);
            }
        }

        cache_initizlied = true;

        // Clear buffer
        _buffer.clear();

        // Iterate through all placeholders
        for (auto& placeholder : _placeholders)
        {
            switch (placeholder.type)
            {
                case PlaceholderType::String:
                {
                    // Output pattern string
                    size_t size = _buffer.size();
                    _buffer.resize(size + placeholder.value.size());
                    std::memcpy(_buffer.data() + size, placeholder.value.c_str(), placeholder.value.size());
                    break;
                }
                case PlaceholderType::UtcDateTime:
                {
                    // Output UTC date & time string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_utc_datetime_str.size());
                    std::memcpy(_buffer.data() + size, cache_utc_datetime_str.c_str(), cache_utc_datetime_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::UtcDate:
                {
                    // Output UTC date string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_utc_date_str.size());
                    std::memcpy(_buffer.data() + size, cache_utc_date_str.c_str(), cache_utc_date_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTime:
                {
                    // Output UTC time string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_utc_time_str.size());
                    std::memcpy(_buffer.data() + size, cache_utc_time_str.c_str(), cache_utc_time_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::UtcYear:
                {
                    // Output UTC year string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_utc_year_str.size());
                    std::memcpy(_buffer.data() + size, cache_utc_year_str.c_str(), cache_utc_year_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMonth:
                {
                    // Output UTC month string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_utc_month_str.size());
                    std::memcpy(_buffer.data() + size, cache_utc_month_str.c_str(), cache_utc_month_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcDay:
                {
                    // Output UTC day string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_utc_day_str.size());
                    std::memcpy(_buffer.data() + size, cache_utc_day_str.c_str(), cache_utc_day_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcHour:
                {
                    // Output UTC hour string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_utc_hour_str.size());
                    std::memcpy(_buffer.data() + size, cache_utc_hour_str.c_str(), cache_utc_hour_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMinute:
                {
                    // Output UTC minute string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_utc_minute_str.size());
                    std::memcpy(_buffer.data() + size, cache_utc_minute_str.c_str(), cache_utc_minute_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcSecond:
                {
                    // Output UTC second string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_utc_second_str.size());
                    std::memcpy(_buffer.data() + size, cache_utc_second_str.c_str(), cache_utc_second_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTimezone:
                {
                    // Output UTC time zone string
                    size_t size = _buffer.size();
                    _buffer.resize(size + 1);
                    std::memcpy(_buffer.data() + size, "Z", 1);
                    break;
                }
                case PlaceholderType::LocalDateTime:
                {
                    // Output local date & time string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_local_datetime_str.size());
                    std::memcpy(_buffer.data() + size, cache_local_datetime_str.c_str(), cache_local_datetime_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    cache_timezone_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::LocalDate:
                {
                    // Output local date string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_local_date_str.size());
                    std::memcpy(_buffer.data() + size, cache_local_date_str.c_str(), cache_local_date_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTime:
                {
                    // Output local time string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_local_time_str.size());
                    std::memcpy(_buffer.data() + size, cache_local_time_str.c_str(), cache_local_time_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    cache_timezone_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::LocalYear:
                {
                    // Output local year string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_local_year_str.size());
                    std::memcpy(_buffer.data() + size, cache_local_year_str.c_str(), cache_local_year_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMonth:
                {
                    // Output local month string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_local_month_str.size());
                    std::memcpy(_buffer.data() + size, cache_local_month_str.c_str(), cache_local_month_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalDay:
                {
                    // Output local day string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_local_day_str.size());
                    std::memcpy(_buffer.data() + size, cache_local_day_str.c_str(), cache_local_day_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalHour:
                {
                    // Output local hour string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_local_hour_str.size());
                    std::memcpy(_buffer.data() + size, cache_local_hour_str.c_str(), cache_local_hour_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMinute:
                {
                    // Output local minute string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_local_minute_str.size());
                    std::memcpy(_buffer.data() + size, cache_local_minute_str.c_str(), cache_local_minute_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalSecond:
                {
                    // Output local second string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_local_second_str.size());
                    std::memcpy(_buffer.data() + size, cache_local_second_str.c_str(), cache_local_second_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTimezone:
                {
                    // Output local time zone string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_timezone_str.size());
                    std::memcpy(_buffer.data() + size, cache_timezone_str.c_str(), cache_timezone_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_timezone_required = true;
                    break;
                }
                case PlaceholderType::Millisecond:
                {
                    // Output millisecond string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_millisecond_str.size());
                    std::memcpy(_buffer.data() + size, cache_millisecond_str.c_str(), cache_millisecond_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::Microsecond:
                {
                    // Output microsecond string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_microsecond_str.size());
                    std::memcpy(_buffer.data() + size, cache_microsecond_str.c_str(), cache_microsecond_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_microsecond_required = true;
                    break;
                }
                case PlaceholderType::Nanosecond:
                {
                    // Output nanosecond string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_nanosecond_str.size());
                    std::memcpy(_buffer.data() + size, cache_nanosecond_str.c_str(), cache_nanosecond_str.size());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_nanosecond_required = true;
                    break;
                }
                case PlaceholderType::Thread:
                {
                    // Output cached thread Id string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_thread_str.size());
                    std::memcpy(_buffer.data() + size, cache_thread_str.c_str(), cache_thread_str.size());
                    // Set the corresponding cache required flag
                    cache_thread_required = true;
                    break;
                }
                case PlaceholderType::Level:
                {
                    // Output cached level string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_level_str.size());
                    std::memcpy(_buffer.data() + size, cache_level_str.c_str(), cache_level_str.size());
                    // Set the corresponding cache required flag
                    cache_level_required = true;
                    break;
                }
                case PlaceholderType::Logger:
                {
                    // Output logger string
                    size_t size = _buffer.size();
                    _buffer.resize(size + record.logger.second);
                    std::memcpy(_buffer.data() + size, record.logger.first, record.logger.second);
                    break;
                }
                case PlaceholderType::Message:
                {
                    // Output message string
                    size_t size = _buffer.size();
                    _buffer.resize(size + record.message.second);
                    std::memcpy(_buffer.data() + size, record.message.first, record.message.second);
                    break;
                }
            }
        }

        // Update raw field of the logging record and return
        record.raw = std::make_pair(_buffer.data(), _buffer.size());
        return record.raw;
    }

private:
    std::vector<Placeholder> _placeholders;
    std::vector<uint8_t> _buffer;

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
        else if (placeholder == "Millisecond")
            _placeholders.push_back(Placeholder(PlaceholderType::Millisecond));
        else if (placeholder == "Microsecond")
            _placeholders.push_back(Placeholder(PlaceholderType::Microsecond));
        else if (placeholder == "Nanosecond")
            _placeholders.push_back(Placeholder(PlaceholderType::Nanosecond));
        else if (placeholder == "Thread")
            _placeholders.push_back(Placeholder(PlaceholderType::Thread));
        else if (placeholder == "Level")
            _placeholders.push_back(Placeholder(PlaceholderType::Level));
        else if (placeholder == "Logger")
            _placeholders.push_back(Placeholder(PlaceholderType::Logger));
        else if (placeholder == "Message")
            _placeholders.push_back(Placeholder(PlaceholderType::Message));
        else if (placeholder == "EndLine")
#if defined(_WIN32) || defined(_WIN64)
            AppendPattern("\r\n");
#elif defined(unix) || defined(__unix) || defined(__unix__)
            AppendPattern("\n");
#endif
        else
            AppendPattern("{" + placeholder + "}");
    }

    void ConvertNumber(std::string& output, int number, int digits)
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

    void ConvertThread(std::string& output, uint64_t thread)
    {
        const char* digits = "0123456789ABCDEF";

        // Prepare the output string
        output.clear();
        output.resize(10, '0');

        // Calculate the output index
        size_t index = output.size() - 1;

        // Output digits
        do
        {
            output[index--] = digits[thread & 0x0F];
        } while (((thread >>= 4) != 0) && (index != 0));

        // Output hex prefix
        output[0] = '0';
        output[1] = 'x';
    }

    void ConvertTimezone(std::string& output, int64_t offset)
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
            output[index--] = '0' + (char)hours;
            --index;
        }
        else
        {
            output[index--] = '0' + (char)(hours % 10);
            hours /= 10;
            output[index--] = '0' + (char)hours;
        }

        // Output minus prefix
        output[0] = (offset < 0) ? '-' : '+';
    }

    void ConvertLevel(std::string& output, Level level)
    {
        switch (level)
        {
            case Level::NONE:
                output = "NONE ";
                break;
            case Level::FATAL:
                output = "FATAL";
                break;
            case Level::ERROR:
                output = "ERROR";
                break;
            case Level::WARN:
                output = "WARN ";
                break;
            case Level::INFO:
                output = "INFO ";
                break;
            case Level::DEBUG:
                output = "DEBUG";
                break;
            case Level::ALL:
                output = "ALL  ";
                break;
            default:
                output = "<\?\?\?>";
                break;
        }
    }
};

TextLayout::TextLayout(const std::string& layout) : _pimpl(new Impl(layout))
{
}

TextLayout::~TextLayout()
{
}

std::pair<void*, size_t> TextLayout::LayoutRecord(Record& record)
{
    return _pimpl->LayoutRecord(record);
}

} // namespace CppLogging
