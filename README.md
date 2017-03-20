# CppLogging

[![Linux build status](https://img.shields.io/travis/chronoxor/CppLogging/master.svg?label=Linux)](https://travis-ci.org/chronoxor/CppLogging)
[![OSX build status](https://img.shields.io/travis/chronoxor/CppLogging/master.svg?label=OSX)](https://travis-ci.org/chronoxor/CppLogging)
[![Cygwin build status](https://img.shields.io/appveyor/ci/chronoxor/CppLogging/master.svg?label=Cygwin)](https://ci.appveyor.com/project/chronoxor/CppLogging)
[![MinGW build status](https://img.shields.io/appveyor/ci/chronoxor/CppLogging/master.svg?label=MinGW)](https://ci.appveyor.com/project/chronoxor/CppLogging)
[![Windows build status](https://img.shields.io/appveyor/ci/chronoxor/CppLogging/master.svg?label=Windows)](https://ci.appveyor.com/project/chronoxor/CppLogging)

C++ Logging Library provides functionality to log different events with a high
throughput in multi-thread environment into different sinks (console, files,
rolling files, syslog, etc.). Logging configuration is very flexible and gives
functionality to build flexible logger hierarchy with combination of logging
processors (sync, async), filters, layouts (binary, text) and appenders.

[CppLogging API reference](http://chronoxor.github.io/CppLogging/index.html)

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
    * [Clone repository with submodules](#clone-repository-with-submodules)
    * [Linux](#linux)
    * [OSX](#osx)
    * [Windows (Cygwin)](#windows-cygwin)
    * [Windows (MinGW)](#windows-mingw)
    * [Windows (MinGW with MSYS)](#windows-mingw-with-msys)
    * [Windows (Visaul Studio 2015)](#windows-visaul-studio-2015)
  * [Logging examples](#logging-examples)
    * [Example 1: Default logger](#example-1-default-logger)
    * [Example 2: Format with logger](#example-2-format-with-logger)
    * [Example 3: Configure custom logger with text layout and console appender](#example-3-configure-custom-logger-with-text-layout-and-console-appender)
    * [Example 4: Configure custom logger with text layout and syslog appender](#example-4-configure-custom-logger-with-text-layout-and-syslog-appender)
    * [Example 5: Configure custom logger with binary layout and file appender](#example-5-configure-custom-logger-with-binary-layout-and-file-appender)
    * [Example 6: Configure logger with custom text layout pattern](#example-6-configure-logger-with-custom-text-layout-pattern)
    * [Example 7: Configure rolling file appender with time-based policy](#example-7-configure-rolling-file-appender-with-time-based-policy)
    * [Example 8: Configure rolling file appender with size-based policy](#example-8-configure-rolling-file-appender-with-size-based-policy)
    * [Example 9: Multi-thread logging with synchronous processor](#example-9-multi-thread-logging-with-synchronous-processor)
    * [Example 10: Multi-thread logging with asynchronous processor](#example-10-multi-thread-logging-with-asynchronous-processor)
  * [Logging benchmarks](#logging-benchmarks)
    * [Benchmark 1: Null appender](#benchmark-1-null-appender)
    * [Benchmark 2: File appender](#benchmark-2-file-appender)
    * [Benchmark 3: Synchronous processor with null appender](#benchmark-3-synchronous-processor-with-null-appender)
    * [Benchmark 4: Asynchronous processor with null appender](#benchmark-4-asynchronous-processor-with-null-appender)
    * [Benchmark 5: Synchronous processor with file appender](#benchmark-5-synchronous-processor-with-file-appender)
    * [Benchmark 6: Asynchronous processor with file appender](#benchmark-6-asynchronous-processor-with-file-appender)
    * [Benchmark 7: Format in logging thread vs format in background thread](#benchmark-7-format-in-logging-thread-vs-format-in-background-thread)
  * [Tools](#tools)
    * [Binary log reader](#binary-log-reader)

# Features
* Cross platform (Linux, OSX, Windows)
* Optimized for performance
* Binary & text layouts
* Synchronous logging
* Asynchronous logging
* Flexible configuration and logger processing hierarchy
* Appenders collection (null, memory, console, file, rolling file, ostream, syslog)
* Logging levels (debug, info, warning, error, fatal)
* Logging filters (by level, by logger name, by message pattern)
* Format logging records using [{fmt} library](http://fmtlib.net)
* Log files rolling policies (time-based, size-bases)
* Log files Zip archivation

# Requirements
* Linux
* OSX
* Windows 7 / Windows 10
* [CMake](http://www.cmake.org)
* [GIT](https://git-scm.com)
* [GCC](https://gcc.gnu.org)

Optional:
* [Clang](http://clang.llvm.org)
* [Clion](https://www.jetbrains.com/clion)
* [MinGW](http://mingw-w64.org/doku.php)
* [Visual Studio 2015](https://www.visualstudio.com)

# How to build?

## Clone repository with submodules
```
git clone https://github.com/chronoxor/CppLogging.git CppLogging
cd CppLogging
git submodule update --init --recursive --remote
```

## Linux
```
cd build
./unix.sh
```

## OSX
```
cd build
./unix.sh
```

## Windows (Cygwin)
```
cd build
cygwin.bat
```

## Windows (MinGW)
```
cd build
mingw.bat
```

## Windows (Visaul Studio 2015)
```
cd build
vs.bat
```

# Logging examples

## Example 1: Default logger
This is the simple example of using default logger. Just link it with
CppLogging library and you'll get default logger functionality with
text layout and console appender:

```C++
#include "logging/logger.h"

int main(int argc, char** argv)
{
    // Create default logger
    CppLogging::Logger logger;

    // Log some messages with different level
    logger.Debug("Debug message");
    logger.Info("Info message");
    logger.Warn("Warning message");
    logger.Error("Error message");
    logger.Fatal("Fatal message");

    return 0;
}
```

Example will create the following log in console:
![Default report][default]
[default]: https://github.com/chronoxor/CppLogging/raw/master/images/default.png "Default report"

## Example 2: Format with logger
CppLogging library provides powerful logging format API based on the
[{fmt} library](http://fmtlib.net):

```C++
#include "logging/logger.h"

class Date
{
public:
    Date(int year, int month, int day) : _year(year), _month(month), _day(day) {}

    friend std::ostream& operator<<(std::ostream& os, const Date& date)
    { return os << date._year << '-' << date._month << '-' << date._day; }

private:
    int _year, _month, _day;
};

int main(int argc, char** argv)
{
    // Create default logger
    CppLogging::Logger logger;

    // Log some messages with format
    logger.Info("argc: {}, argv: {}", argc, (void*)argv);
    logger.Info("no arguments");
    logger.Info("{0}, {1}, {2}", -1, 0, 1);
    logger.Info("{0}, {1}, {2}", 'a', 'b', 'c');
    logger.Info("{}, {}, {}", 'a', 'b', 'c');
    logger.Info("{2}, {1}, {0}", 'a', 'b', 'c');
    logger.Info("{0}{1}{0}", "abra", "cad");
    logger.Info("{:<30}", "left aligned");
    logger.Info("{:>30}", "right aligned");
    logger.Info("{:^30}", "centered");
    logger.Info("{:*^30}", "centered");
    logger.Info("{:+f}; {:+f}", 3.14, -3.14);
    logger.Info("{: f}; {: f}", 3.14, -3.14);
    logger.Info("{:-f}; {:-f}", 3.14, -3.14);
    logger.Info("int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    logger.Info("int: {0:d};  hex: {0:#x};  oct: {0:#o};  bin: {0:#b}", 42);
    logger.Info("The date is {}", Date(2012, 12, 9));
    logger.Info("Elapsed time: {s:.2f} seconds", "s"_a = 1.23);
    logger.Info("The answer is {}"_format(42).c_str());

    return 0;
}
```

Example will create the following log:
```
2016-09-28T13:59:19.970Z [0x000083C8] INFO   - argc: 1, argv: 0x1e5a2aa6b0
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - no arguments
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - -1, 0, 1
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - a, b, c
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - a, b, c
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - c, b, a
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - abracadabra
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - left aligned
2016-09-28T13:59:19.971Z [0x000083C8] INFO   -                  right aligned
2016-09-28T13:59:19.971Z [0x000083C8] INFO   -            centered
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - ***********centered***********
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - +3.140000; -3.140000
2016-09-28T13:59:19.971Z [0x000083C8] INFO   -  3.140000; -3.140000
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - 3.140000; -3.140000
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - int: 42;  hex: 2a;  oct: 52; bin: 101010
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - int: 42;  hex: 0x2a;  oct: 052;  bin: 0b101010
2016-09-28T13:59:19.971Z [0x000083C8] INFO   - The date is 2012-12-9
2016-09-28T13:59:19.972Z [0x000083C8] INFO   - Elapsed time: 1.23 seconds
2016-09-28T13:59:19.972Z [0x000083C8] INFO   - The answer is 42
```

## Example 3: Configure custom logger with text layout and console appender
This example shows how to configure a custom logger with a given name to
perform logging with a text layout and console appender sink:

```C++
#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create default logging sink processor
    auto sink = std::make_shared<CppLogging::Processor>();
    // Add text layout
    sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
    // Add console appender
    sink->appenders().push_back(std::make_shared<CppLogging::ConsoleAppender>());

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
```

## Example 4: Configure custom logger with text layout and syslog appender
*Syslog appender is available only in Unix platforms and does nothing in
Windows!*

This example shows how to configure a custom logger with a given name to
perform logging with a text layout and syslog appender sink:

```C++
#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create default logging sink processor
    auto sink = std::make_shared<CppLogging::Processor>();
    // Add text layout
    sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
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
```

## Example 5: Configure custom logger with binary layout and file appender
This example shows how to configure a custom logger with a given name to
perform logging with a binary layout and file appender sink:

```C++
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
```

## Example 6: Configure logger with custom text layout pattern
Text layout message is flexible to customize with layout pattern. Text layout
pattern is a string with a special placeholders provided inside curly brackets
("{}").

Supported placeholders:
- **{UtcDateTime} / {LocalDateTime}** - converted to the UTC/local date & time (e.g. "1997-07-16T19:20:30.123Z" / "1997-07-16T19:20:30.123+01:00")
- **{UtcDate} / {LocalDate}** - converted to the UTC/local date (e.g. "1997-07-16")
- **{Time} / {LocalTime}** - converted to the UTC/local time (e.g. "19:20:30.123Z" / "19:20:30.123+01:00")
- **{UtcYear} / {LocalYear}** - converted to the UTC/local four-digits year (e.g. "1997")
- **{UtcMonth} / {LocalMonth}** - converted to the UTC/local two-digits month (e.g. "07")
- **{UtcDay} / {LocalDay}** - converted to the UTC/local two-digits day (e.g. "16")
- **{UtcHour} / {LocalHour}** - converted to the UTC/local two-digits hour (e.g. "19")
- **{UtcMinute} / {LocalMinute}** - converted to the UTC/local two-digits minute (e.g. "20")
- **{UtcSecond} / {LocalSecond}** - converted to the UTC/local two-digits second (e.g. "30")
- **{UtcTimezone} / {LocalTimezone}** - converted to the UTC/local timezone suffix (e.g. "Z" / "+01:00")
- **{Millisecond}** - converted to the three-digits millisecond (e.g. "123")
- **{Microsecond}** - converted to the three-digits microsecond (e.g. "123")
- **{Nanosecond}** - converted to the three-digits nanosecond (e.g. "789")
- **{Thread}** - converted to the thread Id (e.g. "0x0028F3D8")
- **{Level}** - converted to the logging level
- **{Logger}** - converted to the logger name
- **{Message}** - converted to the log message
- **{EndLine}** - converted to the end line suffix (e.g. Unix "\n" or Windows "\r\n")

```C++
#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create a custom text layout pattern
    std::string pattern = "{UtcYear}-{UtcMonth}-{UtcDay}T{UtcHour}:{UtcMinute}:{UtcSecond}.{Millisecond}{UtcTimezone} - {Microsecond}.{Nanosecond} - [{Thread}] - {Level} - {Logger} - {Message} - {EndLine}";

    // Create default logging sink processor
    auto sink = std::make_shared<CppLogging::Processor>();
    // Add text layout
    sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>(pattern));
    // Add console appender
    sink->appenders().push_back(std::make_shared<CppLogging::ConsoleAppender>());

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
```

## Example 7: Configure rolling file appender with time-based policy
Time-based rolling policy will create a new logging file to write into using
a special pattern (contains date & time placeholders).

Time-based policy composes logging filename from the given pattern using the
following placeholders:
- **{UtcDateTime} / {LocalDateTime}** - converted to the UTC/local date & time (e.g. "1997-07-16T192030Z" / "1997-07-16T192030+0100")
- **{UtcDate} / {LocalDate}** - converted to the UTC/local date (e.g. "1997-07-16")
- **{Time} / {LocalTime}** - converted to the UTC/local time (e.g. "192030Z" / "192030+0100")
- **{UtcYear} / {LocalYear}** - converted to the UTC/local four-digits year (e.g. "1997")
- **{UtcMonth} / {LocalMonth}** - converted to the UTC/local two-digits month (e.g. "07")
- **{UtcDay} / {LocalDay}** - converted to the UTC/local two-digits day (e.g. "16")
- **{UtcHour} / {LocalHour}** - converted to the UTC/local two-digits hour (e.g. "19")
- **{UtcMinute} / {LocalMinute}** - converted to the UTC/local two-digits minute (e.g. "20")
- **{UtcSecond} / {LocalSecond}** - converted to the UTC/local two-digits second (e.g. "30")
- **{UtcTimezone} / {LocalTimezone}** - converted to the UTC/local timezone suffix (e.g. "Z" / "+0100")

```C++
#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create default logging sink processor
    auto sink = std::make_shared<CppLogging::Processor>();
    // Add text layout
    sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
    // Add rolling file appender which rolls each second and create log file with a pattern "{UtcDateTime}.log"
    sink->appenders().push_back(std::make_shared<CppLogging::FileAppender>(CppCommon::RollingFileAppender(".", TimeRollingPolicy::SECOND, "{UtcDateTime}.log", true)));

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
```

## Example 8: Configure rolling file appender with size-based policy
Size-based rolling policy will create a new logging file to write when the
current file size exceeded size limit. Logging backups are indexed and its
count could be limited as well.

Size-based policy for 5 backups works in a following way:
```
example.log   -> example.1.log
example.1.log -> example.2.log
example.2.log -> example.3.log
example.3.log -> example.4.log
example.4.log -> example.5.log
example.5.log -> remove
```

```C++
#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create default logging sink processor
    auto sink = std::make_shared<CppLogging::Processor>();
    // Add binary layout
    sink->layouts().push_back(std::make_shared<CppLogging::BinaryLayout>());
    // Add rolling file appender which rolls after append 4kb of logs and will keep only 5 recent archives
    sink->appenders().push_back(std::make_shared<CppLogging::FileAppender>(CppCommon::RollingFileAppender(".", "file", "bin.log", 4096, 5, true)));

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
```

## Example 9: Multi-thread logging with synchronous processor
Synchronous processor uses critical-section lock to avoid multiple
threads from logging at the same time (logging threads are waiting
until critical-section is released).

This example shows how to configure a custom logger with a given name to
use synchronous processor in multi-thread environment:

```C++
#include "logging/config.h"
#include "logging/logger.h"

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

void ConfigureLogger()
{
    // Create default logging sink processor
    auto sink = std::make_shared<CppLogging::SyncProcessor>();
    // Add binary layout
    sink->layouts().push_back(std::make_shared<CppLogging::BinaryLayout>());
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
        threads.push_back(std::thread([&stop, thread]()
        {
            // Create example logger
            CppLogging::Logger logger("example");

            while (!stop)
            {
                // Log some messages with different level
                logger.Debug("Debug message");
                logger.Info("Info message");
                logger.Warn("Warning message");
                logger.Error("Error message");
                logger.Fatal("Fatal message");

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
```

## Example 10: Multi-thread logging with asynchronous processor
Asynchronous processor uses lock-free queue to collect logging records from
multiple threads at the same time.

This example shows much better performance with less threads contentions in
comparison with the previous one for lots of threads:

```C++
#include "logging/config.h"
#include "logging/logger.h"

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

void ConfigureLogger()
{
    // Create default logging sink processor
    auto sink = std::make_shared<CppLogging::AsyncProcessor>();
    // Add text layout
    sink->layouts().push_back(std::make_shared<CppLogging::TextLayout>());
    // Add file appender with time-based rolling policy and archivation
    sink->appenders().push_back(std::make_shared<CppLogging::RollingFileAppender>(".", CppLogging::TimeRollingPolicy::SECOND, "{UtcDateTime}.log", true));

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
        threads.push_back(std::thread([&stop, thread]()
        {
            // Create example logger
            CppLogging::Logger logger("example");

            while (!stop)
            {
                // Log some messages with different level
                logger.Debug("Debug message");
                logger.Info("Info message");
                logger.Warn("Warning message");
                logger.Error("Error message");
                logger.Fatal("Fatal message");

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
```

# Logging benchmarks

## Benchmark 1: Null appender
Benchmark source file: [appender_null.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/appender_null.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 21.688 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Sep 28 22:08:14 2016
UTC timestamp: Wed Sep 28 19:08:14 2016
===============================================================================
Benchmark: NullAppender-binary
Attempts: 5
Iterations: 10000000
-------------------------------------------------------------------------------
Phase: NullAppender-binary
Average time: 62 ns / iteration
Minimal time: 62 ns / iteration
Maximal time: 63 ns / iteration
Total time: 624.974 ms
Total iterations: 10000000
Iterations throughput: 16000657 / second
===============================================================================
Benchmark: NullAppender-text
Attempts: 5
Iterations: 10000000
-------------------------------------------------------------------------------
Phase: NullAppender-text
Average time: 174 ns / iteration
Minimal time: 174 ns / iteration
Maximal time: 176 ns / iteration
Total time: 1.744 s
Total iterations: 10000000
Iterations throughput: 5733126 / second
===============================================================================
```

## Benchmark 2: File appender
Benchmark source file: [appender_file.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/appender_file.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 21.181 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Sep 28 22:12:24 2016
UTC timestamp: Wed Sep 28 19:12:24 2016
===============================================================================
Benchmark: FileAppender-binary
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: FileAppender-binary
Average time: 106 ns / iteration
Minimal time: 106 ns / iteration
Maximal time: 117 ns / iteration
Total time: 106.598 ms
Total iterations: 1000000
Iterations throughput: 9381018 / second
===============================================================================
Benchmark: FileAppender-text
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: FileAppender-text
Average time: 229 ns / iteration
Minimal time: 229 ns / iteration
Maximal time: 249 ns / iteration
Total time: 229.126 ms
Total iterations: 1000000
Iterations throughput: 4364404 / second
===============================================================================
```

## Benchmark 3: Synchronous processor with null appender
Benchmark source file: [processor_sync.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/processor_sync.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 21.628 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Sep 28 22:16:57 2016
UTC timestamp: Wed Sep 28 19:16:57 2016
===============================================================================
Benchmark: SyncProcessor-null
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:1)
Average time: 27 ns / iteration
Minimal time: 27 ns / iteration
Maximal time: 30 ns / iteration
Total time: 55.891 ms
Total iterations: 1999999
Iterations throughput: 35783702 / second
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:2)
Average time: 22 ns / iteration
Minimal time: 22 ns / iteration
Maximal time: 29 ns / iteration
Total time: 89.344 ms
Total iterations: 3999999
Iterations throughput: 44770342 / second
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:4)
Average time: 84 ns / iteration
Minimal time: 84 ns / iteration
Maximal time: 92 ns / iteration
Total time: 674.796 ms
Total iterations: 7999999
Iterations throughput: 11855429 / second
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:8)
Average time: 113 ns / iteration
Minimal time: 113 ns / iteration
Maximal time: 113 ns / iteration
Total time: 1.808 s
Total iterations: 15999999
Iterations throughput: 8849109 / second
===============================================================================
Benchmark: SyncProcessor-binary
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:1)
Average time: 74 ns / iteration
Minimal time: 74 ns / iteration
Maximal time: 83 ns / iteration
Total time: 74.038 ms
Total iterations: 1000000
Iterations throughput: 13506405 / second
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:2)
Average time: 123 ns / iteration
Minimal time: 123 ns / iteration
Maximal time: 133 ns / iteration
Total time: 246.187 ms
Total iterations: 2000000
Iterations throughput: 8123893 / second
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:4)
Average time: 205 ns / iteration
Minimal time: 205 ns / iteration
Maximal time: 210 ns / iteration
Total time: 821.948 ms
Total iterations: 4000000
Iterations throughput: 4866485 / second
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:8)
Average time: 276 ns / iteration
Minimal time: 276 ns / iteration
Maximal time: 278 ns / iteration
Total time: 2.213 s
Total iterations: 8000000
Iterations throughput: 3614274 / second
===============================================================================
Benchmark: SyncProcessor-text
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:1)
Average time: 177 ns / iteration
Minimal time: 177 ns / iteration
Maximal time: 180 ns / iteration
Total time: 177.173 ms
Total iterations: 1000000
Iterations throughput: 5644200 / second
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:2)
Average time: 222 ns / iteration
Minimal time: 222 ns / iteration
Maximal time: 241 ns / iteration
Total time: 444.429 ms
Total iterations: 2000000
Iterations throughput: 4500155 / second
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:4)
Average time: 308 ns / iteration
Minimal time: 308 ns / iteration
Maximal time: 315 ns / iteration
Total time: 1.235 s
Total iterations: 4000000
Iterations throughput: 3237170 / second
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:8)
Average time: 405 ns / iteration
Minimal time: 405 ns / iteration
Maximal time: 409 ns / iteration
Total time: 3.242 s
Total iterations: 8000000
Iterations throughput: 2467338 / second
===============================================================================
```

## Benchmark 4: Asynchronous processor with null appender
Benchmark source file: [processor_async.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/processor_async.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 21.576 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Sep 28 22:21:13 2016
UTC timestamp: Wed Sep 28 19:21:13 2016
===============================================================================
Benchmark: AsyncProcessor-null
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: AsyncProcessor-null(threads:1)
Average time: 71 ns / iteration
Minimal time: 71 ns / iteration
Maximal time: 79 ns / iteration
Total time: 143.543 ms
Total iterations: 1999999
Iterations throughput: 13933096 / second
-------------------------------------------------------------------------------
Phase: AsyncProcessor-null(threads:2)
Average time: 47 ns / iteration
Minimal time: 47 ns / iteration
Maximal time: 52 ns / iteration
Total time: 191.668 ms
Total iterations: 3999999
Iterations throughput: 20869336 / second
-------------------------------------------------------------------------------
Phase: AsyncProcessor-null(threads:4)
Average time: 30 ns / iteration
Minimal time: 30 ns / iteration
Maximal time: 32 ns / iteration
Total time: 244.934 ms
Total iterations: 7999999
Iterations throughput: 32661750 / second
-------------------------------------------------------------------------------
Phase: AsyncProcessor-null(threads:8)
Average time: 44 ns / iteration
Minimal time: 44 ns / iteration
Maximal time: 44 ns / iteration
Total time: 704.504 ms
Total iterations: 15999999
Iterations throughput: 22710985 / second
===============================================================================
Benchmark: AsyncProcessor-binary
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: AsyncProcessor-binary(threads:1)
Average time: 130 ns / iteration
Minimal time: 130 ns / iteration
Maximal time: 148 ns / iteration
Total time: 130.037 ms
Total iterations: 1000000
Iterations throughput: 7690098 / second
-------------------------------------------------------------------------------
Phase: AsyncProcessor-binary(threads:2)
Average time: 86 ns / iteration
Minimal time: 86 ns / iteration
Maximal time: 100 ns / iteration
Total time: 172.245 ms
Total iterations: 2000000
Iterations throughput: 11611351 / second
-------------------------------------------------------------------------------
Phase: AsyncProcessor-binary(threads:4)
Average time: 142 ns / iteration
Minimal time: 142 ns / iteration
Maximal time: 148 ns / iteration
Total time: 570.112 ms
Total iterations: 4000000
Iterations throughput: 7016156 / second
-------------------------------------------------------------------------------
Phase: AsyncProcessor-binary(threads:8)
Average time: 138 ns / iteration
Minimal time: 138 ns / iteration
Maximal time: 142 ns / iteration
Total time: 1.111 s
Total iterations: 8000000
Iterations throughput: 7199789 / second
===============================================================================
Benchmark: AsyncProcessor-text
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: AsyncProcessor-text(threads:1)
Average time: 216 ns / iteration
Minimal time: 216 ns / iteration
Maximal time: 241 ns / iteration
Total time: 216.754 ms
Total iterations: 1000000
Iterations throughput: 4613522 / second
-------------------------------------------------------------------------------
Phase: AsyncProcessor-text(threads:2)
Average time: 286 ns / iteration
Minimal time: 286 ns / iteration
Maximal time: 311 ns / iteration
Total time: 573.699 ms
Total iterations: 2000000
Iterations throughput: 3486145 / second
-------------------------------------------------------------------------------
Phase: AsyncProcessor-text(threads:4)
Average time: 338 ns / iteration
Minimal time: 338 ns / iteration
Maximal time: 343 ns / iteration
Total time: 1.352 s
Total iterations: 4000000
Iterations throughput: 2957581 / second
-------------------------------------------------------------------------------
Phase: AsyncProcessor-text(threads:8)
Average time: 350 ns / iteration
Minimal time: 350 ns / iteration
Maximal time: 354 ns / iteration
Total time: 2.805 s
Total iterations: 8000000
Iterations throughput: 2851072 / second
===============================================================================
```

## Benchmark 5: Synchronous processor with file appender
Benchmark source file: [file_sync.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/file_sync.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 21.657 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Sep 28 22:39:44 2016
UTC timestamp: Wed Sep 28 19:39:44 2016
===============================================================================
Benchmark: FileSync-binary
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:1)
Average time: 217 ns / iteration
Minimal time: 217 ns / iteration
Maximal time: 241 ns / iteration
Total time: 217.972 ms
Total iterations: 1000000
Iterations throughput: 4587743 / second
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:2)
Average time: 328 ns / iteration
Minimal time: 328 ns / iteration
Maximal time: 349 ns / iteration
Total time: 657.976 ms
Total iterations: 2000000
Iterations throughput: 3039622 / second
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:4)
Average time: 375 ns / iteration
Minimal time: 375 ns / iteration
Maximal time: 399 ns / iteration
Total time: 1.500 s
Total iterations: 4000000
Iterations throughput: 2666189 / second
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:8)
Average time: 431 ns / iteration
Minimal time: 431 ns / iteration
Maximal time: 451 ns / iteration
Total time: 3.455 s
Total iterations: 8000000
Iterations throughput: 2315052 / second
===============================================================================
Benchmark: FileSync-text
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:1)
Average time: 359 ns / iteration
Minimal time: 359 ns / iteration
Maximal time: 408 ns / iteration
Total time: 359.330 ms
Total iterations: 1000000
Iterations throughput: 2782949 / second
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:2)
Average time: 473 ns / iteration
Minimal time: 473 ns / iteration
Maximal time: 494 ns / iteration
Total time: 946.856 ms
Total iterations: 2000000
Iterations throughput: 2112253 / second
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:4)
Average time: 531 ns / iteration
Minimal time: 531 ns / iteration
Maximal time: 579 ns / iteration
Total time: 2.127 s
Total iterations: 4000000
Iterations throughput: 1880220 / second
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:8)
Average time: 700 ns / iteration
Minimal time: 700 ns / iteration
Maximal time: 724 ns / iteration
Total time: 5.606 s
Total iterations: 8000000
Iterations throughput: 1426888 / second
===============================================================================
```

## Benchmark 6: Asynchronous processor with file appender
Benchmark source file: [file_async.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/file_async.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 21.673 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Sep 28 22:41:46 2016
UTC timestamp: Wed Sep 28 19:41:46 2016
===============================================================================
Benchmark: FileAsync-binary
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: FileAsync-binary(threads:1)
Average time: 131 ns / iteration
Minimal time: 131 ns / iteration
Maximal time: 164 ns / iteration
Total time: 131.399 ms
Total iterations: 1000000
Iterations throughput: 7610405 / second
-------------------------------------------------------------------------------
Phase: FileAsync-binary(threads:2)
Average time: 292 ns / iteration
Minimal time: 292 ns / iteration
Maximal time: 333 ns / iteration
Total time: 584.864 ms
Total iterations: 2000000
Iterations throughput: 3419595 / second
-------------------------------------------------------------------------------
Phase: FileAsync-binary(threads:4)
Average time: 290 ns / iteration
Minimal time: 290 ns / iteration
Maximal time: 322 ns / iteration
Total time: 1.161 s
Total iterations: 4000000
Iterations throughput: 3442603 / second
-------------------------------------------------------------------------------
Phase: FileAsync-binary(threads:8)
Average time: 300 ns / iteration
Minimal time: 300 ns / iteration
Maximal time: 324 ns / iteration
Total time: 2.407 s
Total iterations: 8000000
Iterations throughput: 3322879 / second
===============================================================================
Benchmark: FileAsync-text
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: FileAsync-text(threads:1)
Average time: 283 ns / iteration
Minimal time: 283 ns / iteration
Maximal time: 331 ns / iteration
Total time: 283.559 ms
Total iterations: 1000000
Iterations throughput: 3526594 / second
-------------------------------------------------------------------------------
Phase: FileAsync-text(threads:2)
Average time: 539 ns / iteration
Minimal time: 539 ns / iteration
Maximal time: 591 ns / iteration
Total time: 1.079 s
Total iterations: 2000000
Iterations throughput: 1852223 / second
-------------------------------------------------------------------------------
Phase: FileAsync-text(threads:4)
Average time: 521 ns / iteration
Minimal time: 521 ns / iteration
Maximal time: 561 ns / iteration
Total time: 2.085 s
Total iterations: 4000000
Iterations throughput: 1918385 / second
-------------------------------------------------------------------------------
Phase: FileAsync-text(threads:8)
Average time: 551 ns / iteration
Minimal time: 551 ns / iteration
Maximal time: 576 ns / iteration
Total time: 4.415 s
Total iterations: 8000000
Iterations throughput: 1811865 / second
===============================================================================
```

## Benchmark 7: Format in logging thread vs format in background thread
Benchmark source file: [async_format.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/async_format.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 21.623 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Sep 28 22:54:22 2016
UTC timestamp: Wed Sep 28 19:54:22 2016
===============================================================================
Benchmark: PreFormat
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: PreFormat(threads:1)
Average time: 234 ns / iteration
Minimal time: 234 ns / iteration
Maximal time: 245 ns / iteration
Total time: 234.626 ms
Total iterations: 1000000
Iterations throughput: 4262095 / second
-------------------------------------------------------------------------------
Phase: PreFormat(threads:2)
Average time: 419 ns / iteration
Minimal time: 419 ns / iteration
Maximal time: 549 ns / iteration
Total time: 838.510 ms
Total iterations: 2000000
Iterations throughput: 2385182 / second
-------------------------------------------------------------------------------
Phase: PreFormat(threads:4)
Average time: 493 ns / iteration
Minimal time: 493 ns / iteration
Maximal time: 636 ns / iteration
Total time: 1.974 s
Total iterations: 4000000
Iterations throughput: 2025957 / second
-------------------------------------------------------------------------------
Phase: PreFormat(threads:8)
Average time: 600 ns / iteration
Minimal time: 600 ns / iteration
Maximal time: 616 ns / iteration
Total time: 4.802 s
Total iterations: 8000000
Iterations throughput: 1665872 / second
===============================================================================
Benchmark: PostFormat
Attempts: 5
Iterations: 1000000
-------------------------------------------------------------------------------
Phase: PostFormat(threads:1)
Average time: 441 ns / iteration
Minimal time: 441 ns / iteration
Maximal time: 489 ns / iteration
Total time: 441.860 ms
Total iterations: 1000000
Iterations throughput: 2263157 / second
-------------------------------------------------------------------------------
Phase: PostFormat(threads:2)
Average time: 681 ns / iteration
Minimal time: 681 ns / iteration
Maximal time: 712 ns / iteration
Total time: 1.362 s
Total iterations: 2000000
Iterations throughput: 1468368 / second
-------------------------------------------------------------------------------
Phase: PostFormat(threads:4)
Average time: 684 ns / iteration
Minimal time: 684 ns / iteration
Maximal time: 745 ns / iteration
Total time: 2.738 s
Total iterations: 4000000
Iterations throughput: 1460445 / second
-------------------------------------------------------------------------------
Phase: PostFormat(threads:8)
Average time: 821 ns / iteration
Minimal time: 821 ns / iteration
Maximal time: 848 ns / iteration
Total time: 6.572 s
Total iterations: 8000000
Iterations throughput: 1217128 / second
===============================================================================
```

# Tools

## Binary log reader
Binary log reader is used to convert binary logs ('.bin.log' files) to the
human-readable text format. By default it reads binary stream from 'stdin' and
outputs converted text to 'stdout'. However it is possible to provide explicit
input/output file names using command line arguments:

```
Usage: binlog [options]

Options:
  --version             show program's version number and exit
  -h, --help            show this help message and exit
  -h HELP, --help=HELP  Show help
  -i INPUT, --input=INPUT
                        Input file name
  -o OUTPUT, --output=OUTPUT
                        Output file name

```
