//
// Created by Ivan Shynkarenka on 01.08.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"
#include "logging/processors/async_buffer.h"

#include <functional>
#include <thread>
#include <vector>

using namespace CppLogging;

const uint64_t items_to_produce = 8000000;
const auto settings = CppBenchmark::Settings().ParamRange(8, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

// Create multiple producers / multiple consumers wait-free ring queue
AsyncBuffer queue(4096);
BinaryLayout binarylayout;
TextLayout textlayout;
NullAppender nullappender;

void test(CppBenchmark::Context& context, const std::function<void()>& wait_strategy)
{
    const int producers_count = context.x();
    uint64_t crc = 0;

    // Start consumer thread
    auto consumer = std::thread([&wait_strategy, &crc]()
    {
        for (uint64_t i = 0; i < items_to_produce; ++i)
        {
			// Dequeue using the given waiting strategy
			thread_local Record record;
			while (!queue.Dequeue(record))
				wait_strategy();

			nullappender.AppendRecord(record);

			// Consume the record
			crc += record.message.size();
        }
    });

    // Start producer threads
    std::vector<std::thread> producers;
    for (int producer = 0; producer < producers_count; ++producer)
    {
        producers.push_back(std::thread([&wait_strategy, producer, producers_count]()
        {
            uint64_t items = (items_to_produce / producers_count);
            for (uint64_t i = 0; i < items; ++i)
            {
				char buffer[256];
				itoa((int)i, buffer, 10);

                // Enqueue using the given waiting strategy
				thread_local TextLayout layout;
				thread_local Record record;
				record.timestamp = CppCommon::Timestamp::utc();
				record.logger = "Test";
				record.message = "Test message ";
				record.message.append(buffer);
				record.buffer.clear();
				record.raw.clear();
				layout.LayoutRecord(record);
                while (!queue.Enqueue(record))
                    wait_strategy();
            }
        }));
    }

    // Wait for the consumer thread
    consumer.join();

    // Wait for all producers threads
    for (auto& producer : producers)
        producer.join();

    // Update benchmark metrics
    context.metrics().AddIterations(items_to_produce - 1);
    context.metrics().AddItems(items_to_produce);
    context.metrics().AddBytes(items_to_produce * sizeof(Record));
    context.metrics().SetCustom("Queue.capacity", queue.capacity());
    context.metrics().SetCustom("CRC", crc);
}

BENCHMARK("Test", settings)
{
    test(context, []{ std::this_thread::yield(); });
}

BENCHMARK_MAIN()
