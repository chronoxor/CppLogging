//
// Created by Ivan Shynkarenka on 22.07.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/layouts/text_layout.h"

#include <cstring>

using namespace CppLogging;

const int iterations = 10000000;

char logger[] = "Test logger";
char message[] = "Test message";
uint8_t buffer[1024];

class LayoutFixture
{
protected:
    TextLayout layout;
    Record record;

    LayoutFixture()
    {
        record.logger = std::make_pair(logger, (uint8_t)std::strlen(logger));
        record.message = std::make_pair(message, (uint16_t)std::strlen(message));
        record.buffer = std::make_pair(buffer, (uint32_t)sizeof(buffer));
    }
};

BENCHMARK_FIXTURE(LayoutFixture, "TextLayout", iterations)
{
    auto result = layout.LayoutRecord(record);
    context.metrics().AddBytes(result.second);
}

BENCHMARK_MAIN()
