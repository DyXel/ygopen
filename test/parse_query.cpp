/*
 * Copyright (c) 2022, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <cstdint>
#include <gtest/gtest.h>
#include <ygopen/bit.hpp>

namespace
{

using namespace YGOpen::Bit;

TEST(BitOps, PopcntWorks)
{
	ASSERT_EQ(popcnt(0U), 0);
	ASSERT_EQ(popcnt(~uint32_t{}), 32);
	ASSERT_EQ(popcnt(~uint64_t{}), 64);
	ASSERT_EQ(popcnt(0b1010'0101U), 4);
}

TEST(BitOps, ChooseLWorks)
{
	ASSERT_EQ(choosel(0U, 0b1111'1111U), 0b0000'0000U);
	ASSERT_EQ(choosel(1U, 0b1111'1111U), 0b1000'0000U);
	ASSERT_EQ(choosel(4U, 0b0011'1111U), 0b0011'1100U);
	ASSERT_EQ(choosel(2U, 0b0000'0101U), 0b0000'0101U);
	ASSERT_EQ(choosel(9U, 0b1111'1111U), 0b1111'1111U);
}

TEST(BitOps, ChooseRWorks)
{
	ASSERT_EQ(chooser(0U, 0b1111'1111U), 0b0000'0000U);
	ASSERT_EQ(chooser(1U, 0b1111'1111U), 0b0000'0001U);
	ASSERT_EQ(chooser(4U, 0b0011'1111U), 0b0000'1111U);
	ASSERT_EQ(chooser(2U, 0b1010'0000U), 0b1010'0000U);
	ASSERT_EQ(chooser(9U, 0b1111'1111U), 0b1111'1111U);
}

} // namespace
