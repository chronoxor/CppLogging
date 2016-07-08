/*!
    \file level.h
    \brief Logging level enumeration definition
    \author Ivan Shynkarenka
    \date 08.07.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_LEVEL_H
#define CPPLOGGING_LEVEL_H

#include <cstdint>

namespace CppLogging {

//! Logging level enumeration
enum class Level : uint8_t
{
    NONE  = 0x00,   //!< Log nothing
    FATAL = 0x1F,   //!< Log fatal errors
    ERROR = 0x3F,   //!< Log errors
    WARN  = 0x7F,   //!< Log warnings
    INFO  = 0x9F,   //!< Log information
    DEBUG = 0xBF,   //!< Log debug
    ALL   = 0xFF    //!< Log everything
};

} // namespace CppLogging

#endif // CPPLOGGING_LEVEL_H
