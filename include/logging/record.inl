/*!
    \file record.inl
    \brief Logging record inline implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

namespace CppLogging {

//! @cond INTERNALS
namespace Internals {

template <typename Char, typename T>
std::size_t CalculateExtraSize(const fmt::internal::Arg& arg)
{
    std::size_t result = 0;

    if (arg.type == fmt::internal::Arg::CSTRING)
        result = sizeof(std::size_t) + (std::strlen(arg.string.value) + 1) * sizeof(char);
    else if (arg.type == fmt::internal::Arg::STRING)
        result = sizeof(std::size_t) + arg.string.size * sizeof(char);
    else if (arg.type == fmt::internal::Arg::WSTRING)
        result = sizeof(std::size_t) + arg.wstring.size * sizeof(wchar_t);
    else if (arg.type == fmt::internal::Arg::CUSTOM)
        result = sizeof(T);

    return result;
}

template <typename Char>
unsigned CalculateBufferSize(std::size_t&)
{
    return 0;
}

template <typename Char, typename T>
unsigned CalculateBufferSize(std::size_t& extra_size, const T& argument)
{
    fmt::internal::MakeArg<fmt::BasicFormatter<Char>> arg(argument);
    extra_size += CalculateExtraSize<Char, T>(arg);
    return 1;
}

template <typename Char, typename T>
unsigned CalculateBufferSize(std::size_t& extra_size, const fmt::internal::NamedArgWithType<Char, T>& argument)
{
    extra_size += sizeof(fmt::internal::NamedArgWithType<Char, T>) + sizeof(std::size_t) + argument.name.size() * sizeof(Char) + CalculateExtraSize<Char, fmt::internal::NamedArgWithType<Char, T>>(argument);
    return 1;
}

template <typename Char, typename T, typename... Args>
unsigned CalculateBufferSize(std::size_t& extra_size, const T& argument, const Args&... args)
{
    unsigned count = 0;
    count += CalculateBufferSize<Char>(extra_size, argument);
    count += CalculateBufferSize<Char>(extra_size, args...);
    return count;
}

template <typename Char, typename T>
void SerializeExtraData(uint8_t*& data_buffer, fmt::internal::Arg::Type type, const fmt::internal::Value& value)
{
    // Serialize extra data
    if (type == fmt::internal::Arg::CSTRING)
    {
        std::size_t size = (std::strlen(value.string.value) + 1) * sizeof(char);
        std::memcpy(data_buffer, &size, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        std::memcpy(data_buffer, value.string.value, size);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::STRING)
    {
        std::size_t size = value.string.size * sizeof(char);
        std::memcpy(data_buffer, &size, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        std::memcpy(data_buffer, value.string.value, size);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::WSTRING)
    {
        std::size_t size = value.wstring.size * sizeof(wchar_t);
        std::memcpy(data_buffer, &size, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        std::memcpy(data_buffer, value.wstring.value, size);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::CUSTOM)
    {
        std::size_t size = sizeof(T);
        std::memcpy(data_buffer, &size, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        std::memcpy(data_buffer, value.custom.value, size);
        data_buffer += size;
    }
}

template <typename Char, typename T>
void SerializeExtraData(uint8_t*& data_buffer, const fmt::internal::NamedArgWithType<Char, T>& named)
{
    std::memcpy(data_buffer, &named, sizeof(fmt::internal::NamedArgWithType<Char, T>));
    data_buffer += sizeof(fmt::internal::NamedArgWithType<Char, T>);
    std::size_t size = named.name.size() * sizeof(Char);
    std::memcpy(data_buffer, &size, sizeof(std::size_t));
    data_buffer += sizeof(std::size_t);
    std::memcpy(data_buffer, named.name.data(), size);
    data_buffer += size;
    SerializeExtraData<Char, T>(data_buffer, named.type, named);
}

template <typename Char>
void SerializeArguments(std::size_t, uint8_t*&, uint8_t*&)
{
}

template <typename Char, typename T>
void SerializeArguments(std::size_t item_size, uint8_t*& base_buffer, uint8_t*& data_buffer, const T& argument)
{
    // Serialize argument
    fmt::internal::MakeArg<fmt::BasicFormatter<Char>> arg(argument);
    std::memcpy(base_buffer, &arg, item_size);
    base_buffer += item_size;
    // Serialize extra data
    SerializeExtraData<Char, T>(data_buffer, arg.type, arg);
}

template <typename Char, typename T>
void SerializeArguments(std::size_t item_size, uint8_t*& base_buffer, uint8_t*& data_buffer, const fmt::internal::NamedArgWithType<Char, T>& argument)
{
    // Serialize argument
    const fmt::internal::Arg& arg = argument;
    std::memcpy(base_buffer, &arg, item_size);
    base_buffer += item_size;
    // Serialize extra data
    SerializeExtraData<Char, T>(data_buffer, argument);
}

template <typename Char, typename T, typename... Args>
void SerializeArguments(std::size_t item_size, uint8_t*& base_buffer, uint8_t*& data_buffer, const T& argument, const Args&... args)
{
    SerializeArguments<Char>(item_size, base_buffer, data_buffer, argument);
    SerializeArguments<Char>(item_size, base_buffer, data_buffer, args...);
}

template <typename Char, typename... Args>
void Serialize(std::vector<uint8_t>& buffer, const Args&... args)
{
    std::size_t extra_size = 0;
    unsigned count = CalculateBufferSize<Char>(extra_size, args...);

    // Special check for none format arguments
    if (count == 0)
        return;

    // Caclulate base & full buffer sizes
    std::size_t item_size = (count > fmt::ArgList::MAX_PACKED_ARGS) ? sizeof(fmt::internal::Arg) : sizeof(fmt::internal::Value);
    std::size_t base_size = sizeof(unsigned) + sizeof(std::size_t) + sizeof(fmt::ULongLong) + count * item_size;
    std::size_t full_size = base_size + extra_size;

    // Resize buffer to fit all format arguments
    buffer.resize(full_size);

    uint8_t* base_buffer = buffer.data();
    uint8_t* data_buffer = base_buffer + base_size;

    // Serialize the count of format arguments
    std::memcpy(base_buffer, &count, sizeof(unsigned));
    base_buffer += sizeof(unsigned);

    // Serialize the base buffer size
    std::memcpy(base_buffer, &base_size, sizeof(std::size_t));
    base_buffer += sizeof(std::size_t);

    // Serialize types of format arguments
    fmt::ULongLong types = fmt::internal::make_type(args...);
    std::memcpy(base_buffer, &types, sizeof(fmt::ULongLong));
    base_buffer += sizeof(fmt::ULongLong);

    // Serialize values of format arguments
    SerializeArguments<Char>(item_size, base_buffer, data_buffer, args...);
}

template <typename Char>
void DeserializeExtraData(uint8_t*& data_buffer, fmt::internal::Arg::Type type, fmt::internal::Value& value)
{
    // Deserialize extra data
    if (type == fmt::internal::Arg::NAMED_ARG)
    {
        fmt::internal::NamedArg<Char>* named = (fmt::internal::NamedArg<Char>*)data_buffer;
        value.pointer = named;
        data_buffer += sizeof(fmt::internal::NamedArg<Char>);
        std::size_t size;
        std::memcpy(&size, data_buffer, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        named->name = fmt::BasicStringRef<Char>((const Char*)data_buffer, size);
        data_buffer += size;
        DeserializeExtraData<Char>(data_buffer, named->type, *named);
    }
    else if (type == fmt::internal::Arg::CSTRING)
    {
        std::size_t size;
        std::memcpy(&size, data_buffer, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        value.string.value = (const char*)data_buffer;
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::STRING)
    {
        std::size_t size;
        std::memcpy(&size, data_buffer, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        value.string.value = (const char*)data_buffer;
        value.string.size = size / sizeof(char);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::WSTRING)
    {
        std::size_t size;
        std::memcpy(&size, data_buffer, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        value.wstring.value = (const wchar_t*)data_buffer;
        value.wstring.size = size / sizeof(wchar_t);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::CUSTOM)
    {
        std::size_t size;
        std::memcpy(&size, data_buffer, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        value.custom.value = data_buffer;
        data_buffer += size;
    }
}

template <typename Char>
fmt::ArgList Deserialize(std::vector<uint8_t>& buffer)
{
    // Special check for empty format arguments list
    if (buffer.empty())
        return fmt::ArgList();

    uint8_t* base_buffer = buffer.data();
    uint8_t* data_buffer = base_buffer;

    // Deserialize the count of format arguments
    unsigned count;
    std::memcpy(&count, base_buffer, sizeof(unsigned));
    base_buffer += sizeof(unsigned);

    // Deserialize the base buffer size
    std::size_t base_size;
    std::memcpy(&base_size, base_buffer, sizeof(std::size_t));
    base_buffer += sizeof(std::size_t);

    // Update the data buffer offset
    data_buffer += base_size;

    // Deserialize types of format arguments
    fmt::ULongLong types;
    std::memcpy(&types, base_buffer, sizeof(fmt::ULongLong));
    base_buffer += sizeof(fmt::ULongLong);

    // Calculate the item size
    std::size_t item_size = (count > fmt::ArgList::MAX_PACKED_ARGS) ? sizeof(fmt::internal::Arg) : sizeof(fmt::internal::Value);

    // Deserialize values of format arguments
    uint8_t* local_buffer = base_buffer;
    for (unsigned i = 0; i < count; ++i)
    {
        // Deserialize argument
        fmt::internal::Value* value = (fmt::internal::Value*)local_buffer;
        local_buffer += item_size;
        // Deserialize extra data
        DeserializeExtraData<Char>(data_buffer, fmt::ArgList::type(types, i), *value);
    }

    // Prepare and return arguments list stored in the provided buffer
    return (count > fmt::ArgList::MAX_PACKED_ARGS) ?
        fmt::ArgList(types, (const fmt::internal::Arg*)base_buffer) :
        fmt::ArgList(types, (const fmt::internal::Value*)base_buffer);
}

} // namespace Internals
//! @endcond

inline Record::Record()
    : timestamp(CppCommon::Timestamp::utc()),
      thread(CppCommon::Thread::CurrentThreadId()),
      level(Level::INFO)
{
    logger.reserve(32);
    message.reserve(512);
    buffer.reserve(1024);
    format.reserve(128);
    raw.reserve(512);
}

template <typename... Args>
inline void Record::FormatSerialize(const char* pattern, const Args&... args)
{
    // Save the format pattern
    message = pattern;

    // Serialize arguments list
    Internals::Serialize<char>(format, args...);
}

inline void Record::FormatDeserialize()
{
    // Special check for empty format arguments list
    if (format.empty())
        return;

    // Format the message filed of the logging record
    fmt::ArgList args = Internals::Deserialize<char>(format);
    message = fmt::format(message.c_str(), args);

    // Clear format buffer
    format.clear();
}

inline void Record::Clear()
{
    timestamp = 0;
    thread = 0;
    level = Level::NONE;
    logger.clear();
    message.clear();
    buffer.clear();
    format.clear();
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
    swap(format, record.format);
    swap(raw, record.raw);
}

inline void swap(Record& record1, Record& record2) noexcept
{
    record1.swap(record2);
}

} // namespace CppLogging
