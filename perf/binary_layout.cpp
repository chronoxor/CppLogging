//
// Created by Ivan Shynkarenka on 11.07.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/layouts/binary_layout.h"

using namespace CppLogging;

const int iterations = 10000000;

char logger[] = "Test logger";
char message[] = "Test message";
uint8_t buffer[1024];

class LayoutFixture
{
public:
    LayoutFixture()
    {
        record.logger = std::make_pair(logger, (uint8_t)std::strlen(logger));
        record.message = std::make_pair(message, (uint16_t)std::strlen(message));
        record.buffer = std::make_pair(buffer, (uint32_t)sizeof(buffer));
    }

protected:
    BinaryLayout layout;
    Record record;
};

BENCHMARK_FIXTURE(LayoutFixture, "BinaryLayout", iterations)
{
    auto result = layout.LayoutRecord(record);
    context.metrics().AddBytes(result.second);
}

BENCHMARK_MAIN()
