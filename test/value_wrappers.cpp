/*
 * Copyright (c) 2022, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <algorithm>
#include <gtest/gtest.h>
#include <vector>
#include <ygopen/client/value_wrappers.hpp>

namespace
{

// Wrapper over trivial scalar value.

template<typename T>
class IntWrapperTest : public testing::Test
{
public:
	T wrapper;
};

using IntWrappers = ::testing::Types<YGOpen::Client::Value<int>,
                                     YGOpen::Client::Sequential<int>>;
TYPED_TEST_SUITE(IntWrapperTest, IntWrappers);

TYPED_TEST(IntWrapperTest, DefaultConstructGetWorks)
{
	ASSERT_EQ(this->wrapper.get(), 0);
}

TYPED_TEST(IntWrapperTest, GetSetWorks)
{
	constexpr int value = 444;
	this->wrapper.set(value);
	ASSERT_EQ(this->wrapper.get(), value);
}

// Wrapper over non-trivial value.

template<typename T>
class VectorWrapperTest : public testing::Test
{
public:
	T wrapper;
};

using IntVector = std::vector<int>;

using VectorWrappers = ::testing::Types<YGOpen::Client::Value<IntVector>,
                                        YGOpen::Client::Sequential<IntVector>>;
TYPED_TEST_SUITE(VectorWrapperTest, VectorWrappers);

TYPED_TEST(VectorWrapperTest, GetSetWorks)
{
	auto& w = this->wrapper;
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

} // namespace
