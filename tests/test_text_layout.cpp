//
// Created by Ivan Shynkarenka on 22.07.2016.
//

#include "catch.hpp"

#include "logging/layouts/text_layout.h"

#include <cstring>

using namespace CppLogging;

TEST_CASE("Text layout", "[CppLogging]")
{
    char logger[] = "Test logger";
    char message[] = "Test message";
    uint8_t buffer[1024];

    Record record;
    record.logger = std::make_pair(logger, (uint8_t)std::strlen(logger));
    record.message = std::make_pair(message, (uint16_t)std::strlen(message));
    record.buffer = std::make_pair(buffer, (uint32_t)sizeof(buffer));

    TextLayout layout;
    auto result = layout.LayoutRecord(record);
    REQUIRE(result.first != nullptr);
    REQUIRE(result.second > 0);
}
