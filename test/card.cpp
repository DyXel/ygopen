/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <ygopen/client/card.hpp>
#include <ygopen/client/default_card_traits.hpp>

namespace
{

struct TestCardTraits
{
#define X(NAME, Name, name, value) using Name##Type = int;
#define EXPAND_ARRAY_LIKE_QUERIES
#define EXPAND_SEPARATE_LINK_DATA_QUERIES
#include <ygopen/client/queries.inl>
#undef EXPAND_SEPARATE_LINK_DATA_QUERIES
#undef EXPAND_ARRAY_LIKE_QUERIES
#undef X
};

class CardTest : public ::testing::Test
{
protected:
	static constexpr int VALUE = 0xB16B00B5;
	YGOpen::Client::BasicCard<TestCardTraits> c;
};

TEST_F(CardTest, GettersWorks)
{
#define X(NAME, Name, name, value) \
	ASSERT_EQ(c.name(), 0);        \
	c.name() = VALUE;              \
	ASSERT_EQ(std::as_const(c).name(), VALUE);
#define EXPAND_ARRAY_LIKE_QUERIES
#define EXPAND_SEPARATE_LINK_DATA_QUERIES
#include <ygopen/client/queries.inl>
#undef EXPAND_SEPARATE_LINK_DATA_QUERIES
#undef EXPAND_ARRAY_LIKE_QUERIES
#undef X
}

} // namespace
