/*!
    \file text_layout.cpp
    \brief Text layout implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/layouts/text_layout.h"

#include "system/environment.h"
#include "time/timezone.h"
#include "utility/countof.h"

#include <vector>

namespace CppLogging {

//! @cond INTERNALS

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

        explicit Placeholder(PlaceholderType t) : type(t) {}
        Placeholder(PlaceholderType t, const std::string& v) : type(t), value(v) {}
    };

public:
    Impl(const std::string& pattern) : _pattern(pattern)
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

    ~Impl() = default;

    const std::string& pattern() const noexcept
    {
        return _pattern;
    }

    void LayoutRecord(Record& record)
    {
        thread_local bool cache_initizlied = false;
        thread_local bool cache_time_required = false;
        thread_local bool cache_utc_required = false;
        thread_local bool cache_local_required = false;
        thread_local bool cache_timezone_required = false;
        thread_local bool cache_millisecond_required = false;
        thread_local bool cache_microsecond_required = false;
        thread_local bool cache_nanosecond_required = false;
        thread_local uint64_t cache_seconds = 0;
        thread_local int cache_millisecond = 0;
        thread_local int cache_microsecond = 0;
        thread_local int cache_nanosecond = 0;
        thread_local char cache_utc_datetime_str[] = "1970-01-01T01:01:01.000Z";
        thread_local char cache_utc_date_str[] = "1970-01-01";
        thread_local char cache_utc_time_str[] = "01:01:01.000Z";
        thread_local char cache_utc_year_str[] = "1970";
        thread_local char cache_utc_month_str[] = "01";
        thread_local char cache_utc_day_str[] = "01";
        thread_local char cache_utc_hour_str[] = "00";
        thread_local char cache_utc_minute_str[] = "00";
        thread_local char cache_utc_second_str[] = "00";
        thread_local char cache_utc_timezone_str[] = "Z";
        thread_local char cache_local_datetime_str[] = "1970-01-01T01:01:01.000+00:00";
        thread_local char cache_local_date_str[] = "1970-01-01";
        thread_local char cache_local_time_str[] = "01:01:01.000+00:00";
        thread_local char cache_local_year_str[] = "1970";
        thread_local char cache_local_month_str[] = "01";
        thread_local char cache_local_day_str[] = "01";
        thread_local char cache_local_hour_str[] = "00";
        thread_local char cache_local_minute_str[] = "00";
        thread_local char cache_local_second_str[] = "00";
        thread_local char cache_local_timezone_str[] = "+00:00";
        thread_local char cache_millisecond_str[] = "000";
        thread_local char cache_microsecond_str[] = "000";
        thread_local char cache_nanosecond_str[] = "000";
        thread_local bool cache_thread_required = false;
        thread_local uint64_t cache_thread = 0;
        thread_local char cache_thread_str[] = "0x00000000";
        thread_local bool cache_level_required = false;
        thread_local Level cache_level = Level::FATAL;
        thread_local char cache_level_str[] = "FATAL";
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
                    ConvertTimezone(cache_local_timezone_str, local.total().minutes(), 6);
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
            *buffer++ = ':';
            std::memcpy(buffer, cache_utc_minute_str, CppCommon::countof(cache_utc_minute_str) - 1);
            buffer += CppCommon::countof(cache_utc_minute_str) - 1;
            *buffer++ = ':';
            std::memcpy(buffer, cache_utc_second_str, CppCommon::countof(cache_utc_second_str) - 1);
            buffer += CppCommon::countof(cache_utc_second_str) - 1;
            *buffer++ = '.';
            std::memcpy(buffer, cache_millisecond_str, CppCommon::countof(cache_millisecond_str) - 1);
            buffer += CppCommon::countof(cache_millisecond_str) - 1;
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
            *buffer++ = ':';
            std::memcpy(buffer, cache_local_minute_str, CppCommon::countof(cache_local_minute_str) - 1);
            buffer += CppCommon::countof(cache_local_minute_str) - 1;
            *buffer++ = ':';
            std::memcpy(buffer, cache_local_second_str, CppCommon::countof(cache_local_second_str) - 1);
            buffer += CppCommon::countof(cache_local_second_str) - 1;
            *buffer++ = '.';
            std::memcpy(buffer, cache_millisecond_str, CppCommon::countof(cache_millisecond_str) - 1);
            buffer += CppCommon::countof(cache_millisecond_str) - 1;
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

        // Update thread cache
        if (cache_thread_required || !cache_initizlied)
        {
            if (record.thread != cache_thread)
            {
                cache_thread = record.thread;
                ConvertThread(cache_thread_str, cache_thread, CppCommon::countof(cache_thread_str) - 1);
            }
        }

        // Update level cache
        if (cache_level_required || !cache_initizlied)
        {
            if (record.level != cache_level)
            {
                cache_level = record.level;
                ConvertLevel(cache_level_str, cache_level, CppCommon::countof(cache_level_str) - 1);
            }
        }

        cache_initizlied = true;

        // Clear raw buffer of the logging record
        record.raw.clear();

        // Restore format message
        if (record.IsFormatStored())
            record.message = record.RestoreFormat();

        // Iterate through all placeholders
        for (const auto& placeholder : _placeholders)
        {
            switch (placeholder.type)
            {
                case PlaceholderType::String:
                {
                    // Output pattern string
                    record.raw.insert(record.raw.end(), placeholder.value.begin(), placeholder.value.end());
                    break;
                }
                case PlaceholderType::UtcDateTime:
                {
                    // Output UTC date & time string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_datetime_str), std::end(cache_utc_datetime_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::UtcDate:
                {
                    // Output UTC date string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_date_str), std::end(cache_utc_date_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTime:
                {
                    // Output UTC time string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_time_str), std::end(cache_utc_time_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::UtcYear:
                {
                    // Output UTC year string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_year_str), std::end(cache_utc_year_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMonth:
                {
                    // Output UTC month string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_month_str), std::end(cache_utc_month_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcDay:
                {
                    // Output UTC day string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_day_str), std::end(cache_utc_day_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcHour:
                {
                    // Output UTC hour string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_hour_str), std::end(cache_utc_hour_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMinute:
                {
                    // Output UTC minute string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_minute_str), std::end(cache_utc_minute_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcSecond:
                {
                    // Output UTC second string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_second_str), std::end(cache_utc_second_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTimezone:
                {
                    // Output UTC time zone string
                    record.raw.insert(record.raw.end(), std::begin(cache_utc_timezone_str), std::end(cache_utc_timezone_str) - 1);
                    break;
                }
                case PlaceholderType::LocalDateTime:
                {
                    // Output local date & time string
                    record.raw.insert(record.raw.end(), std::begin(cache_local_datetime_str), std::end(cache_local_datetime_str) - 1);
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
                    record.raw.insert(record.raw.end(), std::begin(cache_local_date_str), std::end(cache_local_date_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTime:
                {
                    // Output local time string
                    record.raw.insert(record.raw.end(), std::begin(cache_local_time_str), std::end(cache_local_time_str) - 1);
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
                    record.raw.insert(record.raw.end(), std::begin(cache_local_year_str), std::end(cache_local_year_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMonth:
                {
                    // Output local month string
                    record.raw.insert(record.raw.end(), std::begin(cache_local_month_str), std::end(cache_local_month_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalDay:
                {
                    // Output local day string
                    record.raw.insert(record.raw.end(), std::begin(cache_local_day_str), std::end(cache_local_day_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalHour:
                {
                    // Output local hour string
                    record.raw.insert(record.raw.end(), std::begin(cache_local_hour_str), std::end(cache_local_hour_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMinute:
                {
                    // Output local minute string
                    record.raw.insert(record.raw.end(), std::begin(cache_local_minute_str), std::end(cache_local_minute_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalSecond:
                {
                    // Output local second string
                    record.raw.insert(record.raw.end(), std::begin(cache_local_second_str), std::end(cache_local_second_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTimezone:
                {
                    // Output local time zone string
                    record.raw.insert(record.raw.end(), std::begin(cache_local_timezone_str), std::end(cache_local_timezone_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_timezone_required = true;
                    break;
                }
                case PlaceholderType::Millisecond:
                {
                    // Output millisecond string
                    record.raw.insert(record.raw.end(), std::begin(cache_millisecond_str), std::end(cache_millisecond_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::Microsecond:
                {
                    // Output microsecond string
                    record.raw.insert(record.raw.end(), std::begin(cache_microsecond_str), std::end(cache_microsecond_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_microsecond_required = true;
                    break;
                }
                case PlaceholderType::Nanosecond:
                {
                    // Output nanosecond string
                    record.raw.insert(record.raw.end(), std::begin(cache_nanosecond_str), std::end(cache_nanosecond_str) - 1);
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_nanosecond_required = true;
                    break;
                }
                case PlaceholderType::Thread:
                {
                    // Output cached thread Id string
                    record.raw.insert(record.raw.end(), std::begin(cache_thread_str), std::end(cache_thread_str) - 1);
                    // Set the corresponding cache required flag
                    cache_thread_required = true;
                    break;
                }
                case PlaceholderType::Level:
                {
                    // Output cached level string
                    record.raw.insert(record.raw.end(), std::begin(cache_level_str), std::end(cache_level_str) - 1);
                    // Set the corresponding cache required flag
                    cache_level_required = true;
                    break;
                }
                case PlaceholderType::Logger:
                {
                    // Output logger string
                    record.raw.insert(record.raw.end(), record.logger.begin(), record.logger.end());
                    break;
                }
                case PlaceholderType::Message:
                {
                    // Output message string
                    record.raw.insert(record.raw.end(), record.message.begin(), record.message.end());
                    break;
                }
            }
        }
    }

private:
    std::string _pattern;
    std::vector<Placeholder> _placeholders;

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
        else if (placeholder == "Millisecond")
            _placeholders.emplace_back(PlaceholderType::Millisecond);
        else if (placeholder == "Microsecond")
            _placeholders.emplace_back(PlaceholderType::Microsecond);
        else if (placeholder == "Nanosecond")
            _placeholders.emplace_back(PlaceholderType::Nanosecond);
        else if (placeholder == "Thread")
            _placeholders.emplace_back(PlaceholderType::Thread);
        else if (placeholder == "Level")
            _placeholders.emplace_back(PlaceholderType::Level);
        else if (placeholder == "Logger")
            _placeholders.emplace_back(PlaceholderType::Logger);
        else if (placeholder == "Message")
            _placeholders.emplace_back(PlaceholderType::Message);
        else if (placeholder == "EndLine")
            AppendPattern(CppCommon::Environment::EndLine());
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

    static void ConvertThread(char* output, uint64_t thread, size_t size)
    {
        const char* digits = "0123456789ABCDEF";

        // Prepare the output string
        std::memset(output, '0', size);

        // Calculate the output index
        size_t index = size - 1;

        // Output digits
        do
        {
            output[index--] = digits[thread & 0x0F];
        } while (((thread >>= 4) != 0) && (index != 0));

        // Output hex prefix
        output[0] = '0';
        output[1] = 'x';
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

    static void ConvertLevel(char* output, Level level, size_t size)
    {
        // Prepare the output string
        std::memset(output, ' ', size);

        switch (level)
        {
            case Level::NONE:
                std::memcpy(output, "NONE", CppCommon::countof("NONE") - 1);
                break;
            case Level::FATAL:
                std::memcpy(output, "FATAL", CppCommon::countof("FATAL") - 1);
                break;
            case Level::ERROR:
                std::memcpy(output, "ERROR", CppCommon::countof("ERROR") - 1);
                break;
            case Level::WARN:
                std::memcpy(output, "WARN", CppCommon::countof("WARN") - 1);
                break;
            case Level::INFO:
                std::memcpy(output, "INFO", CppCommon::countof("INFO") - 1);
                break;
            case Level::DEBUG:
                std::memcpy(output, "DEBUG", CppCommon::countof("DEBUG") - 1);
                break;
            case Level::ALL:
                std::memcpy(output, "ALL", CppCommon::countof("ALL") - 1);
                break;
            default:
                std::memcpy(output, "<\?\?\?>", CppCommon::countof("<\?\?\?>") - 1);
                break;
        }
    }
};

//! @endcond

TextLayout::TextLayout(const std::string& layout) : _pimpl(std::make_unique<Impl>(layout))
{
}

TextLayout::~TextLayout()
{
}

const std::string& TextLayout::pattern() const noexcept
{
    return _pimpl->pattern();
}

void TextLayout::LayoutRecord(Record& record)
{
    _pimpl->LayoutRecord(record);
}

} // namespace CppLogging
