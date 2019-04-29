/*!
    \file layout.h
    \brief Logging layout interface definition
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_LAYOUT_H
#define CPPLOGGING_LAYOUT_H

#include "logging/element.h"
#include "logging/record.h"

namespace CppLogging {

//! Logging layout interface
/*!
    Logging layout takes an instance of a single logging record
    and convert it into a raw buffer (raw filed will be updated).

    \see NullLayout
    \see EmptyLayout
    \see BinaryLayout
    \see TextLayout
*/
class Layout : public Element
{
public:
    //! Layout the given logging record into a raw buffer
    /*!
         This method will update the raw filed of the given logging record.

         \param record - Logging record
    */
    virtual void LayoutRecord(Record& record) = 0;
};

} // namespace CppLogging

#endif // CPPLOGGING_LAYOUT_H
