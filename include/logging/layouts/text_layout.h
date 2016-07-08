/*!
    \file text_layout.h
    \brief Logging text layout definition
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_TEXT_LAYOUT_H
#define CPPLOGGING_TEXT_LAYOUT_H

#include "logging/layout.h"

#include <vector>

namespace CppLogging {

//! Logging text layout
/*!
    Text layout converts the given logging record into the text string
    using a given template. Text layout template is a string with a
    special placeholders provided inside curly brackets ("{}").

    Supported placeholders:
    - {DateTime}/{UtcDateTime} - converted to the local/UTC date & time (e.g. "1997-07-16T19:20:30.123+01:00"/"1997-07-16T19:20:30.123Z")
    - {Date}/{UtcDate} - converted to the local/UTC date (e.g. "1997-07-16")
    - {Time}/{UtcTime} - converted to the local/UTC time (e.g. "19:20:30.123+01:00"/"19:20:30.123Z")
    - {Year}/{UtcYear} - converted to the local/UTC four-digits year (e.g. "1997")
    - {Month}/{UtcMonth} - converted to the local/UTC two-digits month (e.g. "07")
    - {Day}/{UtcDay} - converted to the local/UTC two-digits day (e.g. "16")

    Not thread-safe.
*/
class BinaryLayout
{
public:
    BinaryLayout() : _buffer(1024) {}
    BinaryLayout(const BinaryLayout&) = delete;
    BinaryLayout(BinaryLayout&&) = default;
    ~BinaryLayout();

    BinaryLayout& operator=(const BinaryLayout&) = delete;
    BinaryLayout& operator=(BinaryLayout&&) = default;

    //! Layout the given logging record into a raw buffer
    /*!
         \param record - Logging record
         \return Raw buffer which contains logging record after the layout operation
    */
    std::pair<void*, size_t> LayoutRecord(const Record& record) override;

private:
    std::vector<char> _buffer;
};

} // namespace CppLogging

#endif // CPPLOGGING_BINARY_LAYOUT_H
