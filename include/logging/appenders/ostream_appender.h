/*!
    \file ostream_appender.h
    \brief Output stream (std::ostream) appender definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_APPENDERS_OSTREAM_APPENDER_H
#define CPPLOGGING_APPENDERS_OSTREAM_APPENDER_H

#include "logging/appender.h"

#include <iostream>

namespace CppLogging {

//! Output stream (std::ostream) appender
/*!
    Output stream (std::ostream) appender prints the given logging record
    into the given instance of std::ostream.

    Not thread-safe.
*/
class OstreamAppender : public Appender
{
public:
    //! Initialize the appender with a given output stream
    /*!
         \param stream - Output stream
    */
    explicit OstreamAppender(std::ostream& stream) : _ostream(stream) {}
    OstreamAppender(const OstreamAppender&) = delete;
    OstreamAppender(OstreamAppender&&) = delete;
    virtual ~OstreamAppender() = default;

    OstreamAppender& operator=(const OstreamAppender&) = delete;
    OstreamAppender& operator=(OstreamAppender&&) = delete;

    // Implementation of Appender
    void AppendRecord(Record& record) override;
    void Flush() override;

private:
    std::ostream& _ostream;
};

} // namespace CppLogging

#endif // CPPLOGGING_APPENDERS_OSTREAM_APPENDER_H
