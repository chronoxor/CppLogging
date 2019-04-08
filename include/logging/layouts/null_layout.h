/*!
    \file null_layout.h
    \brief Null layout definition
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_LAYOUTS_NULL_LAYOUT_H
#define CPPLOGGING_LAYOUTS_NULL_LAYOUT_H

#include "logging/layout.h"

namespace CppLogging {

//! Null layout
/*!
    Null layout does nothing with a given logging record.

    Thread-safe.
*/
class NullLayout : public Layout
{
public:
    NullLayout() = default;
    NullLayout(const NullLayout&) = delete;
    NullLayout(NullLayout&&) = delete;
    virtual ~NullLayout() = default;

    NullLayout& operator=(const NullLayout&) = delete;
    NullLayout& operator=(NullLayout&&) = delete;

    // Implementation of Layout
    void LayoutRecord(Record& record) override {}
};

} // namespace CppLogging

#endif // CPPLOGGING_LAYOUTS_NULL_LAYOUT_H
