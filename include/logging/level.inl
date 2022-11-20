/*!
    \file level.inl
    \brief Logging level inline implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

namespace CppLogging {

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, Level level)
{
    switch (level)
    {
        case Level::NONE:
            stream << "None";
            break;
        case Level::FATAL:
            stream << "Fatal";
            break;
        case Level::ERROR:
            stream << "Error";
            break;
        case Level::WARN:
            stream << "Warn";
            break;
        case Level::INFO:
            stream << "Info";
            break;
        case Level::DEBUG:
            stream << "Debug";
            break;
        case Level::ALL:
            stream << "All";
            break;
        default:
            stream << "<unknown>";
            break;
    }
    return stream;
}

} // namespace CppLogging
