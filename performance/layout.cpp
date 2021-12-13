//
// Created by Ivan Shynkarenka on 11.07.2016
//

#include "benchmark/cppbenchmark.h"

#include "logging/layouts/binary_layout.h"
#include "logging/layouts/hash_layout.h"
#include "logging/layouts/text_layout.h"

using namespace CppLogging;

BENCHMARK("BinaryLayout")
{
    static BinaryLayout layout;
    static Record record;

    record.Clear();
    record.logger = "Test logger";
    record.StoreFormat("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, "bin");

    layout.LayoutRecord(record);
    context.metrics().AddBytes(record.raw.size());
}

BENCHMARK("HashLayout")
{
    static HashLayout layout;
    static Record record;

    record.Clear();
    record.logger = "Test logger";
    record.StoreFormat("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, "bin");

    layout.LayoutRecord(record);
    context.metrics().AddBytes(record.raw.size());
}

BENCHMARK("TextLayout")
{
    static TextLayout layout;
    static Record record;

    record.Clear();
    record.logger = "Test logger";
    record.StoreFormat("Test {}.{}.{} message", context.metrics().total_operations(), context.metrics().total_operations() / 1000.0, "txt");

    layout.LayoutRecord(record);
    context.metrics().AddBytes(record.raw.size());
}

BENCHMARK_MAIN()
