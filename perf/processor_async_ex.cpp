//
// Created by Ivan Shynkarenka on 01.08.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

#include "threads/mpmc_ring_queue.h"

#include <functional>
#include <thread>
#include <vector>

using namespace CppLogging;

const uint64_t items_to_produce = 8000000;
const auto settings = CppBenchmark::Settings().ParamRange(8, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

class RecordEx
{
public:
	//! Timestamp of the logging record
	uint64_t timestamp;
	//! Thread Id of the logging record
	uint64_t thread;
	//! Level of the logging record
	Level level;

	char logger[5];
	char message[16];

	RecordEx() noexcept
		: timestamp(CppCommon::Timestamp::utc()),
		thread(CppCommon::Thread::CurrentThreadId()),
		level(Level::INFO)
	{}
	RecordEx(const RecordEx&) noexcept = delete;
	RecordEx(RecordEx&& r) noexcept
	{
		timestamp = std::move(r.timestamp);
		thread = std::move(r.thread);
		level = std::move(r.level);
		std::strcpy(logger, r.logger);
		std::strcpy(message, r.message);
	}
	~RecordEx() noexcept = default;

	RecordEx& operator=(const RecordEx&) noexcept = delete;
	RecordEx& operator=(RecordEx&& r) noexcept
	{
		timestamp = std::move(r.timestamp);
		thread = std::move(r.thread);
		level = std::move(r.level);
		std::strcpy(logger, r.logger);
		std::strcpy(message, r.message);
		return *this;
	}
};

template<typename T, uint64_t N>
void test(CppBenchmark::Context& context, const std::function<void()>& wait_strategy)
{
    const int producers_count = context.x();
    uint64_t crc = 0;

    // Create multiple producers / multiple consumers wait-free ring queue
    CppCommon::MPMCRingQueue<T> queue(N);

    // Start consumer thread
    auto consumer = std::thread([&queue, &wait_strategy, &crc]()
    {
        for (uint64_t i = 0; i < items_to_produce; ++i)
        {
			// Dequeue using the given waiting strategy
			T item;
			while (!queue.Dequeue(item))
				wait_strategy();

			// Consume the item
			crc += 1;
        }
    });

    // Start producer threads
    std::vector<std::thread> producers;
    for (int producer = 0; producer < producers_count; ++producer)
    {
        producers.push_back(std::thread([&queue, &wait_strategy, producer, producers_count]()
        {
            uint64_t items = (items_to_produce / producers_count);
            for (uint64_t i = 0; i < items; ++i)
            {
                // Enqueue using the given waiting strategy
				T record;
				std::strcpy(record.logger, "Test");
				std::strcpy(record.message, "Test message");
                while (!queue.Enqueue(std::forward<T>(record)))
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
    context.metrics().AddBytes(items_to_produce * sizeof(T));
    context.metrics().SetCustom("Queue.capacity", N);
    context.metrics().SetCustom("CRC", crc);
}

BENCHMARK("Test", settings)
{
    test<RecordEx, 1048576>(context, []{ std::this_thread::yield(); });
}

BENCHMARK_MAIN()
