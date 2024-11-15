/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <ygopen/client/undoable.hpp>

namespace
{

class UndoableTest : public ::testing::Test
{
protected:
	static constexpr int VALUE_1 = 123;
	static constexpr int VALUE_2 = 456;
	YGOpen::Client::BasicUndoable<int> v;
};

TEST_F(UndoableTest, AppendingWorks)
{
	ASSERT_EQ(int{v}, 0);
	v = VALUE_1;
	ASSERT_EQ(int{v}, VALUE_1);
	v = VALUE_2;
	ASSERT_EQ(int{v}, VALUE_2);
	v.undo();
	ASSERT_EQ(int{v}, VALUE_1);
	v.undo();
	ASSERT_EQ(int{v}, 0);
}

TEST_F(UndoableTest, NoOpWhenNotAtLast)
{
	v = VALUE_1;
	v.undo();
	v = VALUE_2; // NOTE: Shouldn't have any effect but forward iterator.
	ASSERT_EQ(int{v}, VALUE_1);
}

TEST_F(UndoableTest, DirectlyEquallyComparable)
{
	v = VALUE_1;
	ASSERT_EQ(v, VALUE_1);
	v = VALUE_2;
	ASSERT_EQ(v, VALUE_2);
	v.undo();
	ASSERT_EQ(v, VALUE_1);
}

TEST_F(UndoableTest, UndoCalledTooMuchDies)
{
	v = VALUE_1;
	EXPECT_DEATH(
		{
			v.undo();
			v.undo();
		},
		"");
}

} // namespace
