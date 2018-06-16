//
// Created by Ivan Shynkarenka on 28.09.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/record.h"

using namespace CppLogging;

const uint64_t operations = 10000000;

BENCHMARK("Format(int)", operations)
{
    static Record record;
    record.Format("test {} test", context.metrics().total_operations());
}

BENCHMARK("Format(int, double, string)", operations)
{
    static Record record;
    record.Format("test {}.{}.{} test", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_MAIN()
