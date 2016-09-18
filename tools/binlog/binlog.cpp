/*!
    \file binlog.cpp
    \brief Binary logs reader definition
    \author Ivan Shynkarenka
    \date 16.09.2015
    \copyright MIT License
*/

#include <stdio.h>

#include "version.h"
#include "logging/record.h"
#include "logging/layouts/text_layout.h"

#include <cstring>
#include <iostream>
#include <vector>

#include <OptionParser.h>

using namespace CppLogging;

bool InputRecord(FILE* input, Record& record)
{
    // Clear the logging record
    record.Clear();

    // Read the logging record size
    uint32_t size;
    if (fread(&size, sizeof(uint32_t), 1, input) != 1)
        return false;

    record.raw.resize(size);

    // Read the logging record raw data
    if (fread(record.raw.data(), 1, size, input) != size)
    {
        std::cerr << "Failed to read from the input file!" << std::endl;
        return false;
    }

    // Get the buffer start position
    const uint8_t* buffer = record.raw.data();

    // Deserialize logging record
    std::memcpy(&record.timestamp, buffer, sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(&record.thread, buffer, sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(&record.level, buffer, sizeof(Level));
    buffer += sizeof(Level);

    uint8_t logger_size;
    std::memcpy(&logger_size, buffer, sizeof(uint8_t));
    buffer += sizeof(uint8_t);
    record.logger.insert(record.logger.begin(), buffer, buffer + logger_size);
    buffer += logger_size;

    uint16_t message_size;
    std::memcpy(&message_size, buffer, sizeof(uint16_t));
    buffer += sizeof(uint16_t);
    record.message.insert(record.message.begin(), buffer, buffer + message_size);
    buffer += message_size;

    uint32_t buffer_size;
    std::memcpy(&buffer_size, buffer, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    record.buffer.insert(record.buffer.begin(), buffer, buffer + buffer_size);
    buffer += buffer_size;

    // Skip the last zero byte
    ++buffer;

    return true;
}

bool OutputRecord(FILE* output, Record& record)
{
    TextLayout layout;
    layout.LayoutRecord(record);

    if (fwrite(record.raw.data(), 1, record.raw.size() - 1, output) != (record.raw.size() - 1))
    {
        std::cerr << "Failed to write into the output file!" << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version(version);

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-i", "--input").help("Input file name");
    parser.add_option("-o", "--output").help("Output file name");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    // Open the input file or stdin
    FILE* input_file = options.is_set("input") ? fopen(options.get("input"), "rb") : stdin;
    if (input_file == nullptr)
    {
        std::cerr << "Failed to open the input file!" << std::endl;
        return -1;
    }

    // Open the output file or stdout
    FILE* output_file = options.is_set("output") ? fopen(options.get("input"), "wb") : stdout;
    if (output_file == nullptr)
    {
        std::cerr << "Failed to open the output file!" << std::endl;
        return -1;
    }

    // Process all logging record
    Record record;
    while (InputRecord(input_file, record))
        if (!OutputRecord(output_file, record))
            break;

    // Close the input file
    if (options.is_set("input"))
    {
        if (fclose(input_file) != 0)
        {
            std::cerr << "Failed to close the input file!" << std::endl;
            return -1;
        }
    }

    // Close the output file
    if (options.is_set("output"))
    {
        if (fclose(output_file) != 0)
        {
            std::cerr << "Failed to close the output file!" << std::endl;
            return -1;
        }
    }

    return 0;
}
