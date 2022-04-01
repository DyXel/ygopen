#include <algorithm>
#include <vector>

#include <gtest/gtest.h>
#include <ygopen/client/value_wrappers.hpp>

template<typename T>
using Wrapper = YGOpen::Client::Value<T>;

TEST(TrivialWrapperTest, DefaultConstructGetWorks)
{
	Wrapper<int> w;
	ASSERT_EQ(w.get(), 0);
}

TEST(TrivialWrapperTest, SimpleGetSetWorks)
{
	Wrapper<int> w;
	constexpr int value = 444;
	w.set(value);
	ASSERT_EQ(w.get(), value);
}

TEST(TrivialWrapperTest, ComplexGetSetWorks)
{
	using IntVector = std::vector<int>;
	Wrapper<IntVector> w;
	auto value = IntVector{{1, 2, 3, 4}};
	auto check_eq = [&]()
	{
		auto const& wv = w.get();
		return std::equal(wv.cbegin(), wv.cend(), value.cbegin(), value.cend());
	};

	w.set(value);
	ASSERT_TRUE(check_eq());

	value.pop_back();
	ASSERT_FALSE(check_eq());

	w.set(value);
	ASSERT_TRUE(check_eq());
}
