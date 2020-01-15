//
// Created by Ivan Shynkarenka on 18.09.2016
//

#include "test.h"

#include "logging/record.h"

using namespace CppLogging;

namespace {

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

template <typename... Args>
std::string format(std::string_view pattern, const Args&... args)
{
    Record record;
    record.Format(pattern, args...);
    return record.message;
}

template <typename... Args>
std::string store(std::string_view pattern, const Args&... args)
{
    Record record;
    record.StoreFormat(pattern, args...);
    record.message = record.RestoreFormat();
    return record.message;
}

} // namespace

TEST_CASE("Format message", "[CppLogging]")
{
    REQUIRE(format("no arguments") == "no arguments");
    REQUIRE(format("{0}, {1}, {2}", -1, 0, 1) == "-1, 0, 1");
    REQUIRE(format("{0}, {1}, {2}", 'a', 'b', 'c') == "a, b, c");
    REQUIRE(format("{}, {}, {}", 'a', 'b', 'c') == "a, b, c");
    REQUIRE(format("{2}, {1}, {0}", 'a', 'b', 'c') == "c, b, a");
    REQUIRE(format("{0}{1}{0}", "abra", "cad") == "abracadabra");
    REQUIRE(format("{:<30}", "left aligned") == "left aligned                  ");
    REQUIRE(format("{:>30}", "right aligned") == "                 right aligned");
    REQUIRE(format("{:^30}", "centered") == "           centered           ");
    REQUIRE(format("{:*^30}", "centered") == "***********centered***********");
    REQUIRE(format("{:+f}; {:+f}", 3.14, -3.14) == "+3.140000; -3.140000");
    REQUIRE(format("{: f}; {: f}", 3.14, -3.14) == " 3.140000; -3.140000");
    REQUIRE(format("{:-f}; {:-f}", 3.14, -3.14) == "3.140000; -3.140000");
    REQUIRE(format("int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42) == "int: 42;  hex: 2a;  oct: 52; bin: 101010");
    REQUIRE(format("int: {0:d};  hex: {0:#x};  oct: {0:#o};  bin: {0:#b}", 42) == "int: 42;  hex: 0x2a;  oct: 052;  bin: 0b101010");
    REQUIRE(format("The date is {}", Date(2012, 12, 9)) == "The date is 2012-12-9");
    REQUIRE(format("The datetime is {}", DateTime(Date(2012, 12, 9), 13, 15, 57)) == "The datetime is 2012-12-9 13:15:57");
    REQUIRE(format("Elapsed time: {s:.2f} seconds", "s"_a = 1.23) == "Elapsed time: 1.23 seconds");
    REQUIRE(format("The answer is {}"_format(42)) == "The answer is 42");
}

TEST_CASE("Store message", "[CppLogging]")
{
    REQUIRE(store("no arguments") == "no arguments");
    REQUIRE(store("{0}, {1}, {2}", -1, 0, 1) == "-1, 0, 1");
    REQUIRE(store("{0}, {1}, {2}", 'a', 'b', 'c') == "a, b, c");
    REQUIRE(store("{}, {}, {}", 'a', 'b', 'c') == "a, b, c");
    REQUIRE(store("{2}, {1}, {0}", 'a', 'b', 'c') == "c, b, a");
    REQUIRE(store("{0}{1}{0}", "abra", "cad") == "abracadabra");
    REQUIRE(store("{:<30}", "left aligned") == "left aligned                  ");
    REQUIRE(store("{:>30}", "right aligned") == "                 right aligned");
    REQUIRE(store("{:^30}", "centered") == "           centered           ");
    REQUIRE(store("{:*^30}", "centered") == "***********centered***********");
    REQUIRE(store("{:+f}; {:+f}", 3.14, -3.14) == "+3.140000; -3.140000");
    REQUIRE(store("{: f}; {: f}", 3.14, -3.14) == " 3.140000; -3.140000");
    REQUIRE(store("{:-f}; {:-f}", 3.14, -3.14) == "3.140000; -3.140000");
    REQUIRE(store("int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42) == "int: 42;  hex: 2a;  oct: 52; bin: 101010");
    REQUIRE(store("int: {0:d};  hex: {0:#x};  oct: {0:#o};  bin: {0:#b}", 42) == "int: 42;  hex: 0x2a;  oct: 052;  bin: 0b101010");
    REQUIRE(store("The date is {}", Date(2012, 12, 9)) == "The date is 2012-12-9");
    REQUIRE(store("The datetime is {}", DateTime(Date(2012, 12, 9), 13, 15, 57)) == "The datetime is 2012-12-9 13:15:57");
    REQUIRE(store("Elapsed time: {s:.2f} seconds", "s"_a = 1.23) == "Elapsed time: 1.23 seconds");
    REQUIRE(store("The answer is {}"_format(42)) == "The answer is 42");
}
