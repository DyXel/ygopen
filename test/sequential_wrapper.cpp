#include <gtest/gtest.h>
#include <ygopen/client/value_wrappers.hpp>

namespace
{

class SequentialWrapperTest : public testing::Test
{
protected:
	static constexpr int VALUE_1 = 444;
	static constexpr int VALUE_2 = 444;
	std::pmr::monotonic_buffer_resource alloc;
	YGOpen::Client::Sequential<int> w{&alloc};
};

TEST_F(SequentialWrapperTest, AppendingWorks)
{
	ASSERT_EQ(w.get(), 0);
	w.set(VALUE_1);
	ASSERT_EQ(w.get(), VALUE_1);
	w.set(VALUE_2);
	ASSERT_EQ(w.get(), VALUE_2);
	w.dec();
	ASSERT_EQ(w.get(), VALUE_1);
	w.dec();
	ASSERT_EQ(w.get(), 0);
}

TEST_F(SequentialWrapperTest, NoOpWhenNotAtLast)
{
	w.set(VALUE_1);
	w.dec();
	w.set(VALUE_2); // Shouldn't have any effect but forward iterator.
	ASSERT_EQ(w.get(), VALUE_1);
}

// TODO: maybe add "death test" for when w.dec is called too much.

} // namespace
