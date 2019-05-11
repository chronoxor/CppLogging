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
    buffer.reserve(512);
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
    ARG_CUSTOM,
    ARG_LIST
};

inline void SerializeArgument(Record& record)
{
}

inline void SerializeArgument(Record& record, bool argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_BOOL);

    uint8_t value = argument;

    // Append the argument value
    size_t size = sizeof(value);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &value, size);
}

inline void SerializeArgument(Record& record, char argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_CHAR);

    uint8_t value = argument;

    // Append the argument value
    size_t size = sizeof(value);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &value, size);
}

inline void SerializeArgument(Record& record, wchar_t argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_WCHAR);

    uint32_t value = argument;

    // Append the argument value
    size_t size = sizeof(value);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &value, size);
}

inline void SerializeArgument(Record& record, int8_t argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_INT8);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);
}

inline void SerializeArgument(Record& record, uint8_t argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_UINT8);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);
}

inline void SerializeArgument(Record& record, int16_t argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_INT16);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);

}

inline void SerializeArgument(Record& record, uint16_t argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_UINT16);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);
}

inline void SerializeArgument(Record& record, int32_t argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_INT32);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);
}

inline void SerializeArgument(Record& record, uint32_t argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_UINT32);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);
}

inline void SerializeArgument(Record& record, int64_t argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_INT64);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);
}

inline void SerializeArgument(Record& record, uint64_t argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_UINT64);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);
}

#if defined(__APPLE__)

// Workaround for MacOS issue with template specialization for size_t
// https://stackoverflow.com/questions/11603818/why-is-there-ambiguity-between-uint32-t-and-uint64-t-when-using-size-t-on-mac-os
inline void SerializeArgument(Record& record, size_t argument)
{
    uint64_t arg = argument;

    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_UINT64);

    // Append the argument value
    size_t size = sizeof(arg);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &arg, size);
}

#endif

inline void SerializeArgument(Record& record, float argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_FLOAT);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);
}

inline void SerializeArgument(Record& record, double argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_DOUBLE);

    // Append the argument value
    size_t size = sizeof(argument);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &argument, size);
}

inline void SerializeArgument(Record& record, const char* argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_STRING);

    uint32_t length = (uint32_t)std::strlen(argument);

    // Append the string length
    size_t size = sizeof(length);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &length, size);

    // Append the string value
    size = length;
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, argument, size);
}

inline void SerializeArgument(Record& record, std::string_view argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_STRING);

    uint32_t length = (uint32_t)argument.length();

    // Append the string length
    size_t size = sizeof(length);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &length, size);

    // Append the string value
    size = length;
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, argument.data(), size);
}

inline void SerializeArgument(Record& record, const std::string& argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_STRING);

    uint32_t length = (uint32_t)argument.length();

    // Append the string length
    size_t size = sizeof(length);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &length, size);

    // Append the string value
    size = length;
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, argument.data(), size);
}

template <typename T>
inline void SerializeArgument(Record& record, T* argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_POINTER);

    uint64_t value = (uint64_t)argument;

    // Append the pointer value
    size_t size = sizeof(value);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &value, size);
}

template <typename T>
inline void SerializeArgument(Record& record, const T* argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_POINTER);

    uint64_t value = (uint64_t)argument;

    // Append the pointer value
    size_t size = sizeof(value);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &value, size);
}

template <typename T>
inline void SerializeArgument(Record& record, const fmt::internal::named_arg<T, char>& argument)
{
    // Append the argument type
    record.buffer.emplace_back((uint8_t)ArgumentType::ARG_NAMEDARG);

    uint32_t length = (uint32_t)argument.name.size();

    // Append the argument name length
    size_t size = sizeof(length);
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, &length, size);

    // Append the argument name value
    size = length;
    record.buffer.resize(record.buffer.size() + size);
    std::memcpy(record.buffer.data() + record.buffer.size() - size, argument.name.data(), size);

    SerializeArgument(record, argument.value);
}

template <typename T>
inline void SerializeArgument(Record& record, const T& argument)
{
    // Serialize the custom argument
    record << argument;
}

template <typename T, typename... Args>
inline void SerializeArgument(Record& record, const T& argument, const Args&... args)
{
    SerializeArgument(record, argument);
    SerializeArgument(record, args...);
}

template <typename... Args>
inline Record& Record::Format(std::string_view pattern, const Args&... args)
{
    message = CppCommon::format(pattern, args...);
    return *this;
}

template <typename... Args>
inline Record& Record::StoreFormat(std::string_view pattern, const Args&... args)
{
    message = pattern;
    SerializeArgument(*this, args...);
    return *this;
}

template <typename Arg>
inline Record& Record::StoreCustom(const Arg& arg)
{
    // Serialize argument
    SerializeArgument(*this, arg);

    return *this;
}

template <typename... Args>
inline Record& Record::StoreCustomFormat(std::string_view pattern, const Args&... args)
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_CUSTOM);

    size_t offset = buffer.size();
    buffer.resize(buffer.size() + sizeof(uint32_t));

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
    SerializeArgument(*this, args...);

    size = buffer.size() - offset;
    std::memcpy(buffer.data() + offset, &size, sizeof(uint32_t));

    return *this;
}

inline size_t Record::StoreListBegin()
{
    // Append the argument type
    buffer.emplace_back((uint8_t)ArgumentType::ARG_LIST);

    size_t offset = buffer.size();
    buffer.resize(buffer.size() + sizeof(uint32_t));

    return offset;
}

template <typename... Args>
inline Record& Record::StoreList(const Args&... args)
{
    // Serialize list arguments
    SerializeArgument(*this, args...);

    return *this;
}

template <typename... Args>
inline Record& Record::StoreListFormat(std::string_view pattern, const Args&... args)
{
    return StoreCustomFormat(pattern, args...);
}

inline Record& Record::StoreListEnd(size_t begin)
{
    size_t size = buffer.size() - begin;
    std::memcpy(buffer.data() + begin, &size, sizeof(uint32_t));

    return *this;
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
