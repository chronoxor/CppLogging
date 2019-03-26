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
    // Create default logging sink processor with a binary layout
    auto sink = std::make_shared<CppLogging::Processor>(std::make_shared<CppLogging::BinaryLayout>());
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
    logger.Debug("Debug message {}", 1);
    logger.Info("Info message {}", 2);
    logger.Warn("Warning message {}", 3);
    logger.Error("Error message {}", 4);
    logger.Fatal("Fatal message {}", 5);

    return 0;
}
