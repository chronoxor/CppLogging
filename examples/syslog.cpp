/*!
    \file syslog.cpp
    \brief Syslog logger example
    \author Ivan Shynkarenka
    \date 30.07.2016
    \copyright MIT License
*/

#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create default logging sink processor with a text layout
    auto sink = std::make_shared<CppLogging::Processor>(std::make_shared<CppLogging::TextLayout>());
    // Add syslog appender
    sink->appenders().push_back(std::make_shared<CppLogging::SyslogAppender>());

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
