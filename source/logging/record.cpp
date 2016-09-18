/*!
    \file record.cpp
    \brief Logging record implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/record.h"

namespace CppLogging {

void Record::InitFormat(const char* pattern, fmt::ArgList args)
{
    // Calculate the count of format arguments
    unsigned count = 1;
    while (args[count - 1].type != fmt::internal::Arg::NONE)
        ++count;

    // Special check for none format arguments
    if (count == 1)
        return;

    // Reserve space for format arguments
    if (count <= fmt::ArgList::MAX_PACKED_ARGS)
        format.resize(sizeof(count) + sizeof(fmt::ULongLong) + sizeof(fmt::internal::Value) * count);
    else
        format.resize(sizeof(count) + sizeof(fmt::ULongLong) + sizeof(fmt::internal::Arg) * count);

    uint8_t* data = format.data();

    // Serialize the count of format arguments
    std::memcpy(data, &count, sizeof(unsigned));
    data += sizeof(unsigned);

    // Serialize types of format arguments
    fmt::ULongLong types = args.types();
    std::memcpy(data, &types, sizeof(fmt::ULongLong));
    data += sizeof(fmt::ULongLong);

    // Serialize values of format arguments
    for (unsigned i = 0; i < count; ++i)
    {
        fmt::internal::Arg arg = args[i];
        if (count <= fmt::ArgList::MAX_PACKED_ARGS)
        {
            std::memcpy(data, &arg, sizeof(fmt::internal::Value));
            data += sizeof(fmt::internal::Value);
        }
        else
        {
            std::memcpy(data, &arg, sizeof(fmt::internal::Arg));
            data += sizeof(fmt::internal::Arg);
        }
    }

    // Store message pattern
    message = pattern;
}

void Record::FormatMessage()
{
    // Special check for none format arguments
    if (format.empty())
        return;

    // Get the data buffer start position
    const uint8_t* data = format.data();

    // Deserialize the count of format arguments
    unsigned count;
    std::memcpy(&count, data, sizeof(unsigned));
    data += sizeof(unsigned);

    // Deserialize types of format arguments
    fmt::ULongLong types;
    std::memcpy(&types, data, sizeof(fmt::ULongLong));
    data += sizeof(fmt::ULongLong);

    // Reserve space for format arguments values
    std::vector<fmt::internal::Value> values;
    std::vector<fmt::internal::Arg> args;
    if (count <= fmt::ArgList::MAX_PACKED_ARGS)
        values.resize(count);
    else
        args.resize(count);

    // Deserialize values of format arguments
    for (unsigned i = 0; i < count; ++i)
    {
        if (count <= fmt::ArgList::MAX_PACKED_ARGS)
        {
            std::memcpy(&values[i], data, sizeof(fmt::internal::Value));
            data += sizeof(fmt::internal::Value);
        }
        else
        {
            std::memcpy(&args[i], data, sizeof(fmt::internal::Arg));
            data += sizeof(fmt::internal::Arg);
        }
    }

    // Format the message filed of the logging record
    if (count <= fmt::ArgList::MAX_PACKED_ARGS)
    {
        fmt::ArgList arg(types, values.data());
        message = fmt::format(message.c_str(), arg);
    }
    else
    {
        fmt::ArgList arg(types, args.data());
        message = fmt::format(message.c_str(), arg);
    }
}

} // namespace CppLogging
