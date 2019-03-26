/*!
    \file sync.cpp
    \brief Synchronous logger processor example
    \author Ivan Shynkarenka
    \date 15.09.2016
    \copyright MIT License
*/

#include "logging/config.h"
#include "logging/logger.h"

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

void ConfigureLogger()
{
    // Create default logging sink processor with a binary layout
    auto sink = std::make_shared<CppLogging::SyncProcessor>(std::make_shared<CppLogging::BinaryLayout>());
    // Add file appender with size-based rolling policy and archivation
    sink->appenders().push_back(std::make_shared<CppLogging::RollingFileAppender>(".", "rolling", "bin.log", 4096, 9, true));

    // Configure example logger
    CppLogging::Config::ConfigLogger("example", sink);
}

int main(int argc, char** argv)
{
    // Configure logger
    ConfigureLogger();

    std::cout << "Press Enter to stop..." << std::endl;

    int concurrency = 4;

    // Start some threads
    std::atomic<bool> stop(false);
    std::vector<std::thread> threads;
    for (int thread = 0; thread < concurrency; ++thread)
    {
        threads.push_back(std::thread([&stop]()
        {
            // Create example logger
            CppLogging::Logger logger("example");

            int index = 0;

            while (!stop)
            {
                ++index;

                // Log some messages with different level
                logger.Debug("Debug message {}", index);
                logger.Info("Info message {}", index);
                logger.Warn("Warning message {}", index);
                logger.Error("Error message {}", index);
                logger.Fatal("Fatal message {}", index);

                // Yield for a while...
                CppCommon::Thread::Yield();
            }
        }));
    }

    // Wait for input
    std::cin.get();

    // Stop threads
    stop = true;

    // Wait for all threads
    for (auto& thread : threads)
        thread.join();

    return 0;
}
