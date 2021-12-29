/*!
    \file hashlog.cpp
    \brief Hash logs reader definition
    \author Ivan Shynkarenka
    \date 13.12.2021
    \copyright MIT License
*/

#include "logging/record.h"
#include "logging/layouts/hash_layout.h"
#include "logging/layouts/text_layout.h"
#include "logging/version.h"

#include "../source/logging/appenders/minizip/unzip.h"
#if defined(_WIN32) || defined(_WIN64)
#include "../source/logging/appenders/minizip/iowin32.h"
#endif

#include "errors/fatal.h"
#include "filesystem/file.h"
#include "system/stream.h"
#include "utility/countof.h"
#include "utility/resource.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include <OptionParser.h>

using namespace CppCommon;
using namespace CppLogging;

Path FindHashlog(const Path& path)
{
    // Try to find .hashlog in the current path
    Path hashlog = path.IsRegularFile() ? path : (path / ".hashlog");
    if (hashlog.IsExists())
        return hashlog;

    // Try to find .hashlog in the parent path
    Path parent = path.parent();
    if (parent)
        return FindHashlog(parent);

    // Cannot find .hashlog file
    return Path();
}

std::unordered_map<uint32_t, std::string> ReadHashlog(const Path& path)
{
    std::unordered_map<uint32_t, std::string> hashmap;

    File hashlog(path);

    // Check if .hashlog is exists
    if (!hashlog.IsExists())
        return hashmap;

    // Open .hashlog file
    hashlog.Open(true, false);

    // Check if .hashlog is opened
    if (!hashlog)
        return hashmap;

    // Read the hash map size
    uint32_t size;
    if (hashlog.Read(&size, sizeof(uint32_t)) != sizeof(uint32_t))
        return hashmap;

    hashmap.reserve(size);

    // Read the hash map content
    while (size-- > 0)
    {
        uint32_t hash;
        if (hashlog.Read(&hash, sizeof(uint32_t)) != sizeof(uint32_t))
            return hashmap;
        uint16_t length;
        if (hashlog.Read(&length, sizeof(uint16_t)) != sizeof(uint16_t))
            return hashmap;
        std::vector<uint8_t> buffer(length);
        if (hashlog.Read(buffer.data(), length) != length)
            return hashmap;
        std::string message(buffer.begin(), buffer.end());
        hashmap[hash] = message;
    }

    // Close .hashlog file
    hashlog.Close();

    return hashmap;
}

bool WriteHashlog(const Path& path, const std::unordered_map<uint32_t, std::string>& hashmap)
{
    File hashlog(path);

    // Open or create .hashlog file
    hashlog.OpenOrCreate(false, true, true);

    // Check if .hashlog is avaliable
    if (!hashlog)
        return false;

    // Write the hash map size
    uint32_t size = (uint32_t)hashmap.size();
    if (hashlog.Write(&size, sizeof(uint32_t)) != sizeof(uint32_t))
        return false;

    // Read the hash map content
    for (const auto& item : hashmap)
    {
        uint32_t hash = (uint32_t)item.first;
        if (hashlog.Write(&hash, sizeof(uint32_t)) != sizeof(uint32_t))
            return false;
        uint16_t length = (uint16_t)item.second.size();
        if (hashlog.Write(&length, sizeof(uint16_t)) != sizeof(uint16_t))
            return false;
        auto buffer = item.second.data();
        if (hashlog.Write(buffer, length) != length)
            return false;
    }

    // Close .hashlog file
    hashlog.Close();

    return true;
}

Path UnzipFile(const Path& path)
{
    // Open a zip archive
    unzFile unzf;
#if defined(_WIN32) || defined(_WIN64)
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64W(&ffunc);
    unzf = unzOpen2_64(path.wstring().c_str(), &ffunc);
#else
    unzf = unzOpen64(path.string().c_str());
#endif
    if (unzf == nullptr)
        throwex FileSystemException("Cannot open a zip archive!").Attach(path);

    // Smart resource cleaner pattern
    auto unzip = resource(unzf, [](unzFile handle) { unzClose(handle); });

    File destination(path + ".tmp");

    // Open the destination file for writing
    destination.Create(false, true);

    // Get info about the zip archive
    unz_global_info global_info;
    int result = unzGetGlobalInfo(unzf, &global_info);
    if (result != UNZ_OK)
        throwex FileSystemException("Cannot read a zip archive global info!").Attach(path);

    // Loop to extract all files from the zip archive
    uLong i;
    for (i = 0; i < global_info.number_entry; ++i)
    {
        unz_file_info file_info;
        char filename[1024];

        // Get info about the current file in the zip archive
        result = unzGetCurrentFileInfo(unzf, &file_info, filename, (unsigned)countof(filename), NULL, 0, NULL, 0);
        if (result != UNZ_OK)
            throwex FileSystemException("Cannot read a zip archive file info!").Attach(path);

        // Check if this entry is a file
        const size_t filename_length = strlen(filename);
        if (filename[filename_length - 1] != '/')
        {
            // Open the current file in the zip archive
            result = unzOpenCurrentFile(unzf);
            if (result != UNZ_OK)
                throwex FileSystemException("Cannot open a current file in the zip archive!").Attach(path);

            // Smart resource cleaner pattern
            auto unzip_file = resource(unzf, [](unzFile handle) { unzCloseCurrentFile(handle); });

            // Read data from the current file in the zip archive
            do
            {
                uint8_t buffer[16384];
                result = unzReadCurrentFile(unzf, buffer, (unsigned)countof(buffer));
                if (result > 0)
                    destination.Write(buffer, (size_t)result);
                else if (result < 0)
                    throwex FileSystemException("Cannot read the current file from the zip archive!").Attach(path);
            } while (result != UNZ_EOF);

            // Close the current file in the zip archive
            result = unzCloseCurrentFile(unzf);
            if (result != UNZ_OK)
                throwex FileSystemException("Cannot close the current file in the zip archive!").Attach(path);
            unzip_file.release();
        }
    }

    // Close the destination file
    destination.Close();

    // Close the zip archive
    result = unzClose(unzf);
    if (result != UNZ_OK)
        throwex FileSystemException("Cannot close the zip archive!").Attach(path);
    unzip.release();

    return std::move(destination);
}

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

    // Deserialize the logger name
    uint8_t logger_size;
    std::memcpy(&logger_size, buffer, sizeof(uint8_t));
    buffer += sizeof(uint8_t);
    record.logger.insert(record.logger.begin(), buffer, buffer + logger_size);
    buffer += logger_size;

    // Deserialize the logging message
    uint16_t message_size;
    std::memcpy(&message_size, buffer, sizeof(uint16_t));
    buffer += sizeof(uint16_t);
    record.message.insert(record.message.begin(), buffer, buffer + message_size);
    buffer += message_size;

    // Deserialize the logging buffer
    uint32_t buffer_size;
    std::memcpy(&buffer_size, buffer, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    record.buffer.insert(record.buffer.begin(), buffer, buffer + buffer_size);
    buffer += buffer_size;

    // Skip the last zero byte
    ++buffer;

    return true;
}

bool InputRecord(Reader& input, Record& record, const std::unordered_map<uint32_t, std::string>& hashmap)
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

    // Deserialize the logger name
    uint32_t logger_hash;
    std::memcpy(&logger_hash, buffer, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    const auto& logger = hashmap.find(logger_hash);
    record.logger.assign((logger != hashmap.end()) ? logger->second : format("0x{:X}",  logger_hash));

    // Deserialize the logging message
    uint32_t message_hash;
    std::memcpy(&message_hash, buffer, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    const auto& message = hashmap.find(message_hash);
    record.message.assign((message != hashmap.end()) ? message->second : format("0x{:X}",  message_hash));

    // Deserialize the logging buffer
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

bool UpdateHashmap(std::unordered_map<uint32_t, std::string>& hashmap, std::string_view message, uint32_t message_hash)
{
    if (hashmap.find(message_hash) == hashmap.end())
    {
        std::cout << format("Discovered logging message: \"{}\" with hash = 0x{:8X}", message, message_hash) << std::endl;
        hashmap[message_hash] = message;
        return true;
    }
    else if (message != hashmap[message_hash])
    {
        std::cerr << format("Collision detected!") << std::endl;
        std::cerr << format("Previous logging message: \"{}\" with hash = 0x{:8X}", hashmap[message_hash], message_hash) << std::endl;
        std::cerr << format("Conflict logging message: \"{}\" with hash = 0x{:8X}", message, message_hash) << std::endl;
        throwex Exception("Collision detected!");
    }

    // Skip duplicates
    return false;
}

bool UpdateHashmap(std::unordered_map<uint32_t, std::string>& hashmap, Record& record)
{
    bool result = false;

    // Check the logger name
    result |= UpdateHashmap(hashmap, record.logger, HashLayout::Hash(record.logger));
    // Check the logging message
    result |= UpdateHashmap(hashmap, record.message, HashLayout::Hash(record.message));

    return result;
}

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version(version);

    parser.add_option("-x", "--hashlog").dest("hashlog").help("Hashlog file name");
    parser.add_option("-i", "--input").dest("input").help("Input file name");
    parser.add_option("-o", "--output").dest("output").help("Output file name");
    parser.add_option("-u", "--update").dest("update").help("Update .hashlog");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        return 0;
    }

    try
    {
        // Open the hashlog file
        Path hashlog = FindHashlog(Path::current());
        if (options.is_set("hashlog"))
        {
            hashlog = Path(options.get("hashlog"));
            if (hashlog.IsDirectory())
                hashlog = FindHashlog(hashlog);
        }

        // Read .hashlog file and fill the logging messages hash map
        std::unordered_map<uint32_t, std::string> hashmap = ReadHashlog(hashlog);

        Path temp_file;

        // Open the input file or stdin
        std::unique_ptr<Reader> input(new StdInput());
        if (options.is_set("input") || options.is_set("update"))
        {
            Path path(options.is_set("input") ? options.get("input") : options.get("update"));
            if (path.IsRegularFile() && (path.extension() == ".zip"))
                temp_file = path = UnzipFile(path);
            File* file = new File(path);
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

        if (options.is_set("update"))
        {
            bool store = false;

            // Update hashmap with data from all logging records
            Record record;
            while (InputRecord(*input, record))
                store |= UpdateHashmap(hashmap, record);

            // Store updated .hashlog
            if (store)
                WriteHashlog(hashlog, hashmap);
        }
        else
        {
            // Process all logging records
            Record record;
            while (InputRecord(*input, record, hashmap))
                if (!OutputRecord(*output, record))
                    break;
        }

        // Delete temporary file
        if (temp_file)
            Path::Remove(temp_file);

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }
}
