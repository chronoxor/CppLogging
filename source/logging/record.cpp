/*!
    \file record.cpp
    \brief Logging record implementation
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#include "logging/record.h"

namespace CppLogging {

void Record::Format()
{
    // Special check for empty format arguments list
    if (format.empty())
        return;

    // Format the message filed of the logging record
    fmt::ArgList args = fmt::ArgList::deserialize(format);
    message = fmt::format(message.c_str(), args);

    // Clear format buffer
    format.clear();
}

} // namespace CppLogging
