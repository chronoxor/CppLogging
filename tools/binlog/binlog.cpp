/*!
    \file binlog.cpp
    \brief Binary logs reader definition
    \author Ivan Shynkarenka
    \date 16.09.2015
    \copyright MIT License
*/

#include "logging/record.h"
#include "logging/layouts/text_layout.h"
#include "logging/version.h"

#include "errors/fatal.h"
#include "filesystem/file.h"
#include "system/stream.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include <OptionParser.h>

using namespace CppCommon;
using namespace CppLogging;

bool InputRecord(Reader& input, Record& record)
{
    // Clear the logging record
    record.Clear();

    // Read the logging record size
    uint32_t size;
    if (input.Read(&size, sizeof(uint32_t)) != sizeof(uint32_t))
        return false;

    record.raw.resize(size);

    // Read the logging record raw data
    if (input.Read(record.raw.data(), size) != size)
    {
        std::cerr << "Failed to read from the input source!" << std::endl;
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

bool OutputRecord(Writer& output, Record& record)
{
    TextLayout layout;
    layout.LayoutRecord(record);

    size_t size = record.raw.size() - 1;
    if (output.Write(record.raw.data(), size) != size)
    {
        std::cerr << "Failed to write into the output source!" << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version(version);

    parser.add_option("-i", "--input").dest("input").help("Input file name");
    parser.add_option("-o", "--output").dest("output").help("Output file name");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        return 0;
    }

    try
    {
        // Open the input file or stdin
        std::unique_ptr<Reader> input(new StdInput());
        if (options.is_set("input"))
        {
            File* file = new File(Path(options.get("input")));
            file->Open(true, false);
            input.reset(file);
        }

        // Open the output file or stdout
        std::unique_ptr<Writer> output(new StdOutput());
        if (options.is_set("output"))
        {
            File* file = new File(Path(options.get("output")));
            file->Open(false, true);
            output.reset(file);
        }

        // Process all logging record
        Record record;
        while (InputRecord(*input, record))
            if (!OutputRecord(*output, record))
                break;

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }
}
