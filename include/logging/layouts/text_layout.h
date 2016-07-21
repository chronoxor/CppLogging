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

#include <memory>
#include <string>
#include <vector>

namespace CppLogging {

//! Logging text layout
/*!
    Text layout converts the given logging record into the text string
    using a given pattern. Text layout pattern is a string with a
    special placeholders provided inside curly brackets ("{}").

    Supported placeholders:
    - {DateTime}/{UtcDateTime} - converted to the local/UTC date & time (e.g. "1997-07-16T19:20:30.123+01:00"/"1997-07-16T19:20:30.123Z")
    - {Date}/{UtcDate} - converted to the local/UTC date (e.g. "1997-07-16")
    - {Time}/{UtcTime} - converted to the local/UTC time (e.g. "19:20:30.123+01:00"/"19:20:30.123Z")
    - {Year}/{UtcYear} - converted to the local/UTC four-digits year (e.g. "1997")
    - {Month}/{UtcMonth} - converted to the local/UTC two-digits month (e.g. "07")
    - {Day}/{UtcDay} - converted to the local/UTC two-digits day (e.g. "16")
    - {Hour}/{UtcHour} - converted to the local/UTC two-digits hour (e.g. "19")
    - {Minute}/{UtcMinute} - converted to the local/UTC two-digits minute (e.g. "20")
    - {Second}/{UtcSecond} - converted to the local/UTC two-digits second (e.g. "30")
    - {Milli}/{UtcMilli} - converted to the local/UTC three-digits millisecond (e.g. "123")
    - {Micro}/{UtcMicro} - converted to the local/UTC three-digits microsecond (e.g. "123")
    - {Nano}/{UtcNano} - converted to the local/UTC three-digits nanosecond (e.g. "789")
    - {Timezone}/{UtcTimezone} - converted to the local/UTC timezone suffix (e.g. "+01:00"/"Z")
    - {Thread} - converted to the thread Id (e.g. "0x0028F3D8")
    - {Level} - converted to the logging level
    - {Logger} - converted to the logger name
    - {Message} - converted to the log message
    - {EndLine} - converted to the end line suffix (e.g. Unix "\n" or Windows "\r\n")

    Not thread-safe.
*/
class TextLayout : public Layout
{
public:
    //! Initialize text layout with a given pattern
    /*!
         \param pattern - Layout pattern
    */
    TextLayout(const std::string& pattern = "{UtcDateTime} [{Thread}] {Level} {Logger} - {Message}{EndLine}");
    TextLayout(const TextLayout&) = delete;
    TextLayout(TextLayout&&) = default;
    ~TextLayout();

    TextLayout& operator=(const TextLayout&) = delete;
    TextLayout& operator=(TextLayout&&) = default;

    //! Layout the given logging record into a raw buffer
    /*!
         \param record - Logging record
         \return Raw buffer which contains logging record after the layout operation
    */
    std::pair<void*, size_t> LayoutRecord(const Record& record) override;

private:
    class Impl;
    std::unique_ptr<Impl> _pimpl;
};

} // namespace CppLogging

#endif // CPPLOGGING_TEXT_LAYOUT_H
