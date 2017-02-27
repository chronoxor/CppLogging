//
// Created by Ivan Shynkarenka on 28.09.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/record.h"

using namespace CppLogging;

const uint64_t iterations = 10000000;

BENCHMARK("Format(int)-serialize", iterations)
{
    static Record record;

    record.FormatSerialize("test {} test", context.metrics().total_iterations());
}

BENCHMARK("Format(int)-full", iterations)
{
    static Record record;

    record.FormatSerialize("test {} test", context.metrics().total_iterations());
    record.FormatDeserialize();
}

BENCHMARK("Format(int, double, string)-serialize", iterations)
{
    static Record record;

    record.FormatSerialize("test {}.{}.{} test", context.metrics().total_iterations(), context.metrics().total_iterations() / 1000.0, context.name());
}

BENCHMARK("Format(int, double, string)-full", iterations)
{
    static Record record;

    record.FormatSerialize("test {}.{}.{} test", context.metrics().total_iterations(), context.metrics().total_iterations() / 1000.0, context.name());
    record.FormatDeserialize();
}

BENCHMARK_MAIN()
