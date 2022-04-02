/*
 * Copyright (c) 2022, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <array>
#include <gtest/gtest.h>
#include <ygopen/client/frame.hpp>
#include <ygopen/client/frame_limbo.hpp>

namespace
{

template<typename T>
class FrameTest : public ::testing::Test
{
public:
	T frame;
};

struct Empty
{};

using FrameWithInt = YGOpen::Client::BasicFrame<int>;
using FrameWithEmpty = YGOpen::Client::BasicFrame<Empty>;
using LimboFrameWithInt = YGOpen::Client::LimboFrame<int>;
using LimboFrameWithEmpty = YGOpen::Client::LimboFrame<Empty>;

using FrameTypes = ::testing::Types<FrameWithInt, FrameWithEmpty,
                                    LimboFrameWithInt, LimboFrameWithEmpty>;
TYPED_TEST_SUITE(FrameTest, FrameTypes);

constexpr auto OSEQ_INVALID = YGOpen::Proto::Duel::OSEQ_INVALID;

using namespace YGOpen::Duel;

constexpr std::array LOC_ALL{LOCATION_MAIN_DECK,     LOCATION_HAND,
                             LOCATION_MONSTER_ZONE,  LOCATION_SPELL_ZONE,
                             LOCATION_GRAVEYARD,     LOCATION_BANISHED,
                             LOCATION_EXTRA_DECK,    LOCATION_FIELD_ZONE,
                             LOCATION_PENDULUM_ZONE, LOCATION_SKILL_ZONE};

template<typename Frame>
constexpr auto expect_empty(Frame const& frame) noexcept -> void
{
	using namespace YGOpen::Client;
	for(uint8_t con = 0; con <= 1; con++)
	{
		for(auto const loc : LOC_ALL)
		{
			YGOpen::Proto::Duel::Place p;
			p.set_con(con);
			p.set_loc(loc);
			if(is_pile(p))
			{
				EXPECT_FALSE(frame.has_card(p));
				continue;
			}
			for(uint32_t seq = 1; seq < zone_seq_lim(loc); seq++)
			{
				p.set_seq(seq);
				for(int32_t oseq = OSEQ_INVALID; oseq <= 0; oseq++)
				{
					p.set_oseq(oseq);
					EXPECT_FALSE(frame.has_card(p));
				}
			}
		}
	}
}

TYPED_TEST(FrameTest, IsInitializedEmpty)
{
	expect_empty(this->frame);
}

TYPED_TEST(FrameTest, AddingAndClearingCardsWork)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place p;
	p.set_oseq(OSEQ_INVALID);
	for(auto const loc : LOC_ALL)
	{
		p.set_loc(loc);
		EXPECT_FALSE(frame.has_card(p));
		auto& c = frame.card_add(p);
		ASSERT_TRUE(frame.has_card(p));
		EXPECT_EQ(&c, &frame.card(p));
	}
	frame.clear();
	expect_empty(frame);
}

TYPED_TEST(FrameTest, RemovingCardsWork)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place p;
	p.set_oseq(OSEQ_INVALID);
	for(auto const loc : LOC_ALL)
	{
		p.set_loc(loc);
		frame.card_add(p);
		ASSERT_TRUE(frame.has_card(p));
		frame.card_remove(p);
		EXPECT_FALSE(frame.has_card(p));
	}
	expect_empty(frame);
}

TYPED_TEST(FrameTest, TrivialCardMovesWork)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place p;
	p.set_loc(*LOC_ALL.cbegin());
	p.set_oseq(OSEQ_INVALID);
	frame.card_add(p);
	auto it = LOC_ALL.cbegin() + 1;
	for(size_t i = 0; i < LOC_ALL.size() * 2; ++i, ++it)
	{
		if(it == LOC_ALL.cend())
			it = LOC_ALL.cbegin();
		auto const prev = p;
		ASSERT_TRUE(frame.has_card(prev));
		p.set_loc(*it);
		p.set_oseq(i >= LOC_ALL.size() && !is_pile(*it) ? 0 : OSEQ_INVALID);
		frame.card_move(prev, p);
		EXPECT_FALSE(frame.has_card(prev));
		ASSERT_TRUE(frame.has_card(p));
	}
}

} // namespace
