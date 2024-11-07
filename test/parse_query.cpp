/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <ygopen/client/card.hpp>
#include <ygopen/client/parse_query.hpp>

namespace
{

struct TestCardTraits
{
	using CodeType = uint32_t;
	using PositionType = YGOpen::Duel::Position;
	using AliasType = CodeType;
	using TypeType = YGOpen::Duel::Type;
	using LevelType = int32_t;
	using XyzRankType = uint32_t;
	using AttributeType = YGOpen::Duel::Attribute;
	using RaceType = YGOpen::Duel::Race;
	using AtkType = int32_t;
	using DefType = AtkType;
	using BaseAtkType = AtkType;
	using BaseDefType = AtkType;
	using EquippedToType = YGOpen::Proto::Duel::Place;
	using TargetsType = std::vector<YGOpen::Proto::Duel::Place>;
	using CountersType = std::vector<YGOpen::Proto::Duel::Counter>;
	using OwnerType = YGOpen::Duel::Controller;
	using StatusType = YGOpen::Duel::Status;
	using IsPublicType = bool;
	using PendLScaleType = uint32_t;
	using PendRScaleType = PendLScaleType;
	using IsHiddenType = bool;
	using CoverType = CodeType;
	using LinkRateType = uint32_t;
	using LinkArrowType = YGOpen::Duel::LinkArrow;
};

// TODO: counters, equipped, relations

class TestFrame
{
public:
	using CardType = YGOpen::Client::BasicCard<TestCardTraits>;
	CardType c;
	YGOpen::Proto::Duel::Place pp;

	auto card(YGOpen::Proto::Duel::Place const& p) noexcept -> CardType&
	{
		pp = p;
		return c;
	}

	auto has_card(YGOpen::Proto::Duel::Place const& p) noexcept -> bool
	{
		return true;
	}
};

class ParseQueryTest : public ::testing::Test
{
protected:
	static constexpr int VALUE1 = 12345;
	static constexpr int VALUE2 = 67890;
	TestFrame f;
	YGOpen::Client::QueryCacheHit hits;
	YGOpen::Proto::Duel::Msg::Query q;

	auto SetUp() -> void override
	{
		auto& p = *q.mutable_place();
		p.set_loc(1);
		p.set_seq(2);
		p.set_oseq(3);
	}

	auto TearDown() -> void override
	{
		ASSERT_EQ(f.pp.loc(), 1);
		ASSERT_EQ(f.pp.seq(), 2);
		ASSERT_EQ(f.pp.oseq(), 3);
	}
};

TEST_F(ParseQueryTest, OwnerSettingWorks)
{
	ASSERT_EQ(f.c.owner(), false);
	// Check that value is set
	q.mutable_data()->mutable_owner()->set_value(true);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.owner(), true);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.owner(), true);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_owner()->set_value(true);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.owner(), true);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::OWNER);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_owner()->set_value(false);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.owner(), false);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, IsPublicSettingWorks)
{
	ASSERT_EQ(f.c.is_public(), false);
	// Check that value is set
	q.mutable_data()->mutable_is_public()->set_value(true);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.is_public(), true);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.is_public(), true);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_is_public()->set_value(true);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.is_public(), true);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::IS_PUBLIC);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_is_public()->set_value(false);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.is_public(), false);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, IsHiddenSettingWorks)
{
	ASSERT_EQ(f.c.is_hidden(), false);
	// Check that value is set
	q.mutable_data()->mutable_is_hidden()->set_value(true);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.is_hidden(), true);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.is_hidden(), true);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_is_hidden()->set_value(true);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.is_hidden(), true);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::IS_HIDDEN);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_is_hidden()->set_value(false);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.is_hidden(), false);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, PositionSettingWorks)
{
	ASSERT_EQ(f.c.position(), 0);
	// Check that value is set
	q.mutable_data()->mutable_position()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.position(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.position(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_position()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.position(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::POSITION);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_position()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.position(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, CoverSettingWorks)
{
	ASSERT_EQ(f.c.cover(), 0);
	// Check that value is set
	q.mutable_data()->mutable_cover()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.cover(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.cover(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_cover()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.cover(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::COVER);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_cover()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.cover(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, StatusSettingWorks)
{
	ASSERT_EQ(f.c.status(), 0);
	// Check that value is set
	q.mutable_data()->mutable_status()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.status(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.status(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_status()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.status(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::STATUS);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_status()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.status(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, CodeSettingWorks)
{
	ASSERT_EQ(f.c.code(), 0);
	// Check that value is set
	q.mutable_data()->mutable_code()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.code(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.code(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_code()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.code(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::CODE);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_code()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.code(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, AliasSettingWorks)
{
	ASSERT_EQ(f.c.alias(), 0);
	// Check that value is set
	q.mutable_data()->mutable_alias()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.alias(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.alias(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_alias()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.alias(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::ALIAS);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_alias()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.alias(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, TypeSettingWorks)
{
	ASSERT_EQ(f.c.type(), 0);
	// Check that value is set
	q.mutable_data()->mutable_type()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.type(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.type(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_type()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.type(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::TYPE);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_type()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.type(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, LevelSettingWorks)
{
	ASSERT_EQ(f.c.level(), 0);
	// Check that value is set
	q.mutable_data()->mutable_level()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.level(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.level(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_level()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.level(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::LEVEL);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_level()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.level(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, XyzRankSettingWorks)
{
	ASSERT_EQ(f.c.xyz_rank(), 0);
	// Check that value is set
	q.mutable_data()->mutable_xyz_rank()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.xyz_rank(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.xyz_rank(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_xyz_rank()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.xyz_rank(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::RANK);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_xyz_rank()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.xyz_rank(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, AttributeSettingWorks)
{
	ASSERT_EQ(f.c.attribute(), 0);
	// Check that value is set
	q.mutable_data()->mutable_attribute()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.attribute(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.attribute(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_attribute()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.attribute(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::ATTRIBUTE);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_attribute()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.attribute(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, RaceSettingWorks)
{
	ASSERT_EQ(f.c.race(), 0);
	// Check that value is set
	q.mutable_data()->mutable_race()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.race(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.race(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_race()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.race(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::RACE);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_race()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.race(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, BaseAtkSettingWorks)
{
	ASSERT_EQ(f.c.base_atk(), 0);
	// Check that value is set
	q.mutable_data()->mutable_base_atk()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.base_atk(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.base_atk(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_base_atk()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.base_atk(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::BASE_ATTACK);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_base_atk()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.base_atk(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, AtkSettingWorks)
{
	ASSERT_EQ(f.c.atk(), 0);
	// Check that value is set
	q.mutable_data()->mutable_atk()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.atk(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.atk(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_atk()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.atk(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::ATTACK);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_atk()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.atk(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, BaseDefSettingWorks)
{
	ASSERT_EQ(f.c.base_def(), 0);
	// Check that value is set
	q.mutable_data()->mutable_base_def()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.base_def(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.base_def(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_base_def()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.base_def(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::BASE_DEFENSE);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_base_def()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.base_def(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, DefSettingWorks)
{
	ASSERT_EQ(f.c.def(), 0);
	// Check that value is set
	q.mutable_data()->mutable_def()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.def(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.def(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_def()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.def(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::DEFENSE);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_def()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.def(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, PendLScaleSettingWorks)
{
	ASSERT_EQ(f.c.pend_l_scale(), 0);
	// Check that value is set
	q.mutable_data()->mutable_pend_l_scale()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.pend_l_scale(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.pend_l_scale(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_pend_l_scale()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.pend_l_scale(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::LSCALE);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_pend_l_scale()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.pend_l_scale(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, PendRScaleSettingWorks)
{
	ASSERT_EQ(f.c.pend_r_scale(), 0);
	// Check that value is set
	q.mutable_data()->mutable_pend_r_scale()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.pend_r_scale(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.pend_r_scale(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_pend_r_scale()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.pend_r_scale(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::RSCALE);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_pend_r_scale()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.pend_r_scale(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, LinkRateSettingWorks)
{
	ASSERT_EQ(f.c.link_rate(), 0);
	// Check that value is set
	q.mutable_data()->mutable_link_rate()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.link_rate(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.link_rate(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_link_rate()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.link_rate(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::LINK_RATE);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_link_rate()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.link_rate(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

TEST_F(ParseQueryTest, LinkArrowSettingWorks)
{
	ASSERT_EQ(f.c.link_arrow(), 0);
	// Check that value is set
	q.mutable_data()->mutable_link_arrow()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.link_arrow(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that setting the same value doesn't hit cache when use_cache=false
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that value is NOT set when query is empty
	q.clear_data();
	hits = YGOpen::Client::parse_query(f, q);
	ASSERT_EQ(f.c.link_arrow(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
	// Check that cache is hit when use_cache=true
	q.mutable_data()->mutable_link_arrow()->set_value(VALUE1);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.link_arrow(), VALUE1);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::LINK_ARROW);
	// Check that cache is not hit when use_cache=true
	q.mutable_data()->mutable_link_arrow()->set_value(VALUE2);
	hits = YGOpen::Client::parse_query<true>(f, q);
	ASSERT_EQ(f.c.link_arrow(), VALUE2);
	ASSERT_EQ(hits, YGOpen::Client::QueryCacheHit::UNSPECIFIED);
}

} // namespace
