//
// Created by Ivan Shynkarenka on 01.08.2016.
//

#include "benchmark/cppbenchmark.h"

#include "logging/config.h"
#include "logging/logger.h"

#include <functional>
#include <thread>
#include <vector>

using namespace CppLogging;

const uint64_t items_to_produce = 8000000;
const auto settings = CppBenchmark::Settings().ParamRange(8, 8, [](int from, int to, int& result) { int r = result; result *= 2; return r; });

template<typename T>
class MPMCRingQueue
{
public:
	//! Default class constructor
	/*!
	\param capacity - Ring queue capacity (must be a power of two)
	*/
	explicit MPMCRingQueue(size_t capacity);
	MPMCRingQueue(const MPMCRingQueue&) = delete;
	MPMCRingQueue(MPMCRingQueue&&) = default;
	~MPMCRingQueue() { delete[] _buffer; }

	MPMCRingQueue& operator=(const MPMCRingQueue&) = delete;
	MPMCRingQueue& operator=(MPMCRingQueue&&) = default;

	//! Get ring queue capacity
	size_t capacity() const noexcept { return _capacity; }
	//! Get ring queue size
	size_t size() const noexcept;

	//! Enqueue an item into the ring queue (multiple producers threads method)
	/*!
	The item will be copied into the ring queue.

	\param item - Item to enqueue
	\return 'true' if the item was successfully enqueue, 'false' if the ring queue is full
	*/
	bool Enqueue(T& item);

	//! Dequeue an item from the ring queue (multiple consumers threads method)
	/*!
	The item will be moved from the ring queue.

	\param item - Item to dequeue
	\return 'true' if the item was successfully dequeue, 'false' if the ring queue is empty
	*/
	bool Dequeue(T& item);

private:
	struct Node
	{
		T value;
		std::atomic<size_t> sequence;
	};

	typedef char cache_line_pad[64];

	cache_line_pad _pad0;
	const size_t _capacity;
	const size_t _mask;
	Node* const _buffer;

	cache_line_pad _pad1;
	std::atomic<size_t> _head;

	cache_line_pad _pad2;
	std::atomic<size_t> _tail;
};

template<typename T>
inline MPMCRingQueue<T>::MPMCRingQueue(size_t capacity) : _capacity(capacity), _mask(capacity - 1), _buffer(new Node[capacity]), _head(0), _tail(0)
{
	assert((capacity > 1) && "Ring queue capacity must be greater than one!");
	assert(((capacity & (capacity - 1)) == 0) && "Ring queue capacity must be a power of two!");

	// Populate the sequence initial values
	for (size_t i = 0; i < capacity; ++i)
		_buffer[i].sequence.store(i, std::memory_order_relaxed);
}

template<typename T>
inline size_t MPMCRingQueue<T>::size() const noexcept
{
	const size_t head = _head.load(std::memory_order_acquire);
	const size_t tail = _tail.load(std::memory_order_acquire);

	return head - tail;
}

template<typename T>
inline bool MPMCRingQueue<T>::Enqueue(T& item)
{
	size_t head_sequence = _head.load(std::memory_order_relaxed);

	for (;;)
	{
		Node* node = &_buffer[head_sequence & _mask];
		size_t node_sequence = node->sequence.load(std::memory_order_acquire);

		// If node sequence and head sequence are the same then it means this slot is empty
		int64_t diff = (int64_t)node_sequence - (int64_t)head_sequence;
		if (diff == 0)
		{
			// Claim our spot by moving head. If head isn't the same
			// as we last checked then that means someone beat us to
			// the punch weak compare is faster, but can return spurious
			// results which in this instance is OK, because it's in the loop
			if (_head.compare_exchange_weak(head_sequence, head_sequence + 1, std::memory_order_relaxed))
			{
				// Store the item value
				Record::Swap(node->value, item);

				// Increment the sequence so that the tail knows it's accessible
				node->sequence.store(head_sequence + 1, std::memory_order_release);
				return true;
			}
		}
		else if (diff < 0)
		{
			// If node sequence is less than head sequence then it means this slot is full
			// and therefore buffer is full
			return false;
		}
		else
		{
			// Under normal circumstances this branch should never be taken
			head_sequence = _head.load(std::memory_order_relaxed);
		}
	}

	// Never taken
	return false;
}

template<typename T>
inline bool MPMCRingQueue<T>::Dequeue(T& item)
{
	size_t tail_sequence = _tail.load(std::memory_order_relaxed);

	for (;;)
	{
		Node* node = &_buffer[tail_sequence & _mask];
		size_t node_sequence = node->sequence.load(std::memory_order_acquire);

		// If node sequence and head sequence are the same then it means this slot is empty
		int64_t diff = (int64_t)node_sequence - (int64_t)(tail_sequence + 1);
		if (diff == 0)
		{
			// Claim our spot by moving head. If head isn't the same
			// as we last checked then that means someone beat us to
			// the punch weak compare is faster, but can return spurious
			// results which in this instance is OK, because it's in the loop
			if (_tail.compare_exchange_weak(tail_sequence, tail_sequence + 1, std::memory_order_relaxed))
			{
				// Get the item value
				Record::Swap(item, node->value);

				// Set the sequence to what the head sequence should be next time around
				node->sequence.store(tail_sequence + _mask + 1, std::memory_order_release);
				return true;
			}
		}
		else if (diff < 0)
		{
			// if seq is less than head seq then it means this slot is full and therefore the buffer is full
			return false;
		}
		else
		{
			// Under normal circumstances this branch should never be taken
			tail_sequence = _tail.load(std::memory_order_relaxed);
		}
	}

	// Never taken
	return false;
}

class RecordEx
{
public:
	//! Timestamp of the logging record
	uint64_t timestamp;
	//! Thread Id of the logging record
	uint64_t thread;
	//! Level of the logging record
	Level level;

	std::string logger;
	std::string message;
	std::vector<uint8_t> buffer;
	std::vector<uint8_t> raw;

	RecordEx()
		: timestamp(CppCommon::Timestamp::utc()),
		thread(CppCommon::Thread::CurrentThreadId()),
		level(Level::INFO),
		logger(256, '\0'),
		message(512, '\0')
	{}
	RecordEx(const RecordEx&) = delete;
	RecordEx(RecordEx&& r) = default;
	~RecordEx() = default;

	friend void swap(RecordEx& r1, RecordEx& r2)
	{
		std::swap(r1.timestamp, r2.timestamp);
		std::swap(r1.thread, r2.thread);
		std::swap(r1.level, r2.level);
		std::swap(r1.logger, r2.logger);
		std::swap(r1.message, r2.message);
		std::swap(r1.buffer, r2.buffer);
		std::swap(r1.raw, r2.raw);
	}

	RecordEx& operator=(const RecordEx&) = delete;
	RecordEx& operator=(RecordEx&& r) = default;
};

// Create multiple producers / multiple consumers wait-free ring queue
MPMCRingQueue<Record> queue(4096);

template<typename T, uint64_t N>
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
			thread_local T item;
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
        producers.push_back(std::thread([&wait_strategy, producer, producers_count]()
        {
            uint64_t items = (items_to_produce / producers_count);
            for (uint64_t i = 0; i < items; ++i)
            {
				char buffer[256];
				itoa((int)i, buffer, 10);

                // Enqueue using the given waiting strategy
				thread_local T record;
				record.logger = "Test";
				record.message = "Test message ";
				record.message.append(buffer);
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
    context.metrics().AddBytes(items_to_produce * sizeof(T));
    context.metrics().SetCustom("Queue.capacity", N);
    context.metrics().SetCustom("CRC", crc);
}

BENCHMARK("Test", settings)
{
    test<Record, 4096>(context, []{ std::this_thread::yield(); });
}

BENCHMARK_MAIN()
