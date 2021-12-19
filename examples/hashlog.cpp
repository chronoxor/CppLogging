/*!
    \file hashlog.cpp
    \brief Hash logger example
    \author Ivan Shynkarenka
    \date 19.12.2021
    \copyright MIT License
*/

#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create default logging sink processor with a hash layout
    auto sink = std::make_shared<CppLogging::Processor>(std::make_shared<CppLogging::HashLayout>());
    // Add file appender
    sink->appenders().push_back(std::make_shared<CppLogging::FileAppender>("file.hash.log"));

    // Configure example logger
    CppLogging::Config::ConfigLogger("example", sink);

    // Startup the logging infrastructure
    CppLogging::Config::Startup();
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
