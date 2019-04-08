/*!
    \file empty_layout.h
    \brief Empty layout definition
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_LAYOUTS_EMPTY_LAYOUT_H
#define CPPLOGGING_LAYOUTS_EMPTY_LAYOUT_H

#include "logging/layout.h"

namespace CppLogging {

//! Empty layout
/*!
    Empty layout performs zero memory operation to convert
    the given logging record into the empty raw buffer.

    Thread-safe.
*/
class EmptyLayout : public Layout
{
public:
    EmptyLayout() = default;
    EmptyLayout(const EmptyLayout&) = delete;
    EmptyLayout(EmptyLayout&&) = delete;
    virtual ~EmptyLayout() = default;

    EmptyLayout& operator=(const EmptyLayout&) = delete;
    EmptyLayout& operator=(EmptyLayout&&) = delete;

    // Implementation of Layout
    void LayoutRecord(Record& record) override { record.raw.clear(); }
};

} // namespace CppLogging

#endif // CPPLOGGING_LAYOUTS_EMPTY_LAYOUT_H
