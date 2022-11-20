/*!
    \file rolling_file_appender.inl
    \brief Rolling file appender inline implementation
    \author Ivan Shynkarenka
    \date 12.09.2016
    \copyright MIT License
*/

namespace CppLogging {

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, TimeRollingPolicy policy)
{
    switch (policy)
    {
        case TimeRollingPolicy::YEAR:
            stream << "Year";
            break;
        case TimeRollingPolicy::MONTH:
            stream << "Month";
            break;
        case TimeRollingPolicy::DAY:
            stream << "Day";
            break;
        case TimeRollingPolicy::HOUR:
            stream << "Hour";
            break;
        case TimeRollingPolicy::MINUTE:
            stream << "Minute";
            break;
        case TimeRollingPolicy::SECOND:
            stream << "Second";
            break;
        default:
            stream << "<unknown>";
            break;
    }
    return stream;
}

} // namespace CppLogging
