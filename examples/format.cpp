/*!
    \file format.cpp
    \brief Format logger example
    \author Ivan Shynkarenka
    \date 18.09.2016
    \copyright MIT License
*/

#include "logging/logger.h"

int main(int argc, char** argv)
{
    // Create default logger
    CppLogging::Logger logger;

    // Log some messages with format
    logger.Info("{:+f}; {:+f}", 3.14, -3.14);
    logger.Info("{: f}; {: f}", 3.14, -3.14);
    logger.Info("{:-f}; {:-f}", 3.14, -3.14);
    logger.Info("int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    logger.Info("int: {0:d};  hex: {0:#x};  oct: {0:#o};  bin: {0:#b}", 42);

    return 0;
}
