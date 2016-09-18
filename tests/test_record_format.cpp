//
// Created by Ivan Shynkarenka on 18.09.2016.
//

#include "catch.hpp"

#include "logging/record.h"

using namespace CppLogging;

std::string format(const char* pattern, fmt::ArgList args)
{
    Record record;
    record.InitFormat(pattern, args);
    record.FormatMessage();
    return record.message;
}

FMT_VARIADIC(std::string, format, const char*);

TEST_CASE("Format message", "[CppLogging]")
{
    REQUIRE(format("{:+f}; {:+f}", 3.14, -3.14) == "+3.140000; -3.140000");
    REQUIRE(format("{: f}; {: f}", 3.14, -3.14) == " 3.140000; -3.140000");
    REQUIRE(format("{:-f}; {:-f}", 3.14, -3.14) == "3.140000; -3.140000");
    REQUIRE(format("int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42) == "int: 42;  hex: 2a;  oct: 52; bin: 101010");
    REQUIRE(format("int: {0:d};  hex: {0:#x};  oct: {0:#o};  bin: {0:#b}", 42) == "int: 42;  hex: 0x2a;  oct: 052;  bin: 0b101010");
}
