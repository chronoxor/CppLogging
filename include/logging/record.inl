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

inline void AlignBuffer(uint8_t*& buffer, size_t align)
{
    size_t offset = (uintptr_t)buffer % align;
    buffer += (offset != 0) ? (align - offset) : 0;
}

inline void AlignSerialize(uint8_t*& buffer, const void* data, size_t size, size_t align)
{
    AlignBuffer(buffer, align);
    std::memcpy(buffer, data, size);
    buffer += size;
}

template <typename T>
inline void AlignSerialize(uint8_t*& buffer, const T& value)
{
    AlignBuffer(buffer, alignof(T));
    std::memcpy(buffer, &value, sizeof(T));
    buffer += sizeof(T);
}

template <typename T>
inline void AlignDeserialize(uint8_t*& buffer, T& value)
{
    AlignBuffer(buffer, alignof(T));
    std::memcpy(&value, buffer, sizeof(T));
    buffer += sizeof(T);
}

template <typename Char, typename T>
std::size_t CalculateExtraSize(const fmt::internal::Arg& arg)
{
    std::size_t result = 0;

    if (arg.type == fmt::internal::Arg::CSTRING)
        result = alignof(std::size_t) + sizeof(std::size_t) + alignof(char) + (std::strlen(arg.string.value) + 1) * sizeof(char);
    else if (arg.type == fmt::internal::Arg::STRING)
        result = alignof(std::size_t) + sizeof(std::size_t) + alignof(char) + arg.string.size * sizeof(char);
    else if (arg.type == fmt::internal::Arg::WSTRING)
        result = alignof(std::size_t) + sizeof(std::size_t) + alignof(wchar_t) + arg.wstring.size * sizeof(wchar_t);
    else if (arg.type == fmt::internal::Arg::CUSTOM)
        result = alignof(T) + sizeof(T);

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
    extra_size += alignof(fmt::internal::NamedArgWithType<Char, T>) + sizeof(fmt::internal::NamedArgWithType<Char, T>) + alignof(std::size_t) + sizeof(std::size_t) + alignof(Char) + argument.name.size() * sizeof(Char) + CalculateExtraSize<Char, fmt::internal::NamedArgWithType<Char, T>>(argument);
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
        AlignSerialize(data_buffer, size);
        AlignSerialize(data_buffer, value.string.value, size, sizeof(char));
    }
    else if (type == fmt::internal::Arg::STRING)
    {
        std::size_t size = value.string.size * sizeof(char);
        AlignSerialize(data_buffer, size);
        AlignSerialize(data_buffer, value.string.value, size, sizeof(char));
    }
    else if (type == fmt::internal::Arg::WSTRING)
    {
        std::size_t size = value.wstring.size * sizeof(wchar_t);
        AlignSerialize(data_buffer, size);
        AlignSerialize(data_buffer, value.wstring.value, size, sizeof(wchar_t));
    }
    else if (type == fmt::internal::Arg::CUSTOM)
    {
        std::size_t size = sizeof(T);
        AlignSerialize(data_buffer, size);
        AlignSerialize(data_buffer, value.custom.value, size, size);
    }
}

template <typename Char, typename T>
void SerializeExtraData(uint8_t*& data_buffer, const fmt::internal::NamedArgWithType<Char, T>& named)
{
    AlignSerialize(data_buffer, named);
    std::size_t size = named.name.size() * sizeof(Char);
    AlignSerialize(data_buffer, size);
    AlignSerialize(data_buffer, named.name.data(), size, sizeof(Char));
    SerializeExtraData<Char, T>(data_buffer, named.type, named);
}

template <typename Char>
void SerializeArguments(std::size_t, std::size_t, uint8_t*&, uint8_t*&)
{
}

template <typename Char, typename T>
void SerializeArguments(std::size_t item_align, std::size_t item_size, uint8_t*& base_buffer, uint8_t*& data_buffer, const T& argument)
{
    // Serialize argument
    fmt::internal::MakeArg<fmt::BasicFormatter<Char>> arg(argument);
    AlignSerialize(base_buffer, &arg, item_size, item_align);
    // Serialize extra data
    SerializeExtraData<Char, T>(data_buffer, arg.type, arg);
}

template <typename Char, typename T>
void SerializeArguments(std::size_t item_align, std::size_t item_size, uint8_t*& base_buffer, uint8_t*& data_buffer, const fmt::internal::NamedArgWithType<Char, T>& argument)
{
    // Serialize argument
    const fmt::internal::Arg& arg = argument;
    AlignSerialize(base_buffer, &arg, item_size, item_align);
    // Serialize extra data
    SerializeExtraData<Char, T>(data_buffer, argument);
}

template <typename Char, typename T, typename... Args>
void SerializeArguments(std::size_t item_align, std::size_t item_size, uint8_t*& base_buffer, uint8_t*& data_buffer, const T& argument, const Args&... args)
{
    SerializeArguments<Char>(item_align, item_size, base_buffer, data_buffer, argument);
    SerializeArguments<Char>(item_align, item_size, base_buffer, data_buffer, args...);
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
    std::size_t item_align = (count > fmt::ArgList::MAX_PACKED_ARGS) ? alignof(fmt::internal::Arg) : alignof(fmt::internal::Value);
    std::size_t item_size = (count > fmt::ArgList::MAX_PACKED_ARGS) ? sizeof(fmt::internal::Arg) : sizeof(fmt::internal::Value);
    std::size_t base_size = alignof(unsigned) + sizeof(unsigned) + alignof(std::size_t) + sizeof(std::size_t) + alignof(fmt::ULongLong) + sizeof(fmt::ULongLong) + item_align + count * item_size;
    std::size_t full_size = base_size + extra_size;

    // Resize buffer to fit all format arguments
    buffer.resize(full_size);

    uint8_t* base_buffer = buffer.data();
    uint8_t* data_buffer = base_buffer + base_size;

    // Serialize the count of format arguments
    AlignSerialize(base_buffer, count);

    // Serialize the base buffer size
    AlignSerialize(base_buffer, base_size);

    // Serialize types of format arguments
    fmt::ULongLong types = fmt::internal::make_type(args...);
    AlignSerialize(base_buffer, types);

    // Serialize values of format arguments
    SerializeArguments<Char>(item_align, item_size, base_buffer, data_buffer, args...);
}

template <typename Char>
void DeserializeExtraData(uint8_t*& data_buffer, fmt::internal::Arg::Type type, fmt::internal::Value& value)
{
    // Deserialize extra data
    if (type == fmt::internal::Arg::NAMED_ARG)
    {
        AlignBuffer(data_buffer, sizeof(fmt::internal::NamedArg<Char>));
        fmt::internal::NamedArg<Char>* named = (fmt::internal::NamedArg<Char>*)data_buffer;
        value.pointer = named;
        data_buffer += sizeof(fmt::internal::NamedArg<Char>);
        std::size_t size;
        AlignDeserialize(data_buffer, size);
        AlignBuffer(data_buffer, sizeof(Char));
        named->name = fmt::BasicStringRef<Char>((const Char*)data_buffer, size);
        data_buffer += size;
        DeserializeExtraData<Char>(data_buffer, named->type, *named);
    }
    else if (type == fmt::internal::Arg::CSTRING)
    {
        std::size_t size;
        AlignDeserialize(data_buffer, size);
        AlignBuffer(data_buffer, sizeof(char));
        value.string.value = (const char*)data_buffer;
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::STRING)
    {
        std::size_t size;
        AlignDeserialize(data_buffer, size);
        AlignBuffer(data_buffer, sizeof(char));
        value.string.value = (const char*)data_buffer;
        value.string.size = size / sizeof(char);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::WSTRING)
    {
        std::size_t size;
        AlignDeserialize(data_buffer, size);
        AlignBuffer(data_buffer, sizeof(wchar_t));
        value.wstring.value = (const wchar_t*)data_buffer;
        value.wstring.size = size / sizeof(wchar_t);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::CUSTOM)
    {
        std::size_t size;
        AlignDeserialize(data_buffer, size);
        AlignBuffer(data_buffer, size);
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

    // Deserialize the count of format arguments
    unsigned count;
    AlignDeserialize(base_buffer, count);

    // Deserialize the base buffer size
    std::size_t base_size;
    AlignDeserialize(base_buffer, base_size);

    // Update the data buffer offset
    uint8_t* data_buffer = buffer.data() + base_size;

    // Deserialize types of format arguments
    fmt::ULongLong types;
    AlignDeserialize(base_buffer, types);

    // Calculate the item size
    std::size_t item_align = (count > fmt::ArgList::MAX_PACKED_ARGS) ? alignof(fmt::internal::Arg) : alignof(fmt::internal::Value);
    std::size_t item_size = (count > fmt::ArgList::MAX_PACKED_ARGS) ? sizeof(fmt::internal::Arg) : sizeof(fmt::internal::Value);

    // Align base buffer
    AlignBuffer(base_buffer, item_align);

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
