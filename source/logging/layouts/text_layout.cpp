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
    Impl(const std::string& layout) : _buffer(1024)
    {
        std::string pattern;
        std::string placeholder;

        bool read_placeholder = false;
        for (size_t i = 0; i < layout.size(); ++i)
        {
            if (layout[i] == '{')
            {
                if (read_placeholder)
                    AppendPattern(placeholder);
                else
                    AppendPattern(pattern);
                pattern.clear();
                placeholder.clear();
                read_placeholder = true;
            }
            else if (layout[i] == '}')
            {
                if (read_placeholder)
                {
                    AppendPlaceholder(placeholder);
                    read_placeholder = false;
                }
                else
                    pattern += layout[i];
            }
            else
            {
                if (read_placeholder)
                    placeholder += layout[i];
                else
                    pattern += layout[i];
            }
        }
        if (read_placeholder)
            AppendPattern(placeholder);
        else
            AppendPattern(pattern);
    }

    ~Impl()
    {
    }

    void AppendPattern(const std::string& pattern)
    {
        if (pattern.empty())
            return;

        if (_pattern.empty() || (_pattern[_pattern.size() - 1].type != PlaceholderType::String))
            _pattern.push_back(Placeholder(PlaceholderType::String, pattern));
        else
            _pattern[_pattern.size() - 1].value += pattern;
    }

    void AppendPlaceholder(const std::string& placeholder)
    {
        if (placeholder.empty())
            return;

        if (placeholder == "DateTime")
            _pattern.push_back(Placeholder(PlaceholderType::DateTime));
        else if (placeholder == "Date")
            _pattern.push_back(Placeholder(PlaceholderType::Date));
        else if (placeholder == "Time")
            _pattern.push_back(Placeholder(PlaceholderType::Time));
        else if (placeholder == "Year")
            _pattern.push_back(Placeholder(PlaceholderType::Year));
        else if (placeholder == "Month")
            _pattern.push_back(Placeholder(PlaceholderType::Month));
        else if (placeholder == "Day")
            _pattern.push_back(Placeholder(PlaceholderType::Day));
        else if (placeholder == "Hour")
            _pattern.push_back(Placeholder(PlaceholderType::Hour));
        else if (placeholder == "Minute")
            _pattern.push_back(Placeholder(PlaceholderType::Minute));
        else if (placeholder == "Second")
            _pattern.push_back(Placeholder(PlaceholderType::Second));
        else if (placeholder == "Milli")
            _pattern.push_back(Placeholder(PlaceholderType::Milli));
        else if (placeholder == "Nano")
            _pattern.push_back(Placeholder(PlaceholderType::Nano));
        else if (placeholder == "TimeZone")
            _pattern.push_back(Placeholder(PlaceholderType::TimeZone));
        else if (placeholder == "UtcDateTime")
            _pattern.push_back(Placeholder(PlaceholderType::UtcDateTime));
        else if (placeholder == "UtcDate")
            _pattern.push_back(Placeholder(PlaceholderType::UtcDate));
        else if (placeholder == "UtcTime")
            _pattern.push_back(Placeholder(PlaceholderType::UtcTime));
        else if (placeholder == "UtcYear")
            _pattern.push_back(Placeholder(PlaceholderType::UtcYear));
        else if (placeholder == "UtcMonth")
            _pattern.push_back(Placeholder(PlaceholderType::UtcMonth));
        else if (placeholder == "UtcDay")
            _pattern.push_back(Placeholder(PlaceholderType::UtcDay));
        else if (placeholder == "UtcHour")
            _pattern.push_back(Placeholder(PlaceholderType::UtcHour));
        else if (placeholder == "UtcMinute")
            _pattern.push_back(Placeholder(PlaceholderType::UtcMinute));
        else if (placeholder == "UtcSecond")
            _pattern.push_back(Placeholder(PlaceholderType::UtcSecond));
        else if (placeholder == "UtcMilli")
            _pattern.push_back(Placeholder(PlaceholderType::UtcMilli));
        else if (placeholder == "UtcNano")
            _pattern.push_back(Placeholder(PlaceholderType::UtcNano));
        else if (placeholder == "UtcTimeZone")
            _pattern.push_back(Placeholder(PlaceholderType::UtcTimeZone));
        else if (placeholder == "Thread")
            _pattern.push_back(Placeholder(PlaceholderType::Thread));
        else if (placeholder == "Level")
            _pattern.push_back(Placeholder(PlaceholderType::Level));
        else if (placeholder == "Logger")
            _pattern.push_back(Placeholder(PlaceholderType::Logger));
        else if (placeholder == "Message")
            _pattern.push_back(Placeholder(PlaceholderType::Message));
        else if (placeholder == "EndLine")
#if defined(_WIN32) || defined(_WIN64)
            AppendPattern("\r\n");
#elif defined(unix) || defined(__unix) || defined(__unix__)
            AppendPattern("\n");
#endif
        else
            AppendPattern("{" + placeholder + "}");
    }

    std::pair<void*, size_t> LayoutRecord(const Record& record)
    {
        // Return the serialized buffer
        return std::make_pair(_buffer.data(), _buffer.size());
    }

private:
    std::vector<Placeholder> _pattern;
    std::vector<uint8_t> _buffer;
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
