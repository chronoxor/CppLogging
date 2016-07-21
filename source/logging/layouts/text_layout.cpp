/*!
    \file text_layout.cpp
    \brief Logging text layout implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/layouts/text_layout.h"

namespace CppLogging {

class TextLayout::Impl
{
    enum class PlaceholderType
    {
        String,
        DateTime,
        Date,
        Time,
        Year,
        Month,
        Day,
        Hour,
        Minute,
        Second,
        Milli,
        Micro,
        Nano,
        TimeZone,
        UtcDateTime,
        UtcDate,
        UtcTime,
        UtcYear,
        UtcMonth,
        UtcDay,
        UtcHour,
        UtcMinute,
        UtcSecond,
        UtcMilli,
        UtcMicro,
        UtcNano,
        UtcTimeZone,
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
    }

    ~Impl()
    {
    }

    std::pair<void*, size_t> LayoutRecord(const Record& record)
    {
        static bool cache_initizlied = false;
        static bool cache_thread_required = false;
        static uint64_t cache_thread = 0;
        static std::string cache_thread_str = ConvertThread(cache_thread);
        static bool cache_level_required = false;
        static Level cache_level = Level::NONE;
        static std::string cache_level_str = ConvertLevel(cache_level);

        // Update thread cache
        if (cache_thread_required || !cache_initizlied)
        {
            if (record.thread != cache_thread)
            {
                cache_thread = record.thread;
                cache_thread_str = ConvertThread(cache_thread);
            }
        }
        // Update level cache
        if (cache_level_required || !cache_initizlied)
        {
            if (record.level != cache_level)
            {
                cache_level = record.level;
                cache_level_str = ConvertLevel(cache_level);
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
                    // Output a pattern string
                    size_t size = _buffer.size();
                    _buffer.resize(size + placeholder.value.size());
                    std::memcpy(_buffer.data() + size, placeholder.value.c_str(), placeholder.value.size());
                    break;
                }
                case PlaceholderType::Thread:
                {
                    // Output a cached thread Id string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_thread_str.size());
                    std::memcpy(_buffer.data() + size, cache_thread_str.c_str(), cache_thread_str.size());
                    // Set the corresponding cache required flag
                    cache_thread_required = true;
                    break;
                }
                case PlaceholderType::Level:
                {
                    // Output a cached level string
                    size_t size = _buffer.size();
                    _buffer.resize(size + cache_level_str.size());
                    std::memcpy(_buffer.data() + size, cache_level_str.c_str(), cache_level_str.size());
                    // Set the corresponding cache required flag
                    cache_level_required = true;
                    break;
                }
                case PlaceholderType::Logger:
                {
                    // Output a logger string
                    size_t size = _buffer.size();
                    _buffer.resize(size + record.logger.second);
                    std::memcpy(_buffer.data() + size, record.logger.first, record.logger.second);
                    // Set the corresponding cache required flag
                    cache_level_required = true;
                    break;
                }
                case PlaceholderType::Message:
                {
                    // Output a message string
                    size_t size = _buffer.size();
                    _buffer.resize(size + record.message.second);
                    std::memcpy(_buffer.data() + size, record.message.first, record.message.second);
                    // Set the corresponding cache required flag
                    cache_level_required = true;
                    break;
                }
            }
        }

        // Return the serialized buffer
        return std::make_pair(_buffer.data(), _buffer.size());
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

        if (placeholder == "DateTime")
            _placeholders.push_back(Placeholder(PlaceholderType::DateTime));
        else if (placeholder == "Date")
            _placeholders.push_back(Placeholder(PlaceholderType::Date));
        else if (placeholder == "Time")
            _placeholders.push_back(Placeholder(PlaceholderType::Time));
        else if (placeholder == "Year")
            _placeholders.push_back(Placeholder(PlaceholderType::Year));
        else if (placeholder == "Month")
            _placeholders.push_back(Placeholder(PlaceholderType::Month));
        else if (placeholder == "Day")
            _placeholders.push_back(Placeholder(PlaceholderType::Day));
        else if (placeholder == "Hour")
            _placeholders.push_back(Placeholder(PlaceholderType::Hour));
        else if (placeholder == "Minute")
            _placeholders.push_back(Placeholder(PlaceholderType::Minute));
        else if (placeholder == "Second")
            _placeholders.push_back(Placeholder(PlaceholderType::Second));
        else if (placeholder == "Milli")
            _placeholders.push_back(Placeholder(PlaceholderType::Milli));
        else if (placeholder == "Nano")
            _placeholders.push_back(Placeholder(PlaceholderType::Nano));
        else if (placeholder == "TimeZone")
            _placeholders.push_back(Placeholder(PlaceholderType::TimeZone));
        else if (placeholder == "UtcDateTime")
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
        else if (placeholder == "UtcMilli")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcMilli));
        else if (placeholder == "UtcNano")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcNano));
        else if (placeholder == "UtcTimeZone")
            _placeholders.push_back(Placeholder(PlaceholderType::UtcTimeZone));
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

    std::string ConvertThread(uint64_t thread)
    {
        const char* digits = "0123456789ABCDEF";

        // Calculate digits count
        uint64_t number = thread;
        unsigned count = 0;
        do
        {
            ++count;
        } while ((number >>= 4) != 0);

        // Prepare output string
        std::string result(10, '0');

        // Calculate output index
        size_t index = result.size() - 1;

        // Output digits
        number = thread;
        do
        {
            result[index--] = digits[number & 0x0F];
        } while ((number >>= 4) != 0);

        // Output hex prefix
        result[0] = '0';
        result[1] = 'x';

        return result;
    }

    std::string ConvertLevel(Level level)
    {
        switch (level)
        {
            case Level::NONE:
                return "NONE ";
            case Level::FATAL:
                return "FATAL";
            case Level::ERROR:
                return "ERROR";
            case Level::WARN:
                return "WARN ";
            case Level::INFO:
                return "INFO ";
            case Level::DEBUG:
                return "DEBUG";
            case Level::ALL:
                return "ALL  ";
            default:
                return "<???>";
        }
    }
};

TextLayout::TextLayout(const std::string& layout) : _pimpl(new Impl(layout))
{
}

TextLayout::~TextLayout()
{
}

std::pair<void*, size_t> TextLayout::LayoutRecord(const Record& record)
{
    return _pimpl->LayoutRecord(record);
}

} // namespace CppLogging
