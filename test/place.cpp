/*
 * Copyright (c) 2022, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <ygopen/proto/duel/data.hpp>

namespace
{

using namespace YGOpen::Proto::Duel;

TEST(PlaceTest, ZeroInitializedWorks)
{
	Place p;
	EXPECT_EQ(p.con(), 0);
	EXPECT_TRUE(is_empty(p));
	EXPECT_EQ(p.loc(), YGOpen::Duel::LOCATION_UNSPECIFIED);
	EXPECT_EQ(p.seq(), 0);
	EXPECT_EQ(p.oseq(), 0);
}

TEST(PlaceTest, PlaceLessWorks)
{
	Place p1;
	Place p2;
	auto expect_equivalence = [&]()
	{
		EXPECT_FALSE(PlaceLess{}(p1, p2));
		EXPECT_FALSE(PlaceLess{}(p2, p1));
	};
	auto expect_p1_greater = [&]()
	{
		EXPECT_FALSE(PlaceLess{}(p1, p2));
		EXPECT_TRUE(PlaceLess{}(p2, p1));
	};
	expect_equivalence();
	p1.set_con(1);
	expect_p1_greater();
	p2.set_con(1);
	expect_equivalence();
	p1.set_loc(YGOpen::Duel::LOCATION_MAIN_DECK);
	expect_p1_greater();
	p2.set_loc(YGOpen::Duel::LOCATION_MAIN_DECK);
	expect_equivalence();
	p1.set_seq(1);
	expect_p1_greater();
	p2.set_seq(1);
	expect_equivalence();
	p1.set_oseq(1);
	expect_p1_greater();
	p2.set_oseq(1);
	expect_equivalence();
	p2.set_oseq(-1);
	expect_p1_greater();
}

} // namespace
