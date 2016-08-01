/*!
    \file async_processor.cpp
    \brief Asynchronous logging processor definition
    \author Ivan Shynkarenka
    \date 01.08.2016
    \copyright MIT License
*/

#include "logging/processors/async_processor.h"

#include "errors/exceptions.h"
#include "errors/fatal.h"

namespace CppLogging {

AsyncProcessor::AsyncProcessor(bool discard_on_overflow, size_t capacity, size_t concurrency)
    : _discard_on_overflow(discard_on_overflow),
      _buffer(capacity, concurrency)
{
    // Start processing thread
    _thread = std::thread([this]() { ProcessBufferedRecords(); });
}

AsyncProcessor::~AsyncProcessor()
{
    // Local buffer to store the logging record
    static std::vector<uint8_t> local(1);

    // Calculate logging record size
    uint32_t size = sizeof(uint8_t);

    // Resize the local buffer to required size
    local.resize(size);

    // Get the buffer start position
    uint8_t* buffer = local.data();

    // Stop operation record
    const uint8_t type = 2;

    // Serialize stop operation record
    std::memcpy(buffer, &type, sizeof(uint8_t));
    buffer += sizeof(uint8_t);

    // Enqueue buffered flush operation record
    EnqueueRecord(false, local.data(), local.size());

    // Wait for processing thread
    _thread.join();
}

bool AsyncProcessor::ProcessRecord(Record& record)
{
    // Local buffer to store the logging record
    static std::vector<uint8_t> local(1024);

    // Calculate logging record size
    size_t size = sizeof(uint8_t) + sizeof(size_t) + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(Level) + sizeof(uint8_t) + record.logger.second + sizeof(uint16_t) + record.message.second + sizeof(uint32_t) + record.buffer.second + sizeof(uint32_t) + record.raw.second;

    // Resize the local buffer to required size
    local.resize(size);

    // Get the buffer start position
    uint8_t* buffer = local.data();

    // Logging record
    const uint8_t type = 0;

    // Serialize logging record
    std::memcpy(buffer, &type, sizeof(uint8_t));
    buffer += sizeof(uint8_t);
    std::memcpy(buffer, &size, sizeof(size_t));
    buffer += sizeof(size_t);
    std::memcpy(buffer, &record.timestamp, sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(buffer, &record.thread, sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    std::memcpy(buffer, &record.level, sizeof(Level));
    buffer += sizeof(Level);
    std::memcpy(buffer, &record.logger.second, sizeof(uint8_t));
    buffer += sizeof(uint8_t);
    std::memcpy(buffer, record.logger.first, record.logger.second);
    buffer += record.logger.second;
    std::memcpy(buffer, &record.message.second, sizeof(uint16_t));
    buffer += sizeof(uint16_t);
    std::memcpy(buffer, record.message.first, record.message.second);
    buffer += record.message.second;
    std::memcpy(buffer, &record.buffer.second, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    std::memcpy(buffer, record.buffer.first, record.buffer.second);
    buffer += record.buffer.second;
    std::memcpy(buffer, &record.raw.second, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    std::memcpy(buffer, record.raw.first, record.raw.second);
    buffer += record.buffer.second;

    // Enqueue buffered logger record
    return EnqueueRecord(_discard_on_overflow, local.data(), local.size());
}

bool AsyncProcessor::EnqueueRecord(bool discard_on_overflow, const void* chunk, size_t size)
{
    // Try to enqueue the given chunk of memory
    if (!_buffer.Enqueue(chunk, size))
    {
        // If the overflow policy is discard logging record, return immediately
        if (discard_on_overflow)
            return false;

        // If the overflow policy is blocking then spin, yield and sleep until the buffer is free
        int spins = 1000;
        int yields = 100;
        int sleep = 1;
        const int sleeps = 256;
        do
        {
            if (spins-- > 0)
            {
                // Spin...
            }
            else if (yields-- > 0)
            {
                // Yield...
                CppCommon::Thread::Yield();
            }
            else if (sleep < sleeps)
            {
                // Sleep...
                CppCommon::Thread::Sleep(sleep);
                sleep <<= 1;
            }
        } while (!_buffer.Enqueue(chunk, size));
    }

    return true;
}

void AsyncProcessor::ProcessBufferedRecords()
{
    try
    {
        // Local buffer to store the logging record
        static std::vector<uint8_t> local(1024);

        do
        {
            // Read record type
            uint8_t type;
            size_t type_size = sizeof(uint8_t);
            if (_buffer.Dequeue(&type, type_size))
            {
                switch (type)
                {
                    case 0:
                    {
                        // Read logging record size value
                        size_t size;
                        size_t size_size = sizeof(size_t);
                        if (!_buffer.Dequeue(&size, size_size))
                            throwex CppCommon::RuntimeException("Invalid logging record detected - bad record size!");

                        // Resize local buffer
                        local.resize(size);

                        // Read logging record
                        if (!_buffer.Dequeue(local.data(), size))
                            throwex CppCommon::RuntimeException("Invalid logging record detected - bad record data!");

                        // Get the buffer start position
                        uint8_t* buffer = local.data();

                        // Deserialize logging record
                        Record record;
                        std::memcpy(&record.timestamp, buffer, sizeof(uint64_t));
                        buffer += sizeof(uint64_t);
                        std::memcpy(&record.thread, buffer, sizeof(uint64_t));
                        buffer += sizeof(uint64_t);
                        std::memcpy(&record.level, buffer, sizeof(Level));
                        buffer += sizeof(Level);
                        std::memcpy(&record.logger.second, buffer, sizeof(uint8_t));
                        buffer += sizeof(uint8_t);
                        record.logger.first = (char*)buffer;
                        buffer += record.logger.second;
                        std::memcpy(&record.message.second, buffer, sizeof(uint16_t));
                        buffer += sizeof(uint16_t);
                        record.message.first = (char*)buffer;
                        buffer += record.message.second;
                        std::memcpy(&record.buffer.second, buffer, sizeof(uint32_t));
                        buffer += sizeof(uint32_t);
                        record.buffer.first = buffer;
                        buffer += record.buffer.second;
                        std::memcpy(&record.raw.second, buffer, sizeof(uint32_t));
                        buffer += sizeof(uint32_t);
                        record.raw.first = buffer;
                        buffer += record.raw.second;

                        // Process logging record
                        Processor::ProcessRecord(record);
                        break;
                    }
                    case 1:
                    {
                        // Flush the logging processor
                        Processor::Flush();
                        break;
                    }
                    case 2:
                        return;
                    default:
                        break;
                }
            }

        } while (true);
    }
    catch (...)
    {
        fatality("Asynchronous logging processor terminated!");
    }
}

void AsyncProcessor::Flush()
{
    // Local buffer to store the logging record
    static std::vector<uint8_t> local(1);

    // Calculate logging record size
    uint32_t size = sizeof(uint8_t);

    // Resize the local buffer to required size
    local.resize(size);

    // Get the buffer start position
    uint8_t* buffer = local.data();

    // Flush operation record
    const uint8_t type = 1;

    // Serialize flush operation record
    std::memcpy(buffer, &type, sizeof(uint8_t));
    buffer += sizeof(uint8_t);

    // Enqueue buffered flush operation record
    EnqueueRecord(false, local.data(), local.size());
}

} // namespace CppLogging
