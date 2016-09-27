/*!
    \file record.cpp
    \brief Logging record implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/record.h"

namespace CppLogging {

//! @cond INTERNALS
namespace Internals {

template <typename Char>
std::size_t CalculateExtraSize(const fmt::internal::Arg& arg)
{
    std::size_t result = 0;

    if (arg.type == fmt::internal::Arg::NAMED_ARG)
    {
        const fmt::internal::NamedArg<Char>* named = static_cast<const fmt::internal::NamedArg<Char>*>(arg.pointer);
        result = sizeof(fmt::internal::NamedArg<Char>) + sizeof(std::size_t) + named->name.size() * sizeof(Char) + CalculateExtraSize<Char>(*named);
    }
    else if (arg.type == fmt::internal::Arg::CSTRING)
        result = sizeof(std::size_t) + (std::strlen(arg.string.value) + 1) * sizeof(char);
    else if (arg.type == fmt::internal::Arg::STRING)
        result = sizeof(std::size_t) + arg.string.size * sizeof(char);
    else if (arg.type == fmt::internal::Arg::WSTRING)
        result = sizeof(std::size_t) + arg.wstring.size * sizeof(wchar_t);
    else if (arg.type == fmt::internal::Arg::CUSTOM)
        result = arg.custom.size;

    return result;
}

template <typename Char>
void SerializeExtraData(uint8_t*& data_buffer, fmt::internal::Arg::Type type, const fmt::internal::Value& value)
{
    // Serialize extra data
    if (type == fmt::internal::Arg::NAMED_ARG)
    {
        const fmt::internal::NamedArg<Char>* named = static_cast<const fmt::internal::NamedArg<Char>*>(value.pointer);
        std::memcpy(data_buffer, named, sizeof(fmt::internal::NamedArg<Char>));
        data_buffer += sizeof(fmt::internal::NamedArg<Char>);
        std::size_t size = named->name.size() * sizeof(Char);
        std::memcpy(data_buffer, &size, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        std::memcpy(data_buffer, named->name.data(), size);
        data_buffer += size;
        SerializeExtraData<Char>(data_buffer, named->type, *named);
    }
    else if (type == fmt::internal::Arg::CSTRING)
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
        std::memcpy(data_buffer, value.custom.value, value.custom.size);
        data_buffer += value.custom.size;
    }
}

template <typename Char>
void DeserializeExtraData(uint8_t*& data_buffer, fmt::internal::Arg::Type type, fmt::internal::Value& value)
{
    // Deserialize extra data
    if (type == fmt::internal::Arg::NAMED_ARG)
    {
        fmt::internal::NamedArg<Char>* named = reinterpret_cast<fmt::internal::NamedArg<Char>*>(data_buffer);
        value.pointer = named;
        data_buffer += sizeof(fmt::internal::NamedArg<Char>);
        std::size_t size;
        std::memcpy(&size, data_buffer, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        named->name = fmt::BasicStringRef<Char>(reinterpret_cast<const Char*>(data_buffer), size);
        data_buffer += size;
        DeserializeExtraData<Char>(data_buffer, named->type, *named);
    }
    else if (type == fmt::internal::Arg::CSTRING)
    {
        std::size_t size;
        std::memcpy(&size, data_buffer, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        value.string.value = reinterpret_cast<const char*>(data_buffer);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::STRING)
    {
        std::size_t size;
        std::memcpy(&size, data_buffer, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        value.string.value = reinterpret_cast<const char*>(data_buffer);
        value.string.size = size / sizeof(char);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::WSTRING)
    {
        std::size_t size;
        std::memcpy(&size, data_buffer, sizeof(std::size_t));
        data_buffer += sizeof(std::size_t);
        value.wstring.value = reinterpret_cast<const wchar_t*>(data_buffer);
        value.wstring.size = size / sizeof(wchar_t);
        data_buffer += size;
    }
    else if (type == fmt::internal::Arg::CUSTOM)
    {
        value.custom.value = data_buffer;
        data_buffer += value.custom.size;
    }
}

template <typename Char>
void Serialize(const fmt::ArgList& args, std::vector<uint8_t>& buffer)
{
    // Calculate the count of format arguments
    unsigned count = 1;
    while (args[count - 1].type != fmt::internal::Arg::NONE)
        ++count;

    // Special check for none format arguments
    if (count == 1)
        return;

    // Caclulate base & full buffer sizes
    std::size_t item_size = (count > fmt::ArgList::MAX_PACKED_ARGS) ? sizeof(fmt::internal::Arg) : sizeof(fmt::internal::Value);
    std::size_t base_size = sizeof(count) + sizeof(std::size_t) + sizeof(fmt::ULongLong) + count * item_size;
    std::size_t full_size = base_size;
    for (unsigned i = 0; i < count; ++i)
        full_size += CalculateExtraSize<Char>(args[i]);

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
    fmt::ULongLong types = args.types();
    std::memcpy(base_buffer, &types, sizeof(fmt::ULongLong));
    base_buffer += sizeof(fmt::ULongLong);

    // Serialize values of format arguments
    for (unsigned i = 0; i < count; ++i)
    {
        // Serialize argument
        fmt::internal::Arg arg = args[i];
        std::memcpy(base_buffer, &arg, item_size);
        base_buffer += item_size;
        // Serialize extra data
        SerializeExtraData<Char>(data_buffer, arg.type, arg);
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
        fmt::internal::Value* value = reinterpret_cast<fmt::internal::Value*>(local_buffer);
        local_buffer += item_size;
        // Deserialize extra data
        DeserializeExtraData<Char>(data_buffer, fmt::ArgList::type(types, i), *value);
    }

    // Prepare and return arguments list stored in the provided buffer
    return (count > fmt::ArgList::MAX_PACKED_ARGS) ?
        fmt::ArgList(types, reinterpret_cast<const fmt::internal::Arg*>(base_buffer)) :
        fmt::ArgList(types, reinterpret_cast<const fmt::internal::Value*>(base_buffer));
}

} // namespace Internals
//! @endcond

void Record::Format()
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

void Record::Format(fmt::ArgList args)
{
    // Serialize arguments list
    Internals::Serialize<char>(args, format);
}

} // namespace CppLogging
