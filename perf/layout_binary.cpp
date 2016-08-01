//
// Created by Ivan Shynkarenka on 11.07.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/layouts/binary_layout.h"

#include <cstring>

using namespace CppLogging;

const int iterations = 10000000;

char logger[] = "Test logger";
char message[] = "Test message";
uint8_t buffer[1024];

class BinaryLayoutFixture
{
protected:
    BinaryLayout layout;

    uint8_t slogger;
    uint16_t smessage;
    uint16_t sbuffer;

    BinaryLayoutFixture()
    {
        slogger = (uint8_t)std::strlen(logger);
        smessage = (uint16_t)std::strlen(message);
        sbuffer = (uint32_t)sizeof(buffer);
    }
};

BENCHMARK_FIXTURE(BinaryLayoutFixture, "BinaryLayout", iterations)
{
    Record record;
    record.logger = std::make_pair(logger, slogger);
    record.message = std::make_pair(message, smessage);
    record.buffer = std::make_pair(buffer, sbuffer);

    auto result = layout.LayoutRecord(record);
    context.metrics().AddBytes(result.second);
}

BENCHMARK_MAIN()
