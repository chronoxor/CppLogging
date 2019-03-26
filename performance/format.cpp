//
// Created by Ivan Shynkarenka on 28.09.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/record.h"

using namespace CppLogging;

BENCHMARK("Format(int)")
{
    static Record record;
    record.Clear();
    record.Format("test {} test", context.metrics().total_operations());
}

BENCHMARK("StoreFormat(int)")
{
    static Record record;
    record.Clear();
    record.StoreFormat("test {} test", context.metrics().total_operations());
}

BENCHMARK("Format(int, double, string)")
{
    static Record record;
    record.Clear();
    record.Format("test {}.{}.{} test", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK("StoreFormat(int, double, string)")
{
    static Record record;
    record.Clear();
    record.StoreFormat("test {}.{}.{} test", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, context.name());
}

BENCHMARK_MAIN()
