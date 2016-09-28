/*!
    \file file.cpp
    \brief File logger example
    \author Ivan Shynkarenka
    \date 29.07.2016
    \copyright MIT License
*/

#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create default logging sink processor
    auto sink = std::make_shared<CppLogging::Processor>();
    // Add binary layout
    sink->layouts().push_back(std::make_shared<CppLogging::BinaryLayout>());
    // Add file appender
    sink->appenders().push_back(std::make_shared<CppLogging::FileAppender>(CppCommon::File("file.bin.log")));

    // Configure example logger
    CppLogging::Config::ConfigLogger("example", sink);
}

int main(int argc, char** argv)
{
    // Configure logger
    ConfigureLogger();

    // Create example logger
    CppLogging::Logger logger("example");

    // Log some messages with different level
    logger.Debug("Debug message");
    logger.Info("Info message");
    logger.Warn("Warning message");
    logger.Error("Error message");
    logger.Fatal("Fatal message");

    return 0;
}
