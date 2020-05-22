/*!
    \file record.cpp
    \brief Logging record implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/record.h"

namespace {

std::string RestoreFormatString(std::string_view pattern, const std::vector<uint8_t>& buffer, size_t& offset, size_t size);

size_t ParseArgument(fmt::dynamic_format_arg_store<fmt::format_context>& store, const std::vector<uint8_t>& buffer, size_t& index)
{
    // Parse the argument type
    CppLogging::ArgumentType type;
    std::memcpy(&type, buffer.data() + index, sizeof(uint8_t));
    index += sizeof(uint8_t);

    // Parse the named argument name
    std::string name;
    if (type == CppLogging::ArgumentType::ARG_NAMEDARG)
    {
        uint32_t length;
        std::memcpy(&length, buffer.data() + index, sizeof(uint32_t));
        index += sizeof(uint32_t);

        name.resize(length);
        std::memcpy(name.data(), buffer.data() + index, length);
        index += length;

        // Parse the named argument type
        std::memcpy(&type, buffer.data() + index, sizeof(uint8_t));
        index += sizeof(uint8_t);
    }

    // Parse the argument value
    switch (type)
    {
        case CppLogging::ArgumentType::ARG_BOOL:
        {
            uint8_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(uint8_t));
            index += sizeof(uint8_t);

            name.empty() ? store.push_back(value != 0) : store.push_back(fmt::detail::named_arg(name.c_str(), value != 0));
            break;
        }
        case CppLogging::ArgumentType::ARG_CHAR:
        {
            uint8_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(uint8_t));
            index += sizeof(uint8_t);

            name.empty() ? store.push_back((char)value) : store.push_back(fmt::detail::named_arg(name.c_str(), (char)value));
            break;
        }
        case CppLogging::ArgumentType::ARG_WCHAR:
        {
            uint32_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(uint32_t));
            index += sizeof(uint32_t);

            name.empty() ? store.push_back((char)value) : store.push_back(fmt::detail::named_arg(name.c_str(), (char)value));
            break;
        }
        case CppLogging::ArgumentType::ARG_INT8:
        {
            int8_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(int8_t));
            index += sizeof(int8_t);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_UINT8:
        {
            uint8_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(uint8_t));
            index += sizeof(uint8_t);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_INT16:
        {
            int16_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(int16_t));
            index += sizeof(int16_t);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_UINT16:
        {
            uint16_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(uint16_t));
            index += sizeof(uint16_t);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_INT32:
        {
            int32_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(int32_t));
            index += sizeof(int32_t);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_UINT32:
        {
            uint32_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(uint32_t));
            index += sizeof(uint32_t);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_INT64:
        {
            int64_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(int64_t));
            index += sizeof(int64_t);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_UINT64:
        {
            uint64_t value;
            std::memcpy(&value, buffer.data() + index, sizeof(uint64_t));
            index += sizeof(uint64_t);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_FLOAT:
        {
            float value;
            std::memcpy(&value, buffer.data() + index, sizeof(float));
            index += sizeof(float);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_DOUBLE:
        {
            double value;
            std::memcpy(&value, buffer.data() + index, sizeof(double));
            index += sizeof(double);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
       }
        case CppLogging::ArgumentType::ARG_STRING:
        {
            uint32_t length;
            std::memcpy(&length, buffer.data() + index, sizeof(uint32_t));
            index += sizeof(uint32_t);

            const fmt::string_view value((const char*)buffer.data() + index, length);
            index += length;

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }
        case CppLogging::ArgumentType::ARG_POINTER:
        {
            const void* value;
            std::memcpy(&value, buffer.data() + index, sizeof(uint64_t));
            index += sizeof(uint64_t);

            name.empty() ? store.push_back(value) : store.push_back(fmt::detail::named_arg(name.c_str(), value));
            break;
        }            
        case CppLogging::ArgumentType::ARG_CUSTOM:
        {
            // Parse the custom data type size
            uint32_t custom_size;
            std::memcpy(&custom_size, buffer.data() + index, sizeof(uint32_t));
            index += sizeof(uint32_t);
            custom_size -= sizeof(uint32_t);

            // Parse the pattern length
            uint32_t custom_pattern_length;
            std::memcpy(&custom_pattern_length, buffer.data() + index, sizeof(uint32_t));
            index += sizeof(uint32_t);
            custom_size -= sizeof(uint32_t);

            // Parse the pattern value
            std::string custom_pattern;
            custom_pattern.resize(custom_pattern_length);
            std::memcpy(custom_pattern.data(), buffer.data() + index, custom_pattern_length);
            index += custom_pattern_length;
            custom_size -= custom_pattern_length;

            const std::string custom = RestoreFormatString(custom_pattern, buffer, index, custom_size);

            name.empty() ? store.push_back(custom) : store.push_back(fmt::detail::named_arg(name.c_str(), custom));
            break;
        }
        case CppLogging::ArgumentType::ARG_LIST:
        {
            // Parse the list size
            uint32_t list_size;
            std::memcpy(&list_size, buffer.data() + index, sizeof(uint32_t));
            index += sizeof(uint32_t);
            list_size -= sizeof(uint32_t);

            fmt::dynamic_format_arg_store<fmt::format_context> list_store;

            // Write arguments from the list
            size_t list_items = 0;
            size_t list_offset = index;
            while (index < (list_offset + list_size))
            {
                if (ParseArgument(list_store, buffer, index))
                    ++list_items;
                else
                    return false;
            }

            // Prepare list format pattern
            std::string list_pattern;
            list_pattern.reserve(2 * list_items);
            for (size_t i = 0; i < list_items; ++i)
                list_pattern.append("{}");

            // Perform list format operation
            const std::string list_string = fmt::vformat(list_pattern, list_store);

            name.empty() ? store.push_back(list_string) : store.push_back(fmt::detail::named_arg(name.c_str(), list_string));
            break;
        }
        default:
        {
            assert(false && "Unsupported argument type!");
            return false;
        }
    }

    return true;
}

std::string RestoreFormatString(std::string_view pattern, const std::vector<uint8_t>& buffer, size_t& offset, size_t size)
{
    fmt::dynamic_format_arg_store<fmt::format_context> store;

    // Parse format arguments from the buffer and prepare dynamic format storage
    size_t index = offset;
    while (index < (offset + size))
        if (!ParseArgument(store, buffer, index))
            break;
    offset = index;

    // Perform format operation
    std::string result = fmt::vformat(pattern, store);

    return result;
}

} // namespace

namespace CppLogging {

std::string Record::RestoreFormat(std::string_view pattern, const std::vector<uint8_t>& buffer, size_t offset, size_t size)
{
    size_t index = offset;
    return RestoreFormatString(pattern, buffer, index, size);
}

} // namespace CppLogging
