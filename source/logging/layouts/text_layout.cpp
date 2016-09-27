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

        Placeholder(PlaceholderType type) : type(type) {}
        Placeholder(PlaceholderType type, const std::string& value) : type(type), value(value) {}
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

    ~Impl()
    {
    }

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
        thread_local std::string cache_millisecond_str = "000";
        thread_local std::string cache_microsecond_str = "000";
        thread_local std::string cache_nanosecond_str = "000";
        thread_local bool cache_thread_required = false;
        thread_local uint64_t cache_thread = 0;
        thread_local std::string cache_thread_str = "0x00000000";
        thread_local bool cache_level_required = false;
        thread_local Level cache_level = Level::FATAL;
        thread_local std::string cache_level_str = "FATAL";
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
            cache_utc_time_str += '.';
            cache_utc_time_str += cache_millisecond_str;
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
            cache_local_time_str += '.';
            cache_local_time_str += cache_millisecond_str;
            cache_local_time_str += cache_local_timezone_str;

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

        // Clear raw buffer of the logging record
        record.raw.clear();

        // Iterate through all placeholders
        for (auto& placeholder : _placeholders)
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
                    record.raw.insert(record.raw.end(), cache_utc_datetime_str.begin(), cache_utc_datetime_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::UtcDate:
                {
                    // Output UTC date string
                    record.raw.insert(record.raw.end(), cache_utc_date_str.begin(), cache_utc_date_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTime:
                {
                    // Output UTC time string
                    record.raw.insert(record.raw.end(), cache_utc_time_str.begin(), cache_utc_time_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::UtcYear:
                {
                    // Output UTC year string
                    record.raw.insert(record.raw.end(), cache_utc_year_str.begin(), cache_utc_year_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMonth:
                {
                    // Output UTC month string
                    record.raw.insert(record.raw.end(), cache_utc_month_str.begin(), cache_utc_month_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcDay:
                {
                    // Output UTC day string
                    record.raw.insert(record.raw.end(), cache_utc_day_str.begin(), cache_utc_day_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcHour:
                {
                    // Output UTC hour string
                    record.raw.insert(record.raw.end(), cache_utc_hour_str.begin(), cache_utc_hour_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcMinute:
                {
                    // Output UTC minute string
                    record.raw.insert(record.raw.end(), cache_utc_minute_str.begin(), cache_utc_minute_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcSecond:
                {
                    // Output UTC second string
                    record.raw.insert(record.raw.end(), cache_utc_second_str.begin(), cache_utc_second_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_utc_required = true;
                    break;
                }
                case PlaceholderType::UtcTimezone:
                {
                    // Output UTC time zone string
                    record.raw.insert(record.raw.end(), cache_utc_timezone_str.begin(), cache_utc_timezone_str.end());
                    break;
                }
                case PlaceholderType::LocalDateTime:
                {
                    // Output local date & time string
                    record.raw.insert(record.raw.end(), cache_local_datetime_str.begin(), cache_local_datetime_str.end());
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
                    record.raw.insert(record.raw.end(), cache_local_date_str.begin(), cache_local_date_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTime:
                {
                    // Output local time string
                    record.raw.insert(record.raw.end(), cache_local_time_str.begin(), cache_local_time_str.end());
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
                    record.raw.insert(record.raw.end(), cache_local_year_str.begin(), cache_local_year_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMonth:
                {
                    // Output local month string
                    record.raw.insert(record.raw.end(), cache_local_month_str.begin(), cache_local_month_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalDay:
                {
                    // Output local day string
                    record.raw.insert(record.raw.end(), cache_local_day_str.begin(), cache_local_day_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalHour:
                {
                    // Output local hour string
                    record.raw.insert(record.raw.end(), cache_local_hour_str.begin(), cache_local_hour_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalMinute:
                {
                    // Output local minute string
                    record.raw.insert(record.raw.end(), cache_local_minute_str.begin(), cache_local_minute_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalSecond:
                {
                    // Output local second string
                    record.raw.insert(record.raw.end(), cache_local_second_str.begin(), cache_local_second_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_local_required = true;
                    break;
                }
                case PlaceholderType::LocalTimezone:
                {
                    // Output local time zone string
                    record.raw.insert(record.raw.end(), cache_local_timezone_str.begin(), cache_local_timezone_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_timezone_required = true;
                    break;
                }
                case PlaceholderType::Millisecond:
                {
                    // Output millisecond string
                    record.raw.insert(record.raw.end(), cache_millisecond_str.begin(), cache_millisecond_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_millisecond_required = true;
                    break;
                }
                case PlaceholderType::Microsecond:
                {
                    // Output microsecond string
                    record.raw.insert(record.raw.end(), cache_microsecond_str.begin(), cache_microsecond_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_microsecond_required = true;
                    break;
                }
                case PlaceholderType::Nanosecond:
                {
                    // Output nanosecond string
                    record.raw.insert(record.raw.end(), cache_nanosecond_str.begin(), cache_nanosecond_str.end());
                    // Set the corresponding cache required flag
                    cache_time_required = true;
                    cache_nanosecond_required = true;
                    break;
                }
                case PlaceholderType::Thread:
                {
                    // Output cached thread Id string
                    record.raw.insert(record.raw.end(), cache_thread_str.begin(), cache_thread_str.end());
                    // Set the corresponding cache required flag
                    cache_thread_required = true;
                    break;
                }
                case PlaceholderType::Level:
                {
                    // Output cached level string
                    record.raw.insert(record.raw.end(), cache_level_str.begin(), cache_level_str.end());
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
            AppendPattern(CppCommon::Environment::EndLine());
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

    static void ConvertThread(std::string& output, uint64_t thread)
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

    static void ConvertLevel(std::string& output, Level level)
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

//! @endcond

TextLayout::TextLayout(const std::string& layout) : _pimpl(std::make_unique<Impl>(layout))
{
}

TextLayout::TextLayout(TextLayout&& layout) : _pimpl(std::move(layout._pimpl))
{
}

TextLayout::~TextLayout()
{
}

TextLayout& TextLayout::operator=(TextLayout&& layout)
{
    _pimpl = std::move(layout._pimpl);
    return *this;
}

const std::string& TextLayout::pattern() const noexcept
{
    return _pimpl->pattern();
}

void TextLayout::LayoutRecord(Record& record)
{
    // Format message of the logging record
    record.FormatDeserialize();

    _pimpl->LayoutRecord(record);
}

} // namespace CppLogging
