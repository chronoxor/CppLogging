# CppLogging
C++ Logging Library provides functionality to log different events with a high
throughput in multi-thread environment into different sinks (console, files,
rolling files, syslog, etc.). Logging configuration is very flexible and gives
functionality to build flexible logger hierarchy with combination of logging
processors (sync, async), filters, layouts (binary, text) and appenders.

[CppLogging API reference](http://chronoxor.github.io/CppLogging/index.html)

[![Build status](https://travis-ci.org/chronoxor/CppLogging.svg?branch=master)](https://travis-ci.org/chronoxor/CppLogging)
[![Build status](https://ci.appveyor.com/api/projects/status/91j5tree7gm7awmd?svg=true)](https://ci.appveyor.com/project/chronoxor/CppLogging)

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
    * [Clone repository with submodules](#clone-repository-with-submodules)
    * [Windows (Visaul Studio 2015)](#windows-visaul-studio-2015)
    * [Windows (MinGW with MSYS)](#windows-mingw-with-msys)
    * [Linux](#linux)

# Features
* Binary & text layouts
* Synchronous logging
* Asynchronous logging
* Appenders collection (null, memory, console, file, rolling file, ostream, syslog)
* Logging levels (debug, info, warning, error, fatal)
* Logging filters (by level, by logger name, by message pattern)
* Format logging records using [{fmt} library](http://fmtlib.net)
* Flexible configuration and logger processing hierarchy

# Requirements
* Windows 7 / Windows 10
* Linux
* [CMake](http://www.cmake.org/download/)
* [GIT](https://git-scm.com/)
* [GCC](https://gcc.gnu.org/)

Optional:
* [Visual Studio 2015](https://www.visualstudio.com/)
* [Clion](https://www.jetbrains.com/clion/)
* [MinGW](http://mingw-w64.org/doku.php)
* [MSYS](http://www.mingw.org/wiki/msys)

#How to build?

## Clone repository with submodules
```
git clone https://github.com/chronoxor/CppLogging.git CppLogging
cd CppLogging
git submodule update --init --recursive --remote
```

## Windows (Visaul Studio 2015)
```
cd scripts
01-generate-VisualStudio.bat
02-build-VisualStudio.bat
03-tests.bat
04-install-VisualStudio.bat
05-doxygen-VisualStudio.bat
```

## Windows (MinGW with MSYS)
```
cd scripts
01-generate-MSYS.bat
02-build-MSYS.bat
03-tests.bat
04-install-MSYS.bat
05-doxygen-MSYS.bat
```

## Linux
```
cd scripts
./01-generate-Unix.sh
./02-build-Unix.sh
./03-tests.sh
./04-install-Unix.sh
./05-doxygen-Unix.sh
```
