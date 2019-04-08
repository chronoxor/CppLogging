/*!
    \file binary_layout.h
    \brief Binary layout definition
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_LAYOUTS_BINARY_LAYOUT_H
#define CPPLOGGING_LAYOUTS_BINARY_LAYOUT_H

#include "logging/layout.h"

namespace CppLogging {

//! Binary layout
/*!
    Binary layout performs simple memory copy operation to convert
    the given logging record into the plane raw buffer.

    Thread-safe.
*/
class BinaryLayout : public Layout
{
public:
    BinaryLayout() = default;
    BinaryLayout(const BinaryLayout&) = delete;
    BinaryLayout(BinaryLayout&&) = delete;
    virtual ~BinaryLayout() = default;

    BinaryLayout& operator=(const BinaryLayout&) = delete;
    BinaryLayout& operator=(BinaryLayout&&) = delete;

    // Implementation of Layout
    void LayoutRecord(Record& record) override;
};

} // namespace CppLogging

#endif // CPPLOGGING_LAYOUTS_BINARY_LAYOUT_H
