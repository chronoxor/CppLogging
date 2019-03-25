/*!
    \file record.inl
    \brief Logging record inline implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

namespace CppLogging {

inline Record::Record()
    : timestamp(CppCommon::Timestamp::utc()),
      thread(CppCommon::Thread::CurrentThreadId()),
      level(Level::INFO)
{
    logger.reserve(32);
    message.reserve(512);
    buffer.reserve(1024);
    raw.reserve(512);
}

enum class ArgumentType : uint8_t
{
    ARG_UNKNOWN,
    ARG_BOOL,
    ARG_CHAR,
    ARG_WCHAR,
    ARG_INT8,
    ARG_UINT8,
    ARG_INT16,
    ARG_UINT16,
    ARG_INT32,
    ARG_UINT32,
    ARG_INT64,
    ARG_UINT64,
    ARG_FLOAT,
    ARG_DOUBLE,
    ARG_STRING,
    ARG_POINTER,
    ARG_NAMEDARG,
    ARG_CUSTOM
};

inline size_t SerializeArgument(std::vector<uint8_t>& buffer)
{
    return 0;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, bool argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_BOOL);

    uint8_t value = argument;

    // Append the argument value
    size_t size = sizeof(value);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &value, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, char argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_CHAR);

    uint8_t value = argument;

    // Append the argument value
    size_t size = sizeof(value);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &value, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, wchar_t argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_WCHAR);

    uint32_t value = argument;

    // Append the argument value
    size_t size = sizeof(value);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &value, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, int8_t argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_INT8);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, uint8_t argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_UINT8);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, int16_t argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_INT16);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, uint16_t argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_UINT16);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, int32_t argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_INT32);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, uint32_t argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_UINT32);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, int64_t argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_INT64);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, uint64_t argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_UINT64);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, float argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_FLOAT);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, double argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_DOUBLE);

    // Append the argument value
    size_t size = sizeof(argument);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, const char* argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_STRING);

    uint32_t length = (uint32_t)std::strlen(argument);

    // Append the string length
    size_t size = sizeof(length);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &length, size);

    // Append the string value
    size = length;
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, argument, size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, std::string_view argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_STRING);

    uint32_t length = (uint32_t)argument.length();

    // Append the string length
    size_t size = sizeof(length);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &length, size);

    // Append the string value
    size = length;
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, argument.data(), size);
    return 1;
}

inline size_t SerializeArgument(std::vector<uint8_t>& buffer, const std::string& argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_STRING);

    uint32_t length = (uint32_t)argument.length();

    // Append the string length
    size_t size = sizeof(length);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &length, size);

    // Append the string value
    size = length;
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, argument.data(), size);
    return 1;
}

template <typename T>
inline size_t SerializeArgument(std::vector<uint8_t>& buffer, T* argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_POINTER);

    uint64_t value = (uint64_t)argument;

    // Append the pointer value
    size_t size = sizeof(value);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &value, size);
    return 1;
}

template <typename T>
inline size_t SerializeArgument(std::vector<uint8_t>& buffer, const T* argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_POINTER);

    uint64_t value = (uint64_t)argument;

    // Append the pointer value
    size_t size = sizeof(value);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &value, size);
    return 1;
}

template <typename T>
inline size_t SerializeArgument(std::vector<uint8_t>& buffer, const fmt::internal::named_arg<T, char>& argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_NAMEDARG);

    uint32_t length = (uint32_t)argument.name.size();

    // Append the argument name length
    size_t size = sizeof(length);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &length, size);

    // Append the argument name value
    size = length;
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, argument.name.data(), size);

    SerializeArgument(buffer, argument.value);

    return 1;
}

template <typename T>
inline size_t SerializeArgument(std::vector<uint8_t>& buffer, const T& argument)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_CUSTOM);

    // Serialize the custom argument
    size_t offset = buffer.size();
    buffer.resize(buffer.size() + sizeof(uint32_t));
    argument.StoreFormat(buffer);
    size_t size = buffer.size() - offset;
    std::memcpy(buffer.data() + offset, &size, sizeof(uint32_t));

    return 1;
}

template <typename T, typename... Args>
inline size_t SerializeArgument(std::vector<uint8_t>& buffer, const T& argument, const Args&... args)
{
    size_t result1 = SerializeArgument(buffer, argument);
    size_t result2 = SerializeArgument(buffer, args...);
    return result1 + result2;
}

template <typename... Args>
inline void Record::Format(std::string_view pattern, const Args&... args)
{
    message = CppCommon::format(pattern, args...);
}

template <typename... Args>
inline void Record::StoreFormat(std::string_view pattern, const Args&... args)
{
    message = pattern;

    // Serialize arguments
    size_t offset = buffer.size();
    buffer.resize(buffer.size() + sizeof(uint32_t));
    size_t count = SerializeArgument(buffer, args...);
    std::memcpy(buffer.data() + offset, &count, sizeof(uint32_t));
}

template <typename... Args>
inline void Record::StoreFormat(std::vector<uint8_t>& buffer, std::string_view pattern, const Args&... args)
{
    uint32_t length = (uint32_t)pattern.size();

    // Append the pattern length
    size_t size = sizeof(length);
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, &length, size);

    // Append the pattern value
    size = length;
    buffer.resize(buffer.size() + size);
    std::memcpy(buffer.data() + buffer.size() - size, pattern.data(), size);

    // Serialize arguments
    size_t offset = buffer.size();
    buffer.resize(buffer.size() + sizeof(uint32_t));
    size_t count = SerializeArgument(buffer, args...);
    std::memcpy(buffer.data() + offset, &count, sizeof(uint32_t));
}

inline void Record::Clear()
{
    timestamp = 0;
    thread = 0;
    level = Level::NONE;
    logger.clear();
    message.clear();
    buffer.clear();
    raw.clear();
}

inline void Record::swap(Record& record) noexcept
{
    using std::swap;
    swap(timestamp, record.timestamp);
    swap(thread, record.thread);
    swap(level, record.level);
    swap(logger, record.logger);
    swap(message, record.message);
    swap(buffer, record.buffer);
    swap(raw, record.raw);
}

inline void swap(Record& record1, Record& record2) noexcept
{
    record1.swap(record2);
}

} // namespace CppLogging
