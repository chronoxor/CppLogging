/*!
    \file record.cpp
    \brief Logging record implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/record.h"

#include <map>

namespace {

bool IsDigit(char ch)
{
    return (ch >= '0') && (ch <= '9');
}

bool IsStartName(char ch)
{
    return ((ch >= 'A') && (ch <= 'Z')) || ((ch >= 'a') && (ch <= 'z')) || (ch == '_');
}

bool IsMiddleName(char ch)
{
    return IsStartName(ch) || IsDigit(ch);
}

struct Argument
{
    CppLogging::ArgumentType type;
    size_t offset;
    size_t size;

    bool GetUnsigned(const std::vector<uint8_t>& buffer, size_t& result) const
    {
        switch (type)
        {
            case CppLogging::ArgumentType::ARG_INT8:
            {
                int8_t value;
                std::memcpy(&value, buffer.data() + offset, sizeof(int8_t));
                result = (size_t)value;
                return true;
            }
            case CppLogging::ArgumentType::ARG_UINT8:
            {
                uint8_t value;
                std::memcpy(&value, buffer.data() + offset, sizeof(uint8_t));
                result = (size_t)value;
                return true;
            }
            case CppLogging::ArgumentType::ARG_INT16:
            {
                int16_t value;
                std::memcpy(&value, buffer.data() + offset, sizeof(int16_t));
                result = (size_t)value;
                return true;
            }
            case CppLogging::ArgumentType::ARG_UINT16:
            {
                uint16_t value;
                std::memcpy(&value, buffer.data() + offset, sizeof(uint16_t));
                result = (size_t)value;
                return true;
            }
            case CppLogging::ArgumentType::ARG_INT32:
            {
                int32_t value;
                std::memcpy(&value, buffer.data() + offset, sizeof(int32_t));
                result = (size_t)value;
                return true;
            }
            case CppLogging::ArgumentType::ARG_UINT32:
            {
                uint32_t value;
                std::memcpy(&value, buffer.data() + offset, sizeof(uint32_t));
                result = (size_t)value;
                return true;
            }
            case CppLogging::ArgumentType::ARG_INT64:
            {
                int64_t value;
                std::memcpy(&value, buffer.data() + offset, sizeof(int64_t));
                result = (size_t)value;
                return true;
            }
            case CppLogging::ArgumentType::ARG_UINT64:
            {
                uint64_t value;
                std::memcpy(&value, buffer.data() + offset, sizeof(uint64_t));
                result = (size_t)value;
                return true;
            }
            default:
            {
                assert(false && "Unsupported argument type!");
                return false;
            }
        }
    }
};

struct Arguments
{
    std::vector<Argument> arguments;
    std::map<std::string, Argument> named_arguments;
};

Argument ParseArgument(const std::vector<uint8_t>& buffer, size_t index)
{
    // Parse the argument type
    CppLogging::ArgumentType type;
    std::memcpy(&type, buffer.data() + index, sizeof(uint8_t));
    index += sizeof(uint8_t);

    // Parse the argument value
    switch (type)
    {
        case CppLogging::ArgumentType::ARG_BOOL:
            return Argument{ type, index, sizeof(uint8_t) };
        case CppLogging::ArgumentType::ARG_CHAR:
            return Argument{ type, index, sizeof(uint8_t) };
        case CppLogging::ArgumentType::ARG_WCHAR:
            return Argument{ type, index, sizeof(uint32_t) };
        case CppLogging::ArgumentType::ARG_INT8:
            return Argument{ type, index, sizeof(int8_t) };
        case CppLogging::ArgumentType::ARG_UINT8:
            return Argument{ type, index, sizeof(uint8_t) };
        case CppLogging::ArgumentType::ARG_INT16:
            return Argument{ type, index, sizeof(int16_t) };
        case CppLogging::ArgumentType::ARG_UINT16:
            return Argument{ type, index, sizeof(uint16_t) };
        case CppLogging::ArgumentType::ARG_INT32:
            return Argument{ type, index, sizeof(int32_t) };
        case CppLogging::ArgumentType::ARG_UINT32:
            return Argument{ type, index, sizeof(uint32_t) };
        case CppLogging::ArgumentType::ARG_INT64:
            return Argument{ type, index, sizeof(int64_t) };
        case CppLogging::ArgumentType::ARG_UINT64:
            return Argument{ type, index, sizeof(uint64_t) };
        case CppLogging::ArgumentType::ARG_FLOAT:
            return Argument{ type, index, sizeof(float) };
        case CppLogging::ArgumentType::ARG_DOUBLE:
            return Argument{ type, index, sizeof(double) };
        case CppLogging::ArgumentType::ARG_STRING:
        {
            uint32_t size;
            std::memcpy(&size, buffer.data() + index, sizeof(uint32_t));

            return Argument{ type, index, sizeof(uint32_t) + size };
        }
        case CppLogging::ArgumentType::ARG_POINTER:
            return Argument{ type, index, sizeof(uint64_t) };
        case CppLogging::ArgumentType::ARG_CUSTOM:
        {
            uint32_t size;
            std::memcpy(&size, buffer.data() + index, sizeof(uint32_t));

            return Argument{ type, index, size };
        }
        case CppLogging::ArgumentType::ARG_LIST:
        {
            uint32_t size;
            std::memcpy(&size, buffer.data() + index, sizeof(uint32_t));

            return Argument{ type, index, size };
        }
        default:
        {
            assert(false && "Unsupported argument type!");
            return Argument{ CppLogging::ArgumentType::ARG_UNKNOWN, 0, 0 };
        }
    }
}

Arguments ParseArguments(const std::vector<uint8_t>& buffer, size_t offset, size_t size)
{
    Arguments result;

    size_t index = offset;

    while (index < (offset + size))
    {
        // Parse the argument type
        CppLogging::ArgumentType type;
        std::memcpy(&type, buffer.data() + index, sizeof(uint8_t));

        if (type == CppLogging::ArgumentType::ARG_NAMEDARG)
        {
            index += sizeof(uint8_t);

            uint32_t length;
            std::memcpy(&length, buffer.data() + index, sizeof(uint32_t));
            index += sizeof(uint32_t);

            std::string name;
            name.resize(length);
            std::memcpy(name.data(), buffer.data() + index, length);
            index += length;

            Argument argument = ParseArgument(buffer, index);
            index += sizeof(uint8_t) + argument.size;
            result.named_arguments[name] = argument;
        }
        else
        {
            Argument argument = ParseArgument(buffer, index);
            index += sizeof(uint8_t) + argument.size;
            result.arguments.emplace_back(argument);
        }
    }

    return result;
}

bool ParseUnsigned(std::string_view::iterator& it, const std::string_view::iterator& end, size_t& result)
{
    result = 0;
    size_t max_int = std::numeric_limits<int>::max();
    size_t big_int = max_int / 10;
    char current = *it;
    while (IsDigit(current))
    {
        // Check for overflow.
        if (result > big_int)
        {
            result = max_int + 1;
            break;
        }
        result = 10 * result + (current - '0');
        if (++it == end)
        {
            assert(false && "Invalid format message! Unmatched '}' in the format string.");
            return false;
        }
        current = *it;
    }
    if (result > max_int)
    {
        assert(false && "Invalid format message! Number is too big.");
        return false;
    }
    return true;
}

bool ParseName(std::string_view::iterator& it, const std::string_view::iterator& end, std::string& result)
{
    result = "";
    char current = *it;
    if (IsStartName(current))
    {
        result.push_back(current);
        while (it != end)
        {
            if (++it == end)
            {
                assert(false && "Invalid format message! Unmatched '}' in the format string.");
                return false;
            }
            current = *it;
            if (IsMiddleName(current))
                result.push_back(current);
            else
                break;
        }
    }
    return true;
}

bool WriteFormatArgument(fmt::v5::basic_writer<fmt::v5::back_insert_range<std::string>>& writer, const fmt::v5::format_specs& specs, const Argument& argument, const std::vector<uint8_t> buffer)
{
    switch (argument.type)
    {
        case CppLogging::ArgumentType::ARG_BOOL:
        {
            uint8_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint8_t));
            writer.write(value != 0, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_CHAR:
        {
            uint8_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint8_t));
            writer.write((char)value);
            return true;
        }
        case CppLogging::ArgumentType::ARG_WCHAR:
        {
            uint32_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint32_t));
            writer.write((char)value);
            return true;
        }
        case CppLogging::ArgumentType::ARG_INT8:
        {
            int8_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(int8_t));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_UINT8:
        {
            uint8_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint8_t));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_INT16:
        {
            int16_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(int16_t));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_UINT16:
        {
            uint16_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint16_t));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_INT32:
        {
            int32_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(int32_t));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_UINT32:
        {
            uint32_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint32_t));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_INT64:
        {
            int64_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(int64_t));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_UINT64:
        {
            uint64_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint64_t));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_FLOAT:
        {
            float value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(float));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_DOUBLE:
        {
            double value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(double));
            writer.write(value, specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_STRING:
        {
            uint32_t length;
            std::memcpy(&length, buffer.data() + argument.offset, sizeof(uint32_t));

            writer.write(fmt::string_view((const char*)buffer.data() + argument.offset + sizeof(uint32_t), length), specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_POINTER:
        {
            uint64_t value;
            std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint64_t));
            writer.write_pointer((uintptr_t)value, &specs);
            return true;
        }
        case CppLogging::ArgumentType::ARG_CUSTOM:
        {
            size_t index = argument.offset;

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

            std::string custom = CppLogging::Record::RestoreFormat(custom_pattern, buffer, index, custom_size);

            writer.write(custom);
            return true;
        }
        case CppLogging::ArgumentType::ARG_LIST:
        {
            size_t index = argument.offset;

            // Parse the list size
            uint32_t list_size;
            std::memcpy(&list_size, buffer.data() + index, sizeof(uint32_t));
            index += sizeof(uint32_t);
            list_size -= sizeof(uint32_t);

            // Write arguments from the list
            while (index < (argument.offset + sizeof(uint32_t) + list_size))
            {
                Argument arg = ParseArgument(buffer, index);
                if (!WriteFormatArgument(writer, specs, arg, buffer))
                    return false;
                index += sizeof(uint8_t) + arg.size;
            }

            return true;
        }
        default:
        {
            assert(false && "Unsupported argument type!");
            return false;
        }
    }
}

} // namespace

namespace CppLogging {

std::string Record::RestoreFormat(std::string_view pattern, const std::vector<uint8_t> buffer, size_t offset, size_t size)
{
    std::string result;

    // Parse arguments and check arguments count before formatting
    auto args = ParseArguments(buffer, offset, size);
    if (args.arguments.empty() && args.named_arguments.empty())
        return std::string(pattern);

    size_t argument_current_index = 0;

    // Parse the format message pattern
    for (auto it = pattern.begin(); it < pattern.end(); ++it)
    {
        char current = *it;

        if (current == '{')
        {
            if (++it == pattern.end())
            {
                assert(false && "Invalid format message! Unmatched '}' in the format string.");
                return std::string(pattern);
            }

            char next = *it;

            // Match '{{' pattern
            if (next == '{')
            {
                result.push_back('{');
                continue;
            }

            // Match '}}' pattern
            if (next == '}')
            {
                if (++it != pattern.end())
                {
                    if (*it == '}')
                    {
                        result.push_back('}');
                        continue;
                    }
                }

                --it;
            }

            current = next;

            // Argument settings
            size_t argument_index = argument_current_index++;
            std::string argument_name;
            bool argument_width = false;
            size_t argument_width_index = 0;
            std::string argument_width_name;
            bool argument_precision = false;
            size_t argument_precision_index = 0;
            std::string argument_precision_name;
            fmt::v5::alignment align_type = fmt::v5::alignment::ALIGN_DEFAULT;
            char align_fill = ' ';
            int flags = 0;
            size_t width = 0;
            int precision = -1;
            char type = 0;

            // Parse argument index
            if (IsDigit(current))
            {
                if (!ParseUnsigned(it, pattern.end(), argument_index))
                    return std::string(pattern);
            }
            else if (IsStartName(current))
            {
                if (!ParseName(it, pattern.end(), argument_name))
                    return std::string(pattern);
            }

            current = *it;

            // Parse argument settings
            if ((current == ':') && (it != pattern.end()))
            {
                // Parse align settings
                int align_index = 0;
                if ((it + 1) != pattern.end())
                {
                    ++align_index;
                    ++it;
                }
                do
                {
                    auto align_ch = *(it + align_index);
                    switch (align_ch)
                    {
                        case '<':
                            align_type = fmt::v5::alignment::ALIGN_LEFT;
                            break;
                        case '>':
                            align_type = fmt::v5::alignment::ALIGN_RIGHT;
                            break;
                        case '=':
                            align_type = fmt::v5::alignment::ALIGN_NUMERIC;
                            break;
                        case '^':
                            align_type = fmt::v5::alignment::ALIGN_CENTER;
                            break;
                        default:
                            break;
                    }
                    if (align_type != fmt::v5::alignment::ALIGN_DEFAULT)
                    {
                        if (align_index > 0)
                        {
                            auto ch = *it;
                            if (ch == '{')
                            {
                                assert(false && "Invalid format message! Invalid fill character '{' in the format string.");
                                return std::string(pattern);
                            }
                            it += 2;
                            align_fill = ch;
                        }
                        else
                            ++it;
                        break;
                    }
                } while (align_index-- > 0);

                // Parse sign
                if (it != pattern.end())
                {
                    current = *it;
                    switch (current)
                    {
                        case '+':
                            flags |= fmt::v5::SIGN_FLAG | fmt::v5::PLUS_FLAG;
                            ++it;
                            break;
                        case '-':
                            flags |= fmt::v5::MINUS_FLAG;
                            ++it;
                            break;
                        case ' ':
                            flags |= fmt::v5::SIGN_FLAG;
                            ++it;
                            break;
                        default:
                            break;
                    }
                }

                // Parse hash
                if (it != pattern.end())
                {
                    current = *it;
                    if (current == '#')
                    {
                        flags |= fmt::v5::HASH_FLAG;
                        ++it;
                    }
                }

                // Parse zero flag
                if (it != pattern.end())
                {
                    current = *it;
                    if (current == '0')
                    {
                        align_type = fmt::v5::ALIGN_NUMERIC;
                        align_fill = '0';
                        ++it;
                    }
                }

                // Parse width
                if (it != pattern.end())
                {
                    current = *it;
                    if (IsDigit(current))
                    {
                        if (!ParseUnsigned(it, pattern.end(), width))
                            return std::string(pattern);
                    }
                    else if (current == '{')
                    {
                        if (++it == pattern.end())
                        {
                            assert(false && "Invalid format message! Invalid format width specifier.");
                            return std::string(pattern);
                        }

                        current = *it;

                        if (IsDigit(current))
                        {
                            if (!ParseUnsigned(it, pattern.end(), argument_width_index))
                                return std::string(pattern);
                            argument_width = true;
                        }
                        else if (IsStartName(current))
                        {
                            if (!ParseName(it, pattern.end(), argument_width_name))
                                return std::string(pattern);
                            argument_width = true;
                        }
                        if ((it == pattern.end()) || (*it++ != '}'))
                        {
                            assert(false && "Invalid format message! Invalid format width specifier.");
                            return std::string(pattern);
                        }
                        if (!argument_width)
                        {
                            argument_width_index = argument_current_index++;
                            argument_width = true;
                        }

                        // Parse width argument
                        if (!argument_width_name.empty())
                        {
                            auto it_width = args.named_arguments.find(argument_width_name);
                            if ((it_width == args.named_arguments.end()) || !it_width->second.GetUnsigned(buffer, width))
                            {
                                assert(false && "Invalid format message! Cannot find argument width specifier by name.");
                                return std::string(pattern);
                            }
                        }
                        else
                        {
                            if ((argument_width_index > args.arguments.size()) || !args.arguments[argument_width_index].GetUnsigned(buffer, width))
                            {
                                assert(false && "Invalid format message! Cannot find or parse argument width specifier.");
                                return std::string(pattern);
                            }
                        }
                    }
                }

                // Parse precision
                if (it != pattern.end())
                {
                    current = *it;
                    if (current == '.')
                    {
                        if (++it == pattern.end())
                        {
                            assert(false && "Invalid format message! Invalid format precision specifier.");
                            return std::string(pattern);
                        }

                        current = *it;

                        if (IsDigit(current))
                        {
                            size_t prec;
                            if (!ParseUnsigned(it, pattern.end(), prec))
                                return std::string(pattern);
                            precision = (int)prec;
                        }
                        else if (current == '{')
                        {
                            if (++it == pattern.end())
                            {
                                assert(false && "Invalid format message! Invalid format precision specifier.");
                                return std::string(pattern);
                            }

                            current = *it;

                            if (IsDigit(current))
                            {
                                if (!ParseUnsigned(it, pattern.end(), argument_precision_index))
                                    return std::string(pattern);
                                argument_precision = true;
                            }
                            else if (IsStartName(current))
                            {
                                if (!ParseName(it, pattern.end(), argument_precision_name))
                                    return std::string(pattern);
                                argument_precision = true;
                            }
                            if ((it == pattern.end()) || (*it++ != '}'))
                            {
                                assert(false && "Invalid format message! Invalid format precision specifier.");
                                return std::string(pattern);
                            }
                            if (!argument_precision)
                            {
                                argument_precision_index = argument_current_index++;
                                argument_precision = true;
                            }

                            // Parse precision argument
                            size_t prec = 0;
                            if (!argument_precision_name.empty())
                            {
                                auto it_prec = args.named_arguments.find(argument_precision_name);
                                if ((it_prec == args.named_arguments.end()) || !it_prec->second.GetUnsigned(buffer, prec))
                                {
                                    assert(false && "Invalid format message! Cannot find argument precision specifier by name.");
                                    return std::string(pattern);
                                }
                            }
                            else
                            {
                                if ((argument_precision_index > args.arguments.size()) || !args.arguments[argument_precision_index].GetUnsigned(buffer, prec))
                                {
                                    assert(false && "Invalid format message! Cannot find or parse argument precision specifier.");
                                    return std::string(pattern);
                                }
                            }
                            precision = (int)prec;
                        }
                    }
                }
            }

            // Parse type
            if ((it != pattern.end()) && (*it != '}'))
            {
                current = *it;
                type = current;
                ++it;
            }

            // Parse the end of argument
            if ((it == pattern.end()) || (*it != '}'))
            {
                assert(false && "Invalid format message! Unmatched '}' in the format string.");
                return std::string(pattern);
            }

            fmt::v5::format_specs specs;
            specs.align_ = align_type;
            specs.fill_ = align_fill;
            specs.flags = (uint_least8_t)flags;
            specs.width_ = (unsigned)width;
            specs.precision = precision;
            specs.type = type;

            fmt::v5::basic_writer<fmt::v5::back_insert_range<std::string>> writer(result);

            // Get the format argument
            Argument argument;
            if (!argument_name.empty())
            {
                auto it_arg = args.named_arguments.find(argument_name);
                if (it_arg == args.named_arguments.end())
                {
                    assert(false && "Invalid format message! Cannot find argument by name.");
                    return std::string(pattern);
                }
                argument = args.named_arguments[argument_name];
            }
            else
            {
                if (argument_index > args.arguments.size())
                {
                    assert(false && "Invalid format message! Cannot find argument by index.");
                    return std::string(pattern);
                }
                argument = args.arguments[argument_index];
            }

            // Write format argument
            if (!WriteFormatArgument(writer, specs, argument, buffer))
                return std::string(pattern);
        }
        else
            result.push_back(current);
    }

    return result;
}

} // namespace CppLogging
