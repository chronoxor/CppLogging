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
    * [Benchmark 7: Format in logging thread vs binary serialization without format](#benchmark-7-format-in-logging-thread-vs-binary-serialization-without-format)
    * [Benchmark 8: Binary layout vs text layout](#benchmark-8-binary-layout-vs-text-layout)
    * [Benchmark 9: Format vs store format](#benchmark-9-format-vs-store-format)
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
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 16.910 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 21:20:19 2019
UTC timestamp: Wed Mar 27 18:20:19 2019
===============================================================================
Benchmark: NullAppender-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: NullAppender-binary
Average time: 52 ns/op
Minimal time: 52 ns/op
Maximal time: 55 ns/op
Total time: 2.848 s
Total operations: 54324965
Operations throughput: 19070173 ops/s
===============================================================================
Benchmark: NullAppender-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: NullAppender-text
Average time: 214 ns/op
Minimal time: 214 ns/op
Maximal time: 225 ns/op
Total time: 4.325 s
Total operations: 20163398
Operations throughput: 4661367 ops/s
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
RAM free: 15.824 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 21:21:36 2019
UTC timestamp: Wed Mar 27 18:21:36 2019
===============================================================================
Benchmark: FileAppender-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAppender-binary
Average time: 99 ns/op
Minimal time: 99 ns/op
Maximal time: 114 ns/op
Total time: 3.490 s
Total operations: 34960790
Operations throughput: 10015343 ops/s
===============================================================================
Benchmark: FileAppender-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAppender-text
Average time: 285 ns/op
Minimal time: 285 ns/op
Maximal time: 306 ns/op
Total time: 4.266 s
Total operations: 14965162
Operations throughput: 3507742 ops/s
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
RAM free: 17.407 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 21:28:21 2019
UTC timestamp: Wed Mar 27 18:28:21 2019
===============================================================================
Benchmark: SyncProcessor-null
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:1)
Average time: 80 ns/op
Minimal time: 80 ns/op
Maximal time: 84 ns/op
Total time: 4.227 s
Total operations: 52444919
Operations throughput: 12404934 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:2)
Average time: 51 ns/op
Minimal time: 51 ns/op
Maximal time: 53 ns/op
Total time: 4.865 s
Total operations: 93864920
Operations throughput: 19291320 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:4)
Average time: 179 ns/op
Minimal time: 179 ns/op
Maximal time: 186 ns/op
Total time: 7.643 s
Total operations: 42621432
Operations throughput: 5575895 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-null(threads:8)
Average time: 228 ns/op
Minimal time: 228 ns/op
Maximal time: 231 ns/op
Total time: 6.480 s
Total operations: 28420296
Operations throughput: 4385653 ops/s
===============================================================================
Benchmark: SyncProcessor-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:1)
Average time: 89 ns/op
Minimal time: 89 ns/op
Maximal time: 92 ns/op
Total time: 4.602 s
Total operations: 51618553
Operations throughput: 11215740 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:2)
Average time: 100 ns/op
Minimal time: 100 ns/op
Maximal time: 104 ns/op
Total time: 8.152 s
Total operations: 81421390
Operations throughput: 9986857 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:4)
Average time: 217 ns/op
Minimal time: 217 ns/op
Maximal time: 221 ns/op
Total time: 6.998 s
Total operations: 32183128
Operations throughput: 4598492 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-binary(threads:8)
Average time: 278 ns/op
Minimal time: 278 ns/op
Maximal time: 281 ns/op
Total time: 6.371 s
Total operations: 22856032
Operations throughput: 3587107 ops/s
===============================================================================
Benchmark: SyncProcessor-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:1)
Average time: 283 ns/op
Minimal time: 283 ns/op
Maximal time: 303 ns/op
Total time: 5.107 s
Total operations: 18024389
Operations throughput: 3529002 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:2)
Average time: 337 ns/op
Minimal time: 337 ns/op
Maximal time: 346 ns/op
Total time: 6.441 s
Total operations: 19102240
Operations throughput: 2965400 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:4)
Average time: 451 ns/op
Minimal time: 451 ns/op
Maximal time: 472 ns/op
Total time: 5.836 s
Total operations: 12929948
Operations throughput: 2215485 ops/s
-------------------------------------------------------------------------------
Phase: SyncProcessor-text(threads:8)
Average time: 635 ns/op
Minimal time: 635 ns/op
Maximal time: 647 ns/op
Total time: 5.781 s
Total operations: 9093400
Operations throughput: 1572751 ops/s
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
RAM free: 17.461 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 21:43:05 2019
UTC timestamp: Wed Mar 27 18:43:05 2019
===============================================================================
Benchmark: AsyncWaitProcessor-null
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-null(threads:1)
Average time: 136 ns/op
Minimal time: 136 ns/op
Maximal time: 140 ns/op
Total time: 5.524 s
Total operations: 40467636
Operations throughput: 7324587 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-null(threads:2)
Average time: 91 ns/op
Minimal time: 91 ns/op
Maximal time: 93 ns/op
Total time: 5.776 s
Total operations: 63377688
Operations throughput: 10972051 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-null(threads:4)
Average time: 62 ns/op
Minimal time: 62 ns/op
Maximal time: 62 ns/op
Total time: 6.249 s
Total operations: 100633072
Operations throughput: 16101439 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-null(threads:8)
Average time: 120 ns/op
Minimal time: 120 ns/op
Maximal time: 138 ns/op
Total time: 7.087 s
Total operations: 58752032
Operations throughput: 8289066 ops/s
===============================================================================
Benchmark: AsyncWaitFreeProcessor-null
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-null(threads:1)
Average time: 136 ns/op
Minimal time: 136 ns/op
Maximal time: 140 ns/op
Total time: 5.592 s
Total operations: 40978681
Operations throughput: 7327131 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-null(threads:2)
Average time: 92 ns/op
Minimal time: 92 ns/op
Maximal time: 93 ns/op
Total time: 5.833 s
Total operations: 63027456
Operations throughput: 10804833 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-null(threads:4)
Average time: 62 ns/op
Minimal time: 62 ns/op
Maximal time: 63 ns/op
Total time: 6.205 s
Total operations: 99585424
Operations throughput: 16047167 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-null(threads:8)
Average time: 106 ns/op
Minimal time: 106 ns/op
Maximal time: 119 ns/op
Total time: 6.786 s
Total operations: 63909008
Operations throughput: 9416668 ops/s
===============================================================================
Benchmark: AsyncWaitProcessor-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-binary(threads:1)
Average time: 131 ns/op
Minimal time: 131 ns/op
Maximal time: 133 ns/op
Total time: 5.011 s
Total operations: 38131145
Operations throughput: 7608534 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-binary(threads:2)
Average time: 87 ns/op
Minimal time: 87 ns/op
Maximal time: 89 ns/op
Total time: 5.212 s
Total operations: 59811130
Operations throughput: 11474219 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-binary(threads:4)
Average time: 144 ns/op
Minimal time: 144 ns/op
Maximal time: 154 ns/op
Total time: 7.396 s
Total operations: 51026276
Operations throughput: 6898920 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-binary(threads:8)
Average time: 144 ns/op
Minimal time: 144 ns/op
Maximal time: 147 ns/op
Total time: 6.523 s
Total operations: 45218480
Operations throughput: 6931931 ops/s
===============================================================================
Benchmark: AsyncWaitFreeProcessor-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-binary(threads:1)
Average time: 136 ns/op
Minimal time: 136 ns/op
Maximal time: 139 ns/op
Total time: 5.049 s
Total operations: 37042770
Operations throughput: 7336301 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-binary(threads:2)
Average time: 86 ns/op
Minimal time: 86 ns/op
Maximal time: 90 ns/op
Total time: 5.143 s
Total operations: 59458290
Operations throughput: 11560056 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-binary(threads:4)
Average time: 141 ns/op
Minimal time: 141 ns/op
Maximal time: 150 ns/op
Total time: 7.482 s
Total operations: 52698160
Operations throughput: 7042986 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-binary(threads:8)
Average time: 142 ns/op
Minimal time: 142 ns/op
Maximal time: 150 ns/op
Total time: 6.616 s
Total operations: 46393216
Operations throughput: 7012137 ops/s
===============================================================================
Benchmark: AsyncWaitProcessor-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-text(threads:1)
Average time: 354 ns/op
Minimal time: 354 ns/op
Maximal time: 379 ns/op
Total time: 6.846 s
Total operations: 19297269
Operations throughput: 2818594 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-text(threads:2)
Average time: 408 ns/op
Minimal time: 408 ns/op
Maximal time: 438 ns/op
Total time: 6.199 s
Total operations: 15173816
Operations throughput: 2447449 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-text(threads:4)
Average time: 473 ns/op
Minimal time: 473 ns/op
Maximal time: 483 ns/op
Total time: 6.151 s
Total operations: 12987696
Operations throughput: 2111302 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitProcessor-text(threads:8)
Average time: 545 ns/op
Minimal time: 545 ns/op
Maximal time: 571 ns/op
Total time: 5.925 s
Total operations: 10872848
Operations throughput: 1834828 ops/s
===============================================================================
Benchmark: AsyncWaitFreeProcessor-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-text(threads:1)
Average time: 364 ns/op
Minimal time: 364 ns/op
Maximal time: 401 ns/op
Total time: 6.467 s
Total operations: 17724308
Operations throughput: 2740618 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-text(threads:2)
Average time: 401 ns/op
Minimal time: 401 ns/op
Maximal time: 433 ns/op
Total time: 6.078 s
Total operations: 15141962
Operations throughput: 2491024 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-text(threads:4)
Average time: 449 ns/op
Minimal time: 449 ns/op
Maximal time: 543 ns/op
Total time: 5.538 s
Total operations: 12314328
Operations throughput: 2223352 ops/s
-------------------------------------------------------------------------------
Phase: AsyncWaitFreeProcessor-text(threads:8)
Average time: 501 ns/op
Minimal time: 501 ns/op
Maximal time: 555 ns/op
Total time: 5.822 s
Total operations: 11619480
Operations throughput: 1995719 ops/s
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
RAM free: 17.227 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 23:13:37 2019
UTC timestamp: Wed Mar 27 20:13:37 2019
===============================================================================
Benchmark: FileSync-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:1)
Average time: 227 ns/op
Minimal time: 227 ns/op
Maximal time: 241 ns/op
Total time: 7.525 s
Total operations: 33111269
Operations throughput: 4399767 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:2)
Average time: 319 ns/op
Minimal time: 319 ns/op
Maximal time: 342 ns/op
Total time: 8.822 s
Total operations: 27619980
Operations throughput: 3130472 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:4)
Average time: 384 ns/op
Minimal time: 384 ns/op
Maximal time: 400 ns/op
Total time: 7.569 s
Total operations: 19670920
Operations throughput: 2598539 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-binary(threads:8)
Average time: 512 ns/op
Minimal time: 512 ns/op
Maximal time: 521 ns/op
Total time: 7.187 s
Total operations: 14028248
Operations throughput: 1951843 ops/s
===============================================================================
Benchmark: FileSync-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:1)
Average time: 510 ns/op
Minimal time: 510 ns/op
Maximal time: 547 ns/op
Total time: 7.051 s
Total operations: 13811530
Operations throughput: 1958764 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:2)
Average time: 546 ns/op
Minimal time: 546 ns/op
Maximal time: 572 ns/op
Total time: 7.555 s
Total operations: 13836406
Operations throughput: 1831251 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:4)
Average time: 682 ns/op
Minimal time: 682 ns/op
Maximal time: 716 ns/op
Total time: 7.188 s
Total operations: 10538168
Operations throughput: 1466009 ops/s
-------------------------------------------------------------------------------
Phase: FileSync-text(threads:8)
Average time: 894 ns/op
Minimal time: 894 ns/op
Maximal time: 903 ns/op
Total time: 6.956 s
Total operations: 7776856
Operations throughput: 1117940 ops/s
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
RAM free: 17.590 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 23:45:08 2019
UTC timestamp: Wed Mar 27 20:45:08 2019
===============================================================================
Benchmark: FileAsyncWait-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAsyncWait-binary(threads:1)
Average time: 103 ns/op
Minimal time: 103 ns/op
Maximal time: 125 ns/op
Total time: 4.257 s
Total operations: 40989563
Operations throughput: 9627463 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-binary(threads:2)
Average time: 126.513 mcs/op
Minimal time: 126.513 mcs/op
Maximal time: 140.340 mcs/op
Total time: 2.838 s
Total operations: 22438
Operations throughput: 7904 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-binary(threads:4)
Average time: 310 ns/op
Minimal time: 310 ns/op
Maximal time: 329 ns/op
Total time: 7.908 s
Total operations: 25465332
Operations throughput: 3219949 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-binary(threads:8)
Average time: 354 ns/op
Minimal time: 354 ns/op
Maximal time: 380 ns/op
Total time: 7.662 s
Total operations: 21622472
Operations throughput: 2821727 ops/s
===============================================================================
Benchmark: FileAsyncWaitFree-binary
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-binary(threads:1)
Average time: 107 ns/op
Minimal time: 107 ns/op
Maximal time: 126 ns/op
Total time: 4.459 s
Total operations: 41402456
Operations throughput: 9283109 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-binary(threads:2)
Average time: 135.946 mcs/op
Minimal time: 135.946 mcs/op
Maximal time: 147.538 mcs/op
Total time: 2.964 s
Total operations: 21810
Operations throughput: 7355 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-binary(threads:4)
Average time: 331 ns/op
Minimal time: 331 ns/op
Maximal time: 344 ns/op
Total time: 7.963 s
Total operations: 24025796
Operations throughput: 3016885 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-binary(threads:8)
Average time: 346 ns/op
Minimal time: 346 ns/op
Maximal time: 370 ns/op
Total time: 7.713 s
Total operations: 22267464
Operations throughput: 2886785 ops/s
===============================================================================
Benchmark: FileAsyncWait-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAsyncWait-text(threads:1)
Average time: 373 ns/op
Minimal time: 373 ns/op
Maximal time: 445 ns/op
Total time: 4.837 s
Total operations: 12964360
Operations throughput: 2679716 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-text(threads:2)
Average time: 614 ns/op
Minimal time: 614 ns/op
Maximal time: 1.399 mcs/op
Total time: 8.445 ms
Total operations: 13734
Operations throughput: 1626248 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-text(threads:4)
Average time: 705 ns/op
Minimal time: 705 ns/op
Maximal time: 727 ns/op
Total time: 7.016 s
Total operations: 9950764
Operations throughput: 1418200 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWait-text(threads:8)
Average time: 794 ns/op
Minimal time: 794 ns/op
Maximal time: 828 ns/op
Total time: 6.891 s
Total operations: 8670376
Operations throughput: 1258208 ops/s
===============================================================================
Benchmark: FileAsyncWaitFree-text
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-text(threads:1)
Average time: 384 ns/op
Minimal time: 384 ns/op
Maximal time: 414 ns/op
Total time: 5.349 s
Total operations: 13925080
Operations throughput: 2602936 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-text(threads:2)
Average time: 727 ns/op
Minimal time: 727 ns/op
Maximal time: 1.065 mcs/op
Total time: 12.221 ms
Total operations: 16797
Operations throughput: 1374399 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-text(threads:4)
Average time: 765 ns/op
Minimal time: 765 ns/op
Maximal time: 875 ns/op
Total time: 6.467 s
Total operations: 8445064
Operations throughput: 1305736 ops/s
-------------------------------------------------------------------------------
Phase: FileAsyncWaitFree-text(threads:8)
Average time: 777 ns/op
Minimal time: 777 ns/op
Maximal time: 824 ns/op
Total time: 6.834 s
Total operations: 8791624
Operations throughput: 1286393 ops/s
===============================================================================
```

## Benchmark 7: Format in logging thread vs binary serialization without format
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
RAM free: 17.790 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Wed Mar 27 23:54:02 2019
UTC timestamp: Wed Mar 27 20:54:02 2019
===============================================================================
Benchmark: Format(int, double, string)
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: Format(int, double, string)(threads:1)
Average time: 432 ns/op
Minimal time: 432 ns/op
Maximal time: 451 ns/op
Total time: 4.508 s
Total operations: 10415618
Operations throughput: 2310166 ops/s
-------------------------------------------------------------------------------
Phase: Format(int, double, string)(threads:2)
Average time: 371 ns/op
Minimal time: 371 ns/op
Maximal time: 509 ns/op
Total time: 7.954 ms
Total operations: 21389
Operations throughput: 2688944 ops/s
-------------------------------------------------------------------------------
Phase: Format(int, double, string)(threads:4)
Average time: 321 ns/op
Minimal time: 321 ns/op
Maximal time: 340 ns/op
Total time: 8.972 s
Total operations: 27908800
Operations throughput: 3110554 ops/s
-------------------------------------------------------------------------------
Phase: Format(int, double, string)(threads:8)
Average time: 391 ns/op
Minimal time: 391 ns/op
Maximal time: 431 ns/op
Total time: 8.539 s
Total operations: 21836936
Operations throughput: 2557197 ops/s
===============================================================================
Benchmark: Serialize(int, double, string)
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: Serialize(int, double, string)(threads:1)
Average time: 137 ns/op
Minimal time: 137 ns/op
Maximal time: 159 ns/op
Total time: 4.004 s
Total operations: 29199876
Operations throughput: 7291852 ops/s
-------------------------------------------------------------------------------
Phase: Serialize(int, double, string)(threads:2)
Average time: 173.561 mcs/op
Minimal time: 173.561 mcs/op
Maximal time: 193.575 mcs/op
Total time: 3.322 s
Total operations: 19142
Operations throughput: 5761 ops/s
-------------------------------------------------------------------------------
Phase: Serialize(int, double, string)(threads:4)
Average time: 473 ns/op
Minimal time: 473 ns/op
Maximal time: 498 ns/op
Total time: 8.683 s
Total operations: 18328828
Operations throughput: 2110848 ops/s
-------------------------------------------------------------------------------
Phase: Serialize(int, double, string)(threads:8)
Average time: 487 ns/op
Minimal time: 487 ns/op
Maximal time: 522 ns/op
Total time: 8.415 s
Total operations: 17251000
Operations throughput: 2049912 ops/s
===============================================================================
```

## Benchmark 8: Binary layout vs text layout
Benchmark source file: [layout.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/layout.cpp)

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
RAM free: 17.160 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Thu Mar 28 00:20:39 2019
UTC timestamp: Wed Mar 27 21:20:39 2019
===============================================================================
Benchmark: BinaryLayout
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: BinaryLayout
Average time: 24 ns/op
Minimal time: 24 ns/op
Maximal time: 25 ns/op
Total time: 2.280 s
Total operations: 91409302
Total bytes: 4.437 GiB
Operations throughput: 40091059 ops/s
Bytes throughput: 1.964 GiB/s
===============================================================================
Benchmark: TextLayout
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: TextLayout
Average time: 179 ns/op
Minimal time: 179 ns/op
Maximal time: 185 ns/op
Total time: 4.090 s
Total operations: 22754768
Total bytes: 1.560 GiB
Operations throughput: 5562485 ops/s
Bytes throughput: 387.256 MiB/s
===============================================================================
```

## Benchmark 9: Format vs store format
Benchmark source file: [format.cpp](https://github.com/chronoxor/CppLogging/blob/master/performance/format.cpp)

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
RAM free: 17.694 GiB
===============================================================================
OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
Local timestamp: Thu Mar 28 00:09:59 2019
UTC timestamp: Wed Mar 27 21:09:59 2019
===============================================================================
Benchmark: Format(int)
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: Format(int)
Average time: 129 ns/op
Minimal time: 129 ns/op
Maximal time: 138 ns/op
Total time: 3.899 s
Total operations: 30121235
Operations throughput: 7724549 ops/s
===============================================================================
Benchmark: StoreFormat(int)
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: StoreFormat(int)
Average time: 18 ns/op
Minimal time: 18 ns/op
Maximal time: 19 ns/op
Total time: 1.726 s
Total operations: 94928448
Operations throughput: 54976298 ops/s
===============================================================================
Benchmark: Format(int, double, string)
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: Format(int, double, string)
Average time: 319 ns/op
Minimal time: 319 ns/op
Maximal time: 334 ns/op
Total time: 4.491 s
Total operations: 14047637
Operations throughput: 3127578 ops/s
===============================================================================
Benchmark: StoreFormat(int, double, string)
Attempts: 5
Duration: 5 seconds
-------------------------------------------------------------------------------
Phase: StoreFormat(int, double, string)
Average time: 33 ns/op
Minimal time: 33 ns/op
Maximal time: 35 ns/op
Total time: 2.485 s
Total operations: 73499935
Operations throughput: 29570995 ops/s
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
