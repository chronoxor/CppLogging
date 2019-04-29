/*!
    \file filter.h
    \brief Logging filter interface definition
    \author Ivan Shynkarenka
    \date 26.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_FILTER_H
#define CPPLOGGING_FILTER_H

#include "logging/element.h"
#include "logging/record.h"

namespace CppLogging {

//! Logging filter interface
/*!
    Logging filter takes an instance of a single logging record and
    performs some checks to detect if the record should be filered
    out and not processed anymore.

    \see SwitchFilter
    \see LoggerFilter
    \see LevelFilter
    \see MessageFilter
*/
class Filter : public Element
{
public:
    //! Filter the given logging record
    /*!
         \param record - Logging record
         \return 'true' if the logging record should be processed, 'false' if the logging record was filtered out
    */
    virtual bool FilterRecord(Record& record) = 0;
};

} // namespace CppLogging

#endif // CPPLOGGING_FILTER_H
