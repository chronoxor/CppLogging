/*!
    \file version.h
    \brief Version definition
    \author Ivan Shynkarenka
    \date 26.05.2016
    \copyright MIT License
*/

#ifndef CPPLOGGING_VERSION_H
#define CPPLOGGING_VERSION_H

/*! \mainpage C++ Logging Library

C++ Logging Library provides functionality to log different events with a high
throughput in multi-thread environment into different sinks (console, files,
rolling files, syslog, etc.). Logging configuration is very flexible and gives
functionality to build flexible logger hierarchy with combination of logging
processors (sync, async), filters, layouts (binary, text) and appenders.

This document contains CppLogging API references.

Library description, features, requirements and usage examples can be  find  on
GitHub: https://github.com/chronoxor/CppLogging

*/

/*!
    \namespace CppLogging
    \brief C++ Logging project definitions
*/
namespace CppLogging {

//! Project version
const char version[] = "1.0.0.0";

} // namespace CppLogging

#endif // CPPLOGGING_VERSION_H
