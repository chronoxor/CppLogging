/*!
    \file record.cpp
    \brief Logging record implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/record.h"

namespace {

enum AlignType
{
    ALIGN_DEFAULT,
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER,
    ALIGN_NUMERIC
};

bool ParseNonnegativeInt(std::string::iterator& it, const std::string::iterator& end, size_t& result)
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
            AlignType align_type = AlignType::ALIGN_DEFAULT;
            char align_fill = ' ';
            size_t align_width = 0;

            // Parse argument index
            if ((current >= '0') && (current <= '9'))
            {
                if (!ParseNonnegativeInt(it, message.end(), argument_index))
                    return message;
                current = *it;
            }

            // Parse argument settings
            if (current == ':')
            {
                // Parse align settings
                int align_index = (it + 1 == message.end()) ? 0 : 1;
                do
                {
                    auto align_ch = *(it + align_index);
                    switch (align_ch)
                    {
                        case '<':
                            align_type = AlignType::ALIGN_LEFT;
                            break;
                        case '>':
                            align_type = AlignType::ALIGN_RIGHT;
                            break;
                        case '=':
                            align_type = AlignType::ALIGN_NUMERIC;
                            break;
                        case '^':
                            align_type = AlignType::ALIGN_CENTER;
                            break;
                    }
                    if (align_type != ALIGN_DEFAULT) 
                    {
                        if (align_index > 0) 
                        {
                            auto ch = *it;
                            if (ch == '{')
                            {
                                assert(false && "Invalid format message! Invalid fill character '{' in the format string.");
                                return message;
                            }
                            it += 2;
                            align_fill = ch;
                        }
                        else
                            ++it;
                        break;
                    }
                } while (align_index-- > 0);
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
