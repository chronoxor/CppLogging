# CppLogging

[![Linux build status](https://img.shields.io/travis/chronoxor/CppLogging/master.svg?label=Linux)](https://travis-ci.org/chronoxor/CppLogging)
[![OSX build status](https://img.shields.io/travis/chronoxor/CppLogging/master.svg?label=OSX)](https://travis-ci.org/chronoxor/CppLogging)
[![Cygwin build status](https://img.shields.io/appveyor/ci/chronoxor/CppLogging/master.svg?label=Cygwin)](https://ci.appveyor.com/project/chronoxor/CppLogging)
[![MinGW build status](https://img.shields.io/appveyor/ci/chronoxor/CppLogging/master.svg?label=MinGW)](https://ci.appveyor.com/project/chronoxor/CppLogging)
[![Windows build status](https://img.shields.io/appveyor/ci/chronoxor/CppLogging/master.svg?label=Windows)](https://ci.appveyor.com/project/chronoxor/CppLogging)

C++ Logging Library provides functionality to log different events with a high
throughput in multithreaded environment into different sinks (console, files,
rolling files, syslog, etc.). Logging configuration is very flexible and gives
functionality to build flexible logger hierarchy with combination of logging
processors (sync, async), filters, layouts (binary, text) and appenders.

[CppLogging API reference](https://chronoxor.github.io/CppLogging/index.html)

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
  * [Logging examples](#logging-examples)
    * [Example 1: Default logger](#example-1-default-logger)
    * [Example 2: Format with logger](#example-2-format-with-logger)
    * [Example 3: Configure custom logger with text layout and console appender](#example-3-configure-custom-logger-with-text-layout-and-console-appender)
    * [Example 4: Configure custom logger with text layout and syslog appender](#example-4-configure-custom-logger-with-text-layout-and-syslog-appender)
    * [Example 5: Configure custom logger with binary layout and file appender](#example-5-configure-custom-logger-with-binary-layout-and-file-appender)
    * [Example 6: Configure logger with custom text layout pattern](#example-6-configure-logger-with-custom-text-layout-pattern)
    * [Example 7: Configure rolling file appender with time-based policy](#example-7-configure-rolling-file-appender-with-time-based-policy)
    * [Example 8: Configure rolling file appender with size-based policy](#example-8-configure-rolling-file-appender-with-size-based-policy)
    * [Example 9: Multithreaded logging with synchronous processor](#example-9-multithreaded-logging-with-synchronous-processor)
    * [Example 10: Multithreaded logging with asynchronous processor](#example-10-multithreaded-logging-with-asynchronous-processor)
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
* Linux (binutils-dev uuid-dev)
* OSX
* Windows 10
* [cmake](https://www.cmake.org)
* [gcc](https://gcc.gnu.org)
* [git](https://git-scm.com)
* [gil](https://github.com/chronoxor/gil.git)

Optional:
* [clang](https://clang.llvm.org)
* [clion](https://www.jetbrains.com/clion)
* [Cygwin](https://cygwin.com)
* [MinGW](https://mingw-w64.org/doku.php)
* [Visual Studio](https://www.visualstudio.com)

# How to build?

### Setup repository with [gil (git links) tool](https://github.com/chronoxor/gil#setup)
```shell
git clone https://github.com/chronoxor/CppLogging.git
cd CppLogging
gil update
```

### Linux
```shell
cd build
./unix.sh
```

### OSX
```shell
cd build
./unix.sh
```

### Windows (Cygwin)
```shell
cd build
cygwin.bat
```

### Windows (MinGW)
```shell
cd build
mingw.bat
```

### Windows (Visual Studio)
```shell
cd build
vs.bat
```

# Logging examples

## Example 1: Default logger
This is the simple example of using default logger. Just link it with
CppLogging library and you'll get default logger functionality with
text layout and console appender:

```c++
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
![Default report](https://github.com/chronoxor/CppLogging/raw/master/images/default.png)

## Example 2: Format with logger
CppLogging library provides powerful logging format API based on the
[{fmt} library](http://fmtlib.net):

```c++
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

```c++
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

```c++
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

```c++
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

```c++
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

```c++
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

```c++
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

## Example 9: Multithreaded logging with synchronous processor
Synchronous processor uses critical-section lock to avoid multiple
threads from logging at the same time (logging threads are waiting
until critical-section is released).

This example shows how to configure a custom logger with a given name to
use synchronous processor in multithreaded environment:

```c++
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

## Example 10: Multithreaded logging with asynchronous processor
Asynchronous processor uses lock-free queue to collect logging records from
multiple threads at the same time.

This example shows much better performance with less threads contentions in
comparison with the previous one for lots of threads:

```c++
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
Operations: 10000000
-------------------------------------------------------------------------------
Phase: NullAppender-binary
Average time: 62 ns/op
Minimal time: 62 ns/op
Maximal time: 63 ns/op
Total time: 624.974 ms
Total operations: 10000000
Operations throughput: 16000657 ops/s
===============================================================================
Benchmark: NullAppender-text
Attempts: 5
Operations: 10000000
-------------------------------------------------------------------------------
Phase: NullAppender-text
Average time: 174 ns/op
Minimal time: 174 ns/op
Maximal time: 176 ns/op
Total time: 1.744 s
Total operations: 10000000
Operations throughput: 5733126 ops/s
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
Operations: 1000000
-------------------------------------------------------------------------------
Phase: FileAppender-binary
Average time: 106 ns/op
Minimal time: 106 ns/op
Maximal time: 117 ns/op
Total time: 106.598 ms
Total operations: 1000000
Operations throughput: 9381018 ops/s
===============================================================================
Benchmark: FileAppender-text
Attempts: 5
Operations: 1000000
-------------------------------------------------------------------------------
Phase: FileAppender-text
Average time: 229 ns/op
Minimal time: 229 ns/op
Maximal time: 249 ns/op
Total time: 229.126 ms
Total operations: 1000000
Operations throughput: 4364404 ops/s
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
Operations: 1000000
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:1)
Average time: 27 ns/op
Minimal time: 27 ns/op
Maximal time: 30 ns/op
Total time: 55.891 ms
Total operations: 2000000
Operations throughput: 35783702 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:2)
Average time: 22 ns/op
Minimal time: 22 ns/op
Maximal time: 29 ns/op
Total time: 89.344 ms
Total operations: 4000000
Operations throughput: 44770342 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:4)
Average time: 84 ns/op
Minimal time: 84 ns/op
Maximal time: 92 ns/op
Total time: 674.796 ms
Total operations: 8000000
Operations throughput: 11855429 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:8)
Average time: 113 ns/op
Minimal time: 113 ns/op
Maximal time: 113 ns/op
Total time: 1.808 s
Total operations: 16000000
Operations throughput: 8849109 ops/s
===============================================================================
Benchmark: SyncProcessor-binary
Attempts: 5
Operations: 1000000
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:1)
Average time: 74 ns/op
Minimal time: 74 ns/op
Maximal time: 83 ns/op
Total time: 74.038 ms
Total operations: 1000000
Operations throughput: 13506405 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:2)
Average time: 123 ns/op
Minimal time: 123 ns/op
Maximal time: 133 ns/op
Total time: 246.187 ms
Total operations: 2000000
Operations throughput: 8123893 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:4)
Average time: 205 ns/op
Minimal time: 205 ns/op
Maximal time: 210 ns/op
Total time: 821.948 ms
Total operations: 4000000
Operations throughput: 4866485 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:8)
Average time: 276 ns/op
Minimal time: 276 ns/op
Maximal time: 278 ns/op
Total time: 2.213 s
Total operations: 8000000
Operations throughput: 3614274 ops/s
===============================================================================
Benchmark: SyncProcessor-text
Attempts: 5
Operations: 1000000
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:1)
Average time: 177 ns/op
Minimal time: 177 ns/op
Maximal time: 180 ns/op
Total time: 177.173 ms
Total operations: 1000000
Operations throughput: 5644200 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:2)
Average time: 222 ns/op
Minimal time: 222 ns/op
Maximal time: 241 ns/op
Total time: 444.429 ms
Total operations: 2000000
Operations throughput: 4500155 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:4)
Average time: 308 ns/op
Minimal time: 308 ns/op
Maximal time: 315 ns/op
Total time: 1.235 s
Total operations: 4000000
Operations throughput: 3237170 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:8)
Average time: 405 ns/op
Minimal time: 405 ns/op
Maximal time: 409 ns/op
Total time: 3.242 s
Total operations: 8000000
Operations throughput: 2467338 ops/s
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
Operations: 1000000
-------------------------------------------------------------------------------
Phase: AsyncProcessor-null(threads:1)
Average time: 71 ns/op
Minimal time: 71 ns/op
Maximal time: 79 ns/op
Total time: 143.543 ms
Total operations: 2000000
Operations throughput: 13933096 ops/s
-------------------------------------------------------------------------------
Phase: AsyncProcessor-null(threads:2)
Average time: 47 ns/op
Minimal time: 47 ns/op
Maximal time: 52 ns/op
Total time: 191.668 ms
Total operations: 4000000
Operations throughput: 20869336 ops/s
-------------------------------------------------------------------------------
Phase: AsyncProcessor-null(threads:4)
Average time: 30 ns/op
Minimal time: 30 ns/op
Maximal time: 32 ns/op
Total time: 244.934 ms
Total operations: 8000000
Operations throughput: 32661750 ops/s
-------------------------------------------------------------------------------
Phase: AsyncProcessor-null(threads:8)
Average time: 44 ns/op
Minimal time: 44 ns/op
Maximal time: 44 ns/op
Total time: 704.504 ms
Total operations: 16000000
Operations throughput: 22710985 ops/s
===============================================================================
Benchmark: AsyncProcessor-binary
Attempts: 5
Operations: 1000000
-------------------------------------------------------------------------------
Phase: AsyncProcessor-binary(threads:1)
Average time: 130 ns/op
Minimal time: 130 ns/op
Maximal time: 148 ns/op
Total time: 130.037 ms
Total operations: 1000000
Operations throughput: 7690098 ops/s
-------------------------------------------------------------------------------
Phase: AsyncProcessor-binary(threads:2)
Average time: 86 ns/op
Minimal time: 86 ns/op
Maximal time: 100 ns/op
Total time: 172.245 ms
Total operations: 2000000
Operations throughput: 11611351 ops/s
-------------------------------------------------------------------------------
Phase: AsyncProcessor-binary(threads:4)
Average time: 142 ns/op
Minimal time: 142 ns/op
Maximal time: 148 ns/op
Total time: 570.112 ms
Total operations: 4000000
Operations throughput: 7016156 ops/s
-------------------------------------------------------------------------------
Phase: AsyncProcessor-binary(threads:8)
Average time: 138 ns/op
Minimal time: 138 ns/op
Maximal time: 142 ns/op
Total time: 1.111 s
Total operations: 8000000
Operations throughput: 7199789 ops/s
===============================================================================
Benchmark: AsyncProcessor-text
Attempts: 5
Operations: 1000000
-------------------------------------------------------------------------------
Phase: AsyncProcessor-text(threads:1)
Average time: 216 ns/op
Minimal time: 216 ns/op
Maximal time: 241 ns/op
Total time: 216.754 ms
Total operations: 1000000
Operations throughput: 4613522 ops/s
-------------------------------------------------------------------------------
Phase: AsyncProcessor-text(threads:2)
Average time: 286 ns/op
Minimal time: 286 ns/op
Maximal time: 311 ns/op
Total time: 573.699 ms
Total operations: 2000000
Operations throughput: 3486145 ops/s
-------------------------------------------------------------------------------
Phase: AsyncProcessor-text(threads:4)
Average time: 338 ns/op
Minimal time: 338 ns/op
Maximal time: 343 ns/op
Total time: 1.352 s
Total operations: 4000000
Operations throughput: 2957581 ops/s
-------------------------------------------------------------------------------
Phase: AsyncProcessor-text(threads:8)
Average time: 350 ns/op
Minimal time: 350 ns/op
Maximal time: 354 ns/op
Total time: 2.805 s
Total operations: 8000000
Operations throughput: 2851072 ops/s
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
Operations: 1000000
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:1)
Average time: 217 ns/op
Minimal time: 217 ns/op
Maximal time: 241 ns/op
Total time: 217.972 ms
Total operations: 1000000
Operations throughput: 4587743 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:2)
Average time: 328 ns/op
Minimal time: 328 ns/op
Maximal time: 349 ns/op
Total time: 657.976 ms
Total operations: 2000000
Operations throughput: 3039622 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:4)
Average time: 375 ns/op
Minimal time: 375 ns/op
Maximal time: 399 ns/op
Total time: 1.500 s
Total operations: 4000000
Operations throughput: 2666189 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:8)
Average time: 431 ns/op
Minimal time: 431 ns/op
Maximal time: 451 ns/op
Total time: 3.455 s
Total operations: 8000000
Operations throughput: 2315052 ops/s
===============================================================================
Benchmark: FileSync-text
Attempts: 5
Operations: 1000000
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:1)
Average time: 359 ns/op
Minimal time: 359 ns/op
Maximal time: 408 ns/op
Total time: 359.330 ms
Total operations: 1000000
Operations throughput: 2782949 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:2)
Average time: 473 ns/op
Minimal time: 473 ns/op
Maximal time: 494 ns/op
Total time: 946.856 ms
Total operations: 2000000
Operations throughput: 2112253 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:4)
Average time: 531 ns/op
Minimal time: 531 ns/op
Maximal time: 579 ns/op
Total time: 2.127 s
Total operations: 4000000
Operations throughput: 1880220 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:8)
Average time: 700 ns/op
Minimal time: 700 ns/op
Maximal time: 724 ns/op
Total time: 5.606 s
Total operations: 8000000
Operations throughput: 1426888 ops/s
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
Operations: 1000000
-------------------------------------------------------------------------------
Phase: FileAsync-binary(threads:1)
Average time: 131 ns/op
Minimal time: 131 ns/op
Maximal time: 164 ns/op
Total time: 131.399 ms
Total operations: 1000000
Operations throughput: 7610405 ops/s
-------------------------------------------------------------------------------
Phase: FileAsync-binary(threads:2)
Average time: 292 ns/op
Minimal time: 292 ns/op
Maximal time: 333 ns/op
Total time: 584.864 ms
Total operations: 2000000
Operations throughput: 3419595 ops/s
-------------------------------------------------------------------------------
Phase: FileAsync-binary(threads:4)
Average time: 290 ns/op
Minimal time: 290 ns/op
Maximal time: 322 ns/op
Total time: 1.161 s
Total operations: 4000000
Operations throughput: 3442603 ops/s
-------------------------------------------------------------------------------
Phase: FileAsync-binary(threads:8)
Average time: 300 ns/op
Minimal time: 300 ns/op
Maximal time: 324 ns/op
Total time: 2.407 s
Total operations: 8000000
Operations throughput: 3322879 ops/s
===============================================================================
Benchmark: FileAsync-text
Attempts: 5
Operations: 1000000
-------------------------------------------------------------------------------
Phase: FileAsync-text(threads:1)
Average time: 283 ns/op
Minimal time: 283 ns/op
Maximal time: 331 ns/op
Total time: 283.559 ms
Total operations: 1000000
Operations throughput: 3526594 ops/s
-------------------------------------------------------------------------------
Phase: FileAsync-text(threads:2)
Average time: 539 ns/op
Minimal time: 539 ns/op
Maximal time: 591 ns/op
Total time: 1.079 s
Total operations: 2000000
Operations throughput: 1852223 ops/s
-------------------------------------------------------------------------------
Phase: FileAsync-text(threads:4)
Average time: 521 ns/op
Minimal time: 521 ns/op
Maximal time: 561 ns/op
Total time: 2.085 s
Total operations: 4000000
Operations throughput: 1918385 ops/s
-------------------------------------------------------------------------------
Phase: FileAsync-text(threads:8)
Average time: 551 ns/op
Minimal time: 551 ns/op
Maximal time: 576 ns/op
Total time: 4.415 s
Total operations: 8000000
Operations throughput: 1811865 ops/s
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
Operations: 1000000
-------------------------------------------------------------------------------
Phase: PreFormat(threads:1)
Average time: 234 ns/op
Minimal time: 234 ns/op
Maximal time: 245 ns/op
Total time: 234.626 ms
Total operations: 1000000
Operations throughput: 4262095 ops/s
-------------------------------------------------------------------------------
Phase: PreFormat(threads:2)
Average time: 419 ns/op
Minimal time: 419 ns/op
Maximal time: 549 ns/op
Total time: 838.510 ms
Total operations: 2000000
Operations throughput: 2385182 ops/s
-------------------------------------------------------------------------------
Phase: PreFormat(threads:4)
Average time: 493 ns/op
Minimal time: 493 ns/op
Maximal time: 636 ns/op
Total time: 1.974 s
Total operations: 4000000
Operations throughput: 2025957 ops/s
-------------------------------------------------------------------------------
Phase: PreFormat(threads:8)
Average time: 600 ns/op
Minimal time: 600 ns/op
Maximal time: 616 ns/op
Total time: 4.802 s
Total operations: 8000000
Operations throughput: 1665872 ops/s
===============================================================================
Benchmark: PostFormat
Attempts: 5
Operations: 1000000
-------------------------------------------------------------------------------
Phase: PostFormat(threads:1)
Average time: 441 ns/op
Minimal time: 441 ns/op
Maximal time: 489 ns/op
Total time: 441.860 ms
Total operations: 1000000
Operations throughput: 2263157 ops/s
-------------------------------------------------------------------------------
Phase: PostFormat(threads:2)
Average time: 681 ns/op
Minimal time: 681 ns/op
Maximal time: 712 ns/op
Total time: 1.362 s
Total operations: 2000000
Operations throughput: 1468368 ops/s
-------------------------------------------------------------------------------
Phase: PostFormat(threads:4)
Average time: 684 ns/op
Minimal time: 684 ns/op
Maximal time: 745 ns/op
Total time: 2.738 s
Total operations: 4000000
Operations throughput: 1460445 ops/s
-------------------------------------------------------------------------------
Phase: PostFormat(threads:8)
Average time: 821 ns/op
Minimal time: 821 ns/op
Maximal time: 848 ns/op
Total time: 6.572 s
Total operations: 8000000
Operations throughput: 1217128 ops/s
===============================================================================
```

# Tools

## Binary log reader
Binary log reader is used to convert binary logs ('.bin.log' files) to the
human-readable text format. By default it reads binary stream from 'stdin' and
outputs converted text to 'stdout'. However it is possible to provide explicit
input/output file names using command line arguments:

```shell
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
