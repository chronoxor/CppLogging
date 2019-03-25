/*!
    \file record.cpp
    \brief Logging record implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/record.h"

namespace {

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

std::vector<Argument> ParseArguments(const std::vector<uint8_t>& buffer)
{
    std::vector<Argument> result;

    size_t index = 0;
    while (index < buffer.size())
    {
        // Parse the argument type 
        CppLogging::ArgumentType type;
        std::memcpy(&type, buffer.data() + index, sizeof(uint8_t));
        index += sizeof(uint8_t);

        // Parse the argument value
        switch (type)
        {
            case CppLogging::ArgumentType::ARG_BOOL:
            {
                result.emplace_back(Argument{ type, index, sizeof(uint8_t) });
                index += sizeof(uint8_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_CHAR:
            {
                result.emplace_back(Argument{ type, index, sizeof(uint8_t) });
                index += sizeof(uint8_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_WCHAR:
            {
                result.emplace_back(Argument{ type, index, sizeof(uint32_t) });
                index += sizeof(uint32_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_INT8:
            {
                result.emplace_back(Argument{ type, index, sizeof(int8_t) });
                index += sizeof(int8_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_UINT8:
            {
                result.emplace_back(Argument{ type, index, sizeof(uint8_t) });
                index += sizeof(uint8_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_INT16:
            {
                result.emplace_back(Argument{ type, index, sizeof(int16_t) });
                index += sizeof(int16_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_UINT16:
            {
                result.emplace_back(Argument{ type, index, sizeof(uint16_t) });
                index += sizeof(uint16_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_INT32:
            {
                result.emplace_back(Argument{ type, index, sizeof(int32_t) });
                index += sizeof(int32_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_UINT32:
            {
                result.emplace_back(Argument{ type, index, sizeof(uint32_t) });
                index += sizeof(uint32_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_INT64:
            {
                result.emplace_back(Argument{ type, index, sizeof(int64_t) });
                index += sizeof(int64_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_UINT64:
            {
                result.emplace_back(Argument{ type, index, sizeof(uint64_t) });
                index += sizeof(uint64_t);
                break;
            }
            case CppLogging::ArgumentType::ARG_FLOAT:
            {
                result.emplace_back(Argument{ type, index, sizeof(float) });
                index += sizeof(float);
                break;
            }
            case CppLogging::ArgumentType::ARG_DOUBLE:
            {
                result.emplace_back(Argument{ type, index, sizeof(double) });
                index += sizeof(double);
                break;
            }
            case CppLogging::ArgumentType::ARG_STRING:
            {
                uint32_t size;
                std::memcpy(&size, buffer.data() + index, sizeof(uint32_t));

                result.emplace_back(Argument{ type, index, sizeof(uint32_t) + size });
                index += sizeof(uint32_t) + size;
                break;
            }
            case CppLogging::ArgumentType::ARG_POINTER:
            {
                result.emplace_back(Argument{ type, index, sizeof(uint64_t) });
                index += sizeof(uint64_t);
                break;
            }
            default:
            {
                assert(false && "Unsupported argument type!");
                break;
            }
        }
    }

    return result;
}

bool ParseUnsigned(std::string::iterator& it, const std::string::iterator& end, size_t& result)
{
    result = 0;
    size_t max_int = std::numeric_limits<int>::max();
    size_t big_int = max_int / 10;
    char current = *it;
    while ((current >= '0') && (current <= '9'))
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

} // namespace

namespace CppLogging {

std::string Record::Deserialize()
{
    std::string result;

    size_t argument_current_index = 0;
    auto arguments = ParseArguments(buffer);

    // Parse the format message pattern
    for (auto it = message.begin(); it != message.end(); ++it)
    {
        char current = *it;

        if (current == '{')
        {
            if (++it == message.end())
            {
                assert(false && "Invalid format message! Unmatched '}' in the format string.");
                return message;
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
                if (++it != message.end())
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
            bool argument_width = false;
            size_t argument_width_index = 0;
            bool argument_precision = false;
            size_t argument_precision_index = 0;
            fmt::v5::alignment align_type = fmt::v5::alignment::ALIGN_DEFAULT;
            char align_fill = ' ';
            int flags = 0;
            size_t width = 0;
            int precision = -1;

            // Parse argument index
            if ((current >= '0') && (current <= '9'))
            {
                if (!ParseUnsigned(it, message.end(), argument_index))
                    return message;
            }

            current = *it;

            // Parse argument settings
            if ((current == ':') && (it != message.end()))
            {
                // Parse align settings
                int align_index = 0;
                if (it + 1 != message.end())
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
                                return message;
                            }
                            ++it;
                            align_fill = ch;
                        }
                        break;
                    }
                } while (align_index-- > 0);

                ++it;

                // Parse sign
                if (it != message.end())
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
                if (it != message.end())
                {
                    current = *it;
                    if (current == '#')
                    {
                        flags |= fmt::v5::HASH_FLAG;
                        ++it;
                    }
                }

                // Parse zero flag
                if (it != message.end())
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
                if (it != message.end())
                {
                    current = *it;
                    if ((current >= '0') && (current <= '9'))
                    {
                        if (!ParseUnsigned(it, message.end(), width))
                            return message;
                    }
                    else if (current == '{')
                    {
                        if (++it == message.end())
                        {
                            assert(false && "Invalid format message! Invalid format width specifier.");
                            return message;
                        }

                        current = *it;

                        if ((current >= '0') && (current <= '9'))
                        {
                            if (!ParseUnsigned(it, message.end(), argument_width_index))
                                return message;
                            argument_width = true;
                        }
                        if ((it == message.end()) || (*it++ != '}'))
                        {
                            assert(false && "Invalid format message! Invalid format width specifier.");
                            return message;
                        }
                        if (!argument_width)
                        {
                            argument_width_index = argument_current_index++;
                            argument_width = true;
                        }

                        // Parse width argument
                        if ((argument_width_index > arguments.size()) || !arguments[argument_width_index].GetUnsigned(buffer, width))
                        {
                            assert(false && "Invalid format message! Cannot parse argument width specifier.");
                            return message;
                        }
                    }
                }

                // Parse precision
                if (it != message.end())
                {
                    current = *it;
                    if (current == '.')
                    {
                        if (++it == message.end())
                        {
                            assert(false && "Invalid format message! Invalid format precision specifier.");
                            return message;
                        }

                        current = *it;

                        if ((current >= '0') && (current <= '9'))
                        {
                            size_t prec;
                            if (!ParseUnsigned(it, message.end(), prec))
                                return message;
                            precision = (int)prec;
                        }
                        else if (current == '{')
                        {
                            if (++it == message.end())
                            {
                                assert(false && "Invalid format message! Invalid format precision specifier.");
                                return message;
                            }

                            current = *it;

                            if ((current >= '0') && (current <= '9'))
                            {
                                if (!ParseUnsigned(it, message.end(), argument_precision_index))
                                    return message;
                                argument_precision = true;
                            }
                            if ((it == message.end()) || (*it++ != '}'))
                            {
                                assert(false && "Invalid format message! Invalid format precision specifier.");
                                return message;
                            }
                            if (!argument_precision)
                            {
                                argument_precision_index = argument_current_index++;
                                argument_precision = true;
                            }

                            // Parse precision argument
                            size_t prec;
                            if ((argument_precision_index > arguments.size()) || !arguments[argument_precision_index].GetUnsigned(buffer, prec))
                            {
                                assert(false && "Invalid format message! Cannot parse argument precision specifier.");
                                return message;
                            }
                            precision = (int)prec;
                        }
                    }
                }
            }

            if (it == message.end() || (*it != '}'))
            {
                assert(false && "Invalid format message! Unmatched '}' in the format string.");
                return message;
            }

            fmt::v5::format_specs specs;
            specs.align_ = align_type;
            specs.fill_ = align_fill;
            specs.flags = (uint_least8_t)flags;
            specs.width_ = (unsigned)width;
            specs.precision = precision;

            fmt::v5::basic_writer<fmt::v5::back_insert_range<std::string>> writer(result);

            auto& argument = arguments[argument_index];
            switch (argument.type)
            {
                case ArgumentType::ARG_BOOL:
                {
                    uint8_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint8_t));
                    writer.write(value != 0, specs);
                    break;
                }
                case ArgumentType::ARG_CHAR:
                {
                    uint8_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint8_t));
                    writer.write((char)value);
                    break;
                }
                case ArgumentType::ARG_WCHAR:
                {
                    uint32_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint32_t));
                    writer.write((char)value);
                    break;
                }
                case ArgumentType::ARG_INT8:
                {
                    int8_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(int8_t));
                    writer.write(value, specs);
                    break;
                }
                case ArgumentType::ARG_UINT8:
                {
                    uint8_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint8_t));
                    writer.write(value, specs);
                    break;
                }
                case ArgumentType::ARG_INT16:
                {
                    int16_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(int16_t));
                    writer.write(value, specs);
                    break;
                }
                case ArgumentType::ARG_UINT16:
                {
                    uint16_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint16_t));
                    writer.write(value, specs);
                    break;
                }
                case ArgumentType::ARG_INT32:
                {
                    int32_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(int32_t));
                    writer.write(value, specs);
                    break;
                }
                case ArgumentType::ARG_UINT32:
                {
                    uint32_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint32_t));
                    writer.write(value, specs);
                    break;
                }
                case ArgumentType::ARG_INT64:
                {
                    int64_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(int64_t));
                    writer.write(value, specs);
                    break;
                }
                case ArgumentType::ARG_UINT64:
                {
                    uint64_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint64_t));
                    writer.write(value, specs);
                    break;
                }
                case ArgumentType::ARG_STRING:
                {
                    uint32_t size;
                    std::memcpy(&size, buffer.data() + argument.offset, sizeof(uint32_t));

                    writer.write(buffer.data() + argument.offset + sizeof(uint32_t), size, specs);
                    break;
                }
                case ArgumentType::ARG_POINTER:
                {
                    uint64_t value;
                    std::memcpy(&value, buffer.data() + argument.offset, sizeof(uint64_t));
                    writer.write((void*)value);
                    break;
                }
                default:
                {
                    assert(false && "Unsupported argument type!");
                    return message;
                }
            }
        }
        else
            result.push_back(current);
    }

    return result;
}

bool Record::Validate()
{
    return true;
}

} // namespace CppLogging
