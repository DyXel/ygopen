#include <memory_resource>

#include <gtest/gtest.h>
#include <ygopen/client/value_wrappers.hpp>

template<typename T>
using Wrapper = YGOpen::Client::Sequential<T>;

TEST(SequentialWrapperTest, AppendingWorks)
{
	std::pmr::monotonic_buffer_resource alloc;
	Wrapper<int> w{&alloc};
	constexpr int value1 = 444;
	constexpr int value2 = 4444;
	w.set(value1);
	w.set(value2);
	ASSERT_EQ(w.get(), value2);
	w.dec();
	ASSERT_EQ(w.get(), value1);
	w.dec();
	ASSERT_EQ(w.get(), 0);
}

TEST(SequentialWrapperTest, NoOpWhenNotAtLast)
{
	std::pmr::monotonic_buffer_resource alloc;
	Wrapper<int> w{&alloc};
	constexpr int value = 444;
	w.set(value);
	w.dec();
	w.set(4444); // Shouldn't have any effect but forward iterator.
	ASSERT_EQ(w.get(), value);
}

// TODO: maybe add "death test" for when w.dec is called too much.
