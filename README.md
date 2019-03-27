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
* [CLion](https://www.jetbrains.com/clion)
* [Cygwin](https://cygwin.com)
* [MinGW](https://mingw-w64.org/doku.php)
* [Visual Studio](https://www.visualstudio.com)

# How to build?

### Install [gil (git links) tool](https://github.com/chronoxor/gil)
```shell
pip3 install gil
```

### Setup repository
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

    friend CppLogging::Record& operator<<(CppLogging::Record& record, const Date& date)
    { return record.StoreCustomFormat("{}-{}-{}", date._year, date._month, date._day); }

private:
    int _year, _month, _day;
};

class DateTime
{
public:
    DateTime(Date date, int hours, int minutes, int seconds) : _date(date), _hours(hours), _minutes(minutes), _seconds(seconds) {}

    friend std::ostream& operator<<(std::ostream& os, const DateTime& datetime)
    { return os << datetime._date << " " << datetime._hours << ':' << datetime._minutes << ':' << datetime._seconds; }

    friend CppLogging::Record& operator<<(CppLogging::Record& record, const DateTime& datetime)
    {
        const size_t begin = record.StoreListBegin();
        record.StoreList(datetime._date);
        record.StoreList(' ');
        record.StoreList(datetime._hours);
        record.StoreList(':');
        record.StoreList(datetime._minutes);
        record.StoreList(':');
        record.StoreList(datetime._seconds);
        return record.StoreListEnd(begin);
    }

private:
    Date _date;
    int _hours, _minutes, _seconds;
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
    logger.Info("The datetime is {}", DateTime(Date(2012, 12, 9), 13, 15, 57));
    logger.Info("Elapsed time: {s:.2f} seconds", "s"_a = 1.23);
    logger.Info("The answer is {}"_format(42));

    return 0;
}
```

Example will create the following log:
```
2019-03-27T12:04:19.881Z [0x0000FFB8] INFO   - argc: 1, argv: 0x1ff83563210
2019-03-27T12:04:19.882Z [0x0000FFB8] INFO   - no arguments
2019-03-27T12:04:19.883Z [0x0000FFB8] INFO   - -1, 0, 1
2019-03-27T12:04:19.884Z [0x0000FFB8] INFO   - a, b, c
2019-03-27T12:04:19.884Z [0x0000FFB8] INFO   - a, b, c
2019-03-27T12:04:19.884Z [0x0000FFB8] INFO   - c, b, a
2019-03-27T12:04:19.884Z [0x0000FFB8] INFO   - abracadabra
2019-03-27T12:04:19.885Z [0x0000FFB8] INFO   - left aligned
2019-03-27T12:04:19.885Z [0x0000FFB8] INFO   -                  right aligned
2019-03-27T12:04:19.885Z [0x0000FFB8] INFO   -            centered
2019-03-27T12:04:19.886Z [0x0000FFB8] INFO   - ***********centered***********
2019-03-27T12:04:19.886Z [0x0000FFB8] INFO   - +3.140000; -3.140000
2019-03-27T12:04:19.887Z [0x0000FFB8] INFO   -  3.140000; -3.140000
2019-03-27T12:04:19.887Z [0x0000FFB8] INFO   - 3.140000; -3.140000
2019-03-27T12:04:19.887Z [0x0000FFB8] INFO   - int: 42;  hex: 2a;  oct: 52; bin: 101010
2019-03-27T12:04:19.888Z [0x0000FFB8] INFO   - int: 42;  hex: 0x2a;  oct: 052;  bin: 0b101010
2019-03-27T12:04:19.888Z [0x0000FFB8] INFO   - The date is 2012-12-9
2019-03-27T12:04:19.888Z [0x0000FFB8] INFO   - The datetime is 2012-12-9 13:15:57
2019-03-27T12:04:19.889Z [0x0000FFB8] INFO   - Elapsed time: 1.23 seconds
2019-03-27T12:04:19.889Z [0x0000FFB8] INFO   - The answer is 42
```

## Example 3: Configure custom logger with text layout and console appender
This example shows how to configure a custom logger with a given name to
perform logging with a text layout and console appender sink:

```c++
#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create default logging sink processor with a text layout
    auto sink = std::make_shared<CppLogging::Processor>(std::make_shared<CppLogging::TextLayout>());
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
```

## Example 5: Configure custom logger with binary layout and file appender
This example shows how to configure a custom logger with a given name to
perform logging with a binary layout and file appender sink:

```c++
#include "logging/config.h"
#include "logging/logger.h"

void ConfigureLogger()
{
    // Create default logging sink processor with a binary layout
    auto sink = std::make_shared<CppLogging::Processor>(std::make_shared<CppLogging::BinaryLayout>());
    // Add file appender
    sink->appenders().push_back(std::make_shared<CppLogging::FileAppender>("file.bin.log"));

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
    const std::string pattern = "{UtcYear}-{UtcMonth}-{UtcDay}T{UtcHour}:{UtcMinute}:{UtcSecond}.{Millisecond}{UtcTimezone} - {Microsecond}.{Nanosecond} - [{Thread}] - {Level} - {Logger} - {Message} - {EndLine}";

    // Create default logging sink processor with a text layout
    auto sink = std::make_shared<CppLogging::Processor>(std::make_shared<CppLogging::TextLayout>(pattern));
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
    // Create default logging sink processor with a text layout
    auto sink = std::make_shared<CppLogging::Processor>(std::make_shared<CppLogging::TextLayout>());
    // Add rolling file appender which rolls each second and create log file with a pattern "{UtcDateTime}.log"
    sink->appenders().push_back(std::make_shared<CppLogging::RollingFileAppender>(".", CppLogging::TimeRollingPolicy::SECOND, "{UtcDateTime}.log", true));

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
    // Create default logging sink processor with a binary layout
    auto sink = std::make_shared<CppLogging::Processor>(std::make_shared<CppLogging::BinaryLayout>());
    // Add rolling file appender which rolls after append 4kb of logs and will keep only 5 recent archives
    sink->appenders().push_back(std::make_shared<CppLogging::RollingFileAppender>(".", "file", "bin.log", 4096, 5, true));

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
    // Create default logging sink processor with a text layout
    auto sink = std::make_shared<CppLogging::AsyncWaitProcessor>(std::make_shared<CppLogging::TextLayout>());
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
        threads.push_back(std::thread([&stop]()
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
CPU architecutre: Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
CPU logical cores: 12
CPU physical cores: 6
CPU clock speed: 2.208 GHz
CPU Hyper-Threading: enabled
RAM total: 31.757 GiB
RAM free: 20.778 GiB
===============================================================================
OS version: Microsoft Windows 8  (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 15:53:10 2019
UTC timestamp: Wed Mar 27 12:53:10 2019
===============================================================================
Benchmark: NullAppender-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: NullAppender-binary
Average time: 54 ns/op
Minimal time: 54 ns/op
Maximal time: 54 ns/op
Total time: 3.116 s
Total operations: 57191472
Operations throughput: 18350957 ops/s
===============================================================================
Benchmark: NullAppender-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: NullAppender-text
Average time: 230 ns/op
Minimal time: 230 ns/op
Maximal time: 236 ns/op
Total time: 4.264 s
Total operations: 18491200
Operations throughput: 4336537 ops/s
===============================================================================
```

## Benchmark 2: File appender
Benchmark source file: [appender_file.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/appender_file.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
CPU logical cores: 12
CPU physical cores: 6
CPU clock speed: 2.208 GHz
CPU Hyper-Threading: enabled
RAM total: 31.757 GiB
RAM free: 19.087 GiB
===============================================================================
OS version: Microsoft Windows 8  (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 15:54:31 2019
UTC timestamp: Wed Mar 27 12:54:31 2019
===============================================================================
Benchmark: FileAppender-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAppender-binary
Average time: 90 ns/op
Minimal time: 90 ns/op
Maximal time: 93 ns/op
Total time: 3.605 s
Total operations: 39912830
Operations throughput: 11070931 ops/s
===============================================================================
Benchmark: FileAppender-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAppender-text
Average time: 271 ns/op
Minimal time: 271 ns/op
Maximal time: 278 ns/op
Total time: 4.395 s
Total operations: 16173663
Operations throughput: 3679369 ops/s
===============================================================================
```

## Benchmark 3: Synchronous processor with null appender
Benchmark source file: [processor_sync.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/processor_sync.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
CPU logical cores: 12
CPU physical cores: 6
CPU clock speed: 2.208 GHz
CPU Hyper-Threading: enabled
RAM total: 31.757 GiB
RAM free: 21.039 GiB
===============================================================================
OS version: Microsoft Windows 8  (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 16:02:50 2019
UTC timestamp: Wed Mar 27 13:02:50 2019
===============================================================================
Benchmark: SyncProcessor-null
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:1)
Average time: 81 ns/op
Minimal time: 81 ns/op
Maximal time: 81 ns/op
Total time: 4.549 s
Total operations: 55779606
Operations throughput: 12260349 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:2)
Average time: 102 ns/op
Minimal time: 102 ns/op
Maximal time: 117 ns/op
Total time: 9.588 s
Total operations: 93385368
Operations throughput: 9739525 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:4)
Average time: 177 ns/op
Minimal time: 177 ns/op
Maximal time: 193 ns/op
Total time: 7.746 s
Total operations: 43538920
Operations throughput: 5620489 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:8)
Average time: 222 ns/op
Minimal time: 222 ns/op
Maximal time: 225 ns/op
Total time: 6.296 s
Total operations: 28233976
Operations throughput: 4484415 ops/s
===============================================================================
Benchmark: SyncProcessor-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:1)
Average time: 93 ns/op
Minimal time: 93 ns/op
Maximal time: 94 ns/op
Total time: 4.660 s
Total operations: 49884690
Operations throughput: 10703408 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:2)
Average time: 130 ns/op
Minimal time: 130 ns/op
Maximal time: 146 ns/op
Total time: 9.283 s
Total operations: 71263812
Operations throughput: 7676172 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:4)
Average time: 200 ns/op
Minimal time: 200 ns/op
Maximal time: 204 ns/op
Total time: 6.177 s
Total operations: 30850384
Operations throughput: 4993767 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:8)
Average time: 296 ns/op
Minimal time: 296 ns/op
Maximal time: 301 ns/op
Total time: 6.068 s
Total operations: 20432176
Operations throughput: 3367129 ops/s
===============================================================================
Benchmark: SyncProcessor-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:1)
Average time: 297 ns/op
Minimal time: 297 ns/op
Maximal time: 304 ns/op
Total time: 5.460 s
Total operations: 18352310
Operations throughput: 3361061 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:2)
Average time: 351 ns/op
Minimal time: 351 ns/op
Maximal time: 370 ns/op
Total time: 6.400 s
Total operations: 18203816
Operations throughput: 2843934 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:4)
Average time: 476 ns/op
Minimal time: 476 ns/op
Maximal time: 491 ns/op
Total time: 6.152 s
Total operations: 12920708
Operations throughput: 2100139 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:8)
Average time: 575 ns/op
Minimal time: 575 ns/op
Maximal time: 586 ns/op
Total time: 5.959 s
Total operations: 10354720
Operations throughput: 1737508 ops/s
===============================================================================
```

## Benchmark 4: Asynchronous processor with null appender
Benchmark source file: [processor_async.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/processor_async.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
CPU logical cores: 12
CPU physical cores: 6
CPU clock speed: 2.208 GHz
CPU Hyper-Threading: enabled
RAM total: 31.757 GiB
RAM free: 21.002 GiB
===============================================================================
OS version: Microsoft Windows 8  (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 16:19:08 2019
UTC timestamp: Wed Mar 27 13:19:08 2019
===============================================================================
Benchmark: AsyncWaitProcessor-null
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-null(threads:1)
Average time: 134 ns/op
Minimal time: 134 ns/op
Maximal time: 141 ns/op
Total time: 6.033 s
Total operations: 44976168
Operations throughput: 7455023 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-null(threads:2)
Average time: 110 ns/op
Minimal time: 110 ns/op
Maximal time: 115 ns/op
Total time: 6.438 s
Total operations: 58195188
Operations throughput: 9039268 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-null(threads:4)
Average time: 84 ns/op
Minimal time: 84 ns/op
Maximal time: 86 ns/op
Total time: 6.582 s
Total operations: 78026184
Operations throughput: 11852964 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-null(threads:8)
Average time: 140 ns/op
Minimal time: 140 ns/op
Maximal time: 151 ns/op
Total time: 6.183 s
Total operations: 43878216
Operations throughput: 7095827 ops/s
===============================================================================
Benchmark: AsyncWaitFreeProcessor-null
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-null(threads:1)
Average time: 133 ns/op
Minimal time: 133 ns/op
Maximal time: 146 ns/op
Total time: 5.940 s
Total operations: 44474022
Operations throughput: 7486620 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-null(threads:2)
Average time: 110 ns/op
Minimal time: 110 ns/op
Maximal time: 112 ns/op
Total time: 6.477 s
Total operations: 58416282
Operations throughput: 9017922 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-null(threads:4)
Average time: 89 ns/op
Minimal time: 89 ns/op
Maximal time: 90 ns/op
Total time: 6.691 s
Total operations: 75060304
Operations throughput: 11217520 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-null(threads:8)
Average time: 144 ns/op
Minimal time: 144 ns/op
Maximal time: 151 ns/op
Total time: 6.057 s
Total operations: 41881240
Operations throughput: 6913597 ops/s
===============================================================================
Benchmark: AsyncWaitProcessor-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-binary(threads:1)
Average time: 116 ns/op
Minimal time: 116 ns/op
Maximal time: 130 ns/op
Total time: 5.133 s
Total operations: 44014650
Operations throughput: 8574376 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-binary(threads:2)
Average time: 108 ns/op
Minimal time: 108 ns/op
Maximal time: 109 ns/op
Total time: 6.398 s
Total operations: 59107374
Operations throughput: 9237683 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-binary(threads:4)
Average time: 191 ns/op
Minimal time: 191 ns/op
Maximal time: 283 ns/op
Total time: 5.715 s
Total operations: 29837940
Operations throughput: 5220444 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-binary(threads:8)
Average time: 246 ns/op
Minimal time: 246 ns/op
Maximal time: 275 ns/op
Total time: 5.871 s
Total operations: 23839880
Operations throughput: 4060572 ops/s
===============================================================================
Benchmark: AsyncWaitFreeProcessor-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-binary(threads:1)
Average time: 149 ns/op
Minimal time: 149 ns/op
Maximal time: 178 ns/op
Total time: 4.484 s
Total operations: 30074012
Operations throughput: 6706715 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-binary(threads:2)
Average time: 115 ns/op
Minimal time: 115 ns/op
Maximal time: 146 ns/op
Total time: 5.240 s
Total operations: 45248064
Operations throughput: 8633681 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-binary(threads:4)
Average time: 184 ns/op
Minimal time: 184 ns/op
Maximal time: 254 ns/op
Total time: 6.264 s
Total operations: 33920192
Operations throughput: 5415030 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-binary(threads:8)
Average time: 246 ns/op
Minimal time: 246 ns/op
Maximal time: 265 ns/op
Total time: 5.786 s
Total operations: 23449392
Operations throughput: 4052628 ops/s
===============================================================================
Benchmark: AsyncWaitProcessor-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-text(threads:1)
Average time: 349 ns/op
Minimal time: 349 ns/op
Maximal time: 371 ns/op
Total time: 6.572 s
Total operations: 18792224
Operations throughput: 2859062 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-text(threads:2)
Average time: 386 ns/op
Minimal time: 386 ns/op
Maximal time: 413 ns/op
Total time: 6.001 s
Total operations: 15521430
Operations throughput: 2586115 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-text(threads:4)
Average time: 493 ns/op
Minimal time: 493 ns/op
Maximal time: 525 ns/op
Total time: 5.930 s
Total operations: 12021376
Operations throughput: 2027009 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-text(threads:8)
Average time: 632 ns/op
Minimal time: 632 ns/op
Maximal time: 663 ns/op
Total time: 5.727 s
Total operations: 9059896
Operations throughput: 1581719 ops/s
===============================================================================
Benchmark: AsyncWaitFreeProcessor-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-text(threads:1)
Average time: 355 ns/op
Minimal time: 355 ns/op
Maximal time: 379 ns/op
Total time: 6.515 s
Total operations: 18329296
Operations throughput: 2813048 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-text(threads:2)
Average time: 401 ns/op
Minimal time: 401 ns/op
Maximal time: 421 ns/op
Total time: 6.184 s
Total operations: 15414816
Operations throughput: 2492408 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-text(threads:4)
Average time: 517 ns/op
Minimal time: 517 ns/op
Maximal time: 533 ns/op
Total time: 5.985 s
Total operations: 11559852
Operations throughput: 1931368 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-text(threads:8)
Average time: 722 ns/op
Minimal time: 722 ns/op
Maximal time: 735 ns/op
Total time: 5.930 s
Total operations: 8208632
Operations throughput: 1384127 ops/s
===============================================================================
```

## Benchmark 5: Synchronous processor with file appender
Benchmark source file: [file_sync.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/file_sync.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
CPU logical cores: 12
CPU physical cores: 6
CPU clock speed: 2.208 GHz
CPU Hyper-Threading: enabled
RAM total: 31.757 GiB
RAM free: 21.136 GiB
===============================================================================
OS version: Microsoft Windows 8  (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 16:27:31 2019
UTC timestamp: Wed Mar 27 13:27:31 2019
===============================================================================
Benchmark: FileSync-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:1)
Average time: 148 ns/op
Minimal time: 148 ns/op
Maximal time: 154 ns/op
Total time: 5.386 s
Total operations: 36302916
Operations throughput: 6739413 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:2)
Average time: 210 ns/op
Minimal time: 210 ns/op
Maximal time: 224 ns/op
Total time: 6.955 s
Total operations: 33089598
Operations throughput: 4757379 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:4)
Average time: 307 ns/op
Minimal time: 307 ns/op
Maximal time: 319 ns/op
Total time: 6.660 s
Total operations: 21686472
Operations throughput: 3256093 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:8)
Average time: 472 ns/op
Minimal time: 472 ns/op
Maximal time: 502 ns/op
Total time: 6.042 s
Total operations: 12776416
Operations throughput: 2114452 ops/s
===============================================================================
Benchmark: FileSync-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:1)
Average time: 388 ns/op
Minimal time: 388 ns/op
Maximal time: 403 ns/op
Total time: 5.673 s
Total operations: 14598922
Operations throughput: 2573139 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:2)
Average time: 455 ns/op
Minimal time: 455 ns/op
Maximal time: 478 ns/op
Total time: 6.650 s
Total operations: 14603576
Operations throughput: 2196023 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:4)
Average time: 578 ns/op
Minimal time: 578 ns/op
Maximal time: 582 ns/op
Total time: 6.371 s
Total operations: 11012340
Operations throughput: 1728452 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:8)
Average time: 778 ns/op
Minimal time: 778 ns/op
Maximal time: 795 ns/op
Total time: 6.194 s
Total operations: 7960232
Operations throughput: 1284963 ops/s
===============================================================================
```

## Benchmark 6: Asynchronous processor with file appender
Benchmark source file: [file_async.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/file_async.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
CPU logical cores: 12
CPU physical cores: 6
CPU clock speed: 2.208 GHz
CPU Hyper-Threading: enabled
RAM total: 31.757 GiB
RAM free: 21.364 GiB
===============================================================================
OS version: Microsoft Windows 8  (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 16:36:29 2019
UTC timestamp: Wed Mar 27 13:36:29 2019
===============================================================================
Benchmark: FileAsyncWait-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAsyncWait-binary(threads:1)
Average time: 101 ns/op
Minimal time: 101 ns/op
Maximal time: 156 ns/op
Total time: 4.892 s
Total operations: 48050088
Operations throughput: 9821855 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-binary(threads:2)
Average time: 350 ns/op
Minimal time: 350 ns/op
Maximal time: 567 ns/op
Total time: 2.370 s
Total operations: 6760258
Operations throughput: 2852398 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-binary(threads:4)
Average time: 229 ns/op
Minimal time: 229 ns/op
Maximal time: 261 ns/op
Total time: 6.620 s
Total operations: 28893840
Operations throughput: 4364315 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-binary(threads:8)
Average time: 379 ns/op
Minimal time: 379 ns/op
Maximal time: 440 ns/op
Total time: 5.670 s
Total operations: 14934640
Operations throughput: 2633834 ops/s
===============================================================================
Benchmark: FileAsyncWaitFree-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-binary(threads:1)
Average time: 118 ns/op
Minimal time: 118 ns/op
Maximal time: 121 ns/op
Total time: 5.661 s
Total operations: 47644122
Operations throughput: 8415222 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-binary(threads:2)
Average time: 244 ns/op
Minimal time: 244 ns/op
Maximal time: 373 ns/op
Total time: 3.382 s
Total operations: 13813918
Operations throughput: 4083771 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-binary(threads:4)
Average time: 243 ns/op
Minimal time: 243 ns/op
Maximal time: 254 ns/op
Total time: 6.496 s
Total operations: 26651640
Operations throughput: 4102283 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-binary(threads:8)
Average time: 311 ns/op
Minimal time: 311 ns/op
Maximal time: 321 ns/op
Total time: 6.205 s
Total operations: 19890512
Operations throughput: 3205152 ops/s
===============================================================================
Benchmark: FileAsyncWait-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAsyncWait-text(threads:1)
Average time: 425 ns/op
Minimal time: 425 ns/op
Maximal time: 463 ns/op
Total time: 6.564 s
Total operations: 15436114
Operations throughput: 2351408 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-text(threads:2)
Average time: 530 ns/op
Minimal time: 530 ns/op
Maximal time: 563 ns/op
Total time: 4.822 s
Total operations: 9090630
Operations throughput: 1884947 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-text(threads:4)
Average time: 540 ns/op
Minimal time: 540 ns/op
Maximal time: 559 ns/op
Total time: 6.314 s
Total operations: 11678216
Operations throughput: 1849562 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-text(threads:8)
Average time: 624 ns/op
Minimal time: 624 ns/op
Maximal time: 659 ns/op
Total time: 6.041 s
Total operations: 9676336
Operations throughput: 1601580 ops/s
===============================================================================
Benchmark: FileAsyncWaitFree-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-text(threads:1)
Average time: 438 ns/op
Minimal time: 438 ns/op
Maximal time: 454 ns/op
Total time: 6.527 s
Total operations: 14872420
Operations throughput: 2278352 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-text(threads:2)
Average time: 501 ns/op
Minimal time: 501 ns/op
Maximal time: 574 ns/op
Total time: 4.599 s
Total operations: 9168388
Operations throughput: 1993181 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-text(threads:4)
Average time: 592 ns/op
Minimal time: 592 ns/op
Maximal time: 638 ns/op
Total time: 6.052 s
Total operations: 10208464
Operations throughput: 1686616 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-text(threads:8)
Average time: 688 ns/op
Minimal time: 688 ns/op
Maximal time: 700 ns/op
Total time: 6.030 s
Total operations: 8758360
Operations throughput: 1452460 ops/s
===============================================================================

```

## Benchmark 7: Format in logging thread vs format in background thread
Benchmark source file: [async_format.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/async_format.cpp)

Benchmark report is the following:
```
===============================================================================
CppBenchmark report. Version 1.0.0.0
===============================================================================
CPU architecutre: Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz
CPU logical cores: 12
CPU physical cores: 6
CPU clock speed: 2.208 GHz
CPU Hyper-Threading: enabled
RAM total: 31.757 GiB
RAM free: 21.286 GiB
===============================================================================
OS version: Microsoft Windows 8  (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 16:43:04 2019
UTC timestamp: Wed Mar 27 13:43:04 2019
===============================================================================
Benchmark: Format(int, double, string)
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: Format(int, double, string)(threads:1)
Average time: 722 ns/op
Minimal time: 722 ns/op
Maximal time: 754 ns/op
Total time: 5.899 s
Total operations: 8162637
Operations throughput: 1383683 ops/s
-------------------------------------------------------------------------------
Phase: Format(int, double, string)(threads:2)
Average time: 406 ns/op
Minimal time: 406 ns/op
Maximal time: 428 ns/op
Total time: 5.212 s
Total operations: 12818384
Operations throughput: 2458961 ops/s
-------------------------------------------------------------------------------
Phase: Format(int, double, string)(threads:4)
Average time: 279 ns/op
Minimal time: 279 ns/op
Maximal time: 289 ns/op
Total time: 6.843 s
Total operations: 24456284
Operations throughput: 3573654 ops/s
-------------------------------------------------------------------------------
Phase: Format(int, double, string)(threads:8)
Average time: 462 ns/op
Minimal time: 462 ns/op
Maximal time: 496 ns/op
Total time: 6.494 s
Total operations: 14031432
Operations throughput: 2160412 ops/s
===============================================================================
Benchmark: Serialize(int, double, string)
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: Serialize(int, double, string)(threads:1)
Average time: 251 ns/op
Minimal time: 251 ns/op
Maximal time: 266 ns/op
Total time: 3.493 s
Total operations: 13909437
Operations throughput: 3981181 ops/s
-------------------------------------------------------------------------------
Phase: Serialize(int, double, string)(threads:2)
Average time: 353 ns/op
Minimal time: 353 ns/op
Maximal time: 474 ns/op
Total time: 3.887 s
Total operations: 11006936
Operations throughput: 2831139 ops/s
-------------------------------------------------------------------------------
Phase: Serialize(int, double, string)(threads:4).thread
Average time: 1.338 mcs/op
Minimal time: 1.338 mcs/op
Maximal time: 1.545 mcs/op
Total time: 4.916 s
Total operations: 3671994
Operations throughput: 746838 ops/s
-------------------------------------------------------------------------------
Phase: Serialize(int, double, string)(threads:8).thread
Average time: 3.266 mcs/op
Minimal time: 3.266 mcs/op
Maximal time: 4.085 mcs/op
Total time: 4.269 s
Total operations: 1307225
Operations throughput: 306147 ops/s
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
