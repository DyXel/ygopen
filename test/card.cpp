/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <ygopen/client/card.hpp>

namespace
{

struct TestCardTraits
{
	using OwnerType = int;
	using IsPublicType = int;
	using IsHiddenType = int;
	using PositionType = int;
	using CoverType = int;
	using StatusType = int;
	using CodeType = int;
	using AliasType = int;
	using TypeType = int;
	using LevelType = int;
	using XyzRankType = int;
	using AttributeType = int;
	using RaceType = int;
	using AtkDefType = int;
	using PendScalesType = int;
	using LinkRateType = int;
	using LinkArrowType = int;
	using CountersType = int;
	using EquippedType = int;
	using RelationsType = int;
};

class CardTest : public ::testing::Test
{
protected:
	static constexpr int VALUE = 0xB16B00B5;
	YGOpen::Client::BasicCard<TestCardTraits> c;
};

TEST_F(CardTest, OwnerGettersWorks)
{
	ASSERT_EQ(c.owner(), 0);
	c.owner() = VALUE;
	ASSERT_EQ(std::as_const(c).owner(), VALUE);
}

TEST_F(CardTest, IsPublicGettersWorks)
{
	ASSERT_EQ(c.is_public(), 0);
	c.is_public() = VALUE;
	ASSERT_EQ(std::as_const(c).is_public(), VALUE);
}

TEST_F(CardTest, IsHiddenGettersWorks)
{
	ASSERT_EQ(c.is_hidden(), 0);
	c.is_hidden() = VALUE;
	ASSERT_EQ(std::as_const(c).is_hidden(), VALUE);
}

TEST_F(CardTest, PositionGettersWorks)
{
	ASSERT_EQ(c.position(), 0);
	c.position() = VALUE;
	ASSERT_EQ(std::as_const(c).position(), VALUE);
}

TEST_F(CardTest, CoverGettersWorks)
{
	ASSERT_EQ(c.cover(), 0);
	c.cover() = VALUE;
	ASSERT_EQ(std::as_const(c).cover(), VALUE);
}

TEST_F(CardTest, StatusGettersWorks)
{
	ASSERT_EQ(c.status(), 0);
	c.status() = VALUE;
	ASSERT_EQ(std::as_const(c).status(), VALUE);
}

TEST_F(CardTest, CodeGettersWorks)
{
	ASSERT_EQ(c.code(), 0);
	c.code() = VALUE;
	ASSERT_EQ(std::as_const(c).code(), VALUE);
}

TEST_F(CardTest, AliasGettersWorks)
{
	ASSERT_EQ(c.alias(), 0);
	c.alias() = VALUE;
	ASSERT_EQ(std::as_const(c).alias(), VALUE);
}

TEST_F(CardTest, TypeGettersWorks)
{
	ASSERT_EQ(c.type(), 0);
	c.type() = VALUE;
	ASSERT_EQ(std::as_const(c).type(), VALUE);
}

TEST_F(CardTest, LevelGettersWorks)
{
	ASSERT_EQ(c.level(), 0);
	c.level() = VALUE;
	ASSERT_EQ(std::as_const(c).level(), VALUE);
}

TEST_F(CardTest, XyzRankGettersWorks)
{
	ASSERT_EQ(c.xyz_rank(), 0);
	c.xyz_rank() = VALUE;
	ASSERT_EQ(std::as_const(c).xyz_rank(), VALUE);
}

TEST_F(CardTest, AttributeGettersWorks)
{
	ASSERT_EQ(c.attribute(), 0);
	c.attribute() = VALUE;
	ASSERT_EQ(std::as_const(c).attribute(), VALUE);
}

TEST_F(CardTest, RaceGettersWorks)
{
	ASSERT_EQ(c.race(), 0);
	c.race() = VALUE;
	ASSERT_EQ(std::as_const(c).race(), VALUE);
}

TEST_F(CardTest, BaseAtkGettersWorks)
{
	ASSERT_EQ(c.base_atk(), 0);
	c.base_atk() = VALUE;
	ASSERT_EQ(std::as_const(c).base_atk(), VALUE);
}

TEST_F(CardTest, AtkGettersWorks)
{
	ASSERT_EQ(c.atk(), 0);
	c.atk() = VALUE;
	ASSERT_EQ(std::as_const(c).atk(), VALUE);
}

TEST_F(CardTest, BaseDefGettersWorks)
{
	ASSERT_EQ(c.base_def(), 0);
	c.base_def() = VALUE;
	ASSERT_EQ(std::as_const(c).base_def(), VALUE);
}

TEST_F(CardTest, DefGettersWorks)
{
	ASSERT_EQ(c.def(), 0);
	c.def() = VALUE;
	ASSERT_EQ(std::as_const(c).def(), VALUE);
}

TEST_F(CardTest, PendLScaleGettersWorks)
{
	ASSERT_EQ(c.pend_l_scale(), 0);
	c.pend_l_scale() = VALUE;
	ASSERT_EQ(std::as_const(c).pend_l_scale(), VALUE);
}

TEST_F(CardTest, PendRScaleGettersWorks)
{
	ASSERT_EQ(c.pend_r_scale(), 0);
	c.pend_r_scale() = VALUE;
	ASSERT_EQ(std::as_const(c).pend_r_scale(), VALUE);
}

TEST_F(CardTest, LinkRateGettersWorks)
{
	ASSERT_EQ(c.link_rate(), 0);
	c.link_rate() = VALUE;
	ASSERT_EQ(std::as_const(c).link_rate(), VALUE);
}

TEST_F(CardTest, LinkArrowGettersWorks)
{
	ASSERT_EQ(c.link_arrow(), 0);
	c.link_arrow() = VALUE;
	ASSERT_EQ(std::as_const(c).link_arrow(), VALUE);
}

TEST_F(CardTest, CountersGettersWorks)
{
	ASSERT_EQ(c.counters(), 0);
	c.counters() = VALUE;
	ASSERT_EQ(std::as_const(c).counters(), VALUE);
}

TEST_F(CardTest, EquippedGettersWorks)
{
	ASSERT_EQ(c.equipped(), 0);
	c.equipped() = VALUE;
	ASSERT_EQ(std::as_const(c).equipped(), VALUE);
}

TEST_F(CardTest, RelationsGettersWorks)
{
	ASSERT_EQ(c.relations(), 0);
	c.relations() = VALUE;
	ASSERT_EQ(std::as_const(c).relations(), VALUE);
}

} // namespace
