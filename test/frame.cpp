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

template<typename T>
class FrameDeathTest : public FrameTest<T>
{};

struct Empty
{};

using FrameWithInt = YGOpen::Client::BasicFrame<int>;
using FrameWithEmpty = YGOpen::Client::BasicFrame<Empty>;
using LimboFrameWithInt = YGOpen::Client::LimboFrame<int>;
using LimboFrameWithEmpty = YGOpen::Client::LimboFrame<Empty>;

using FrameTypes = ::testing::Types<FrameWithInt, FrameWithEmpty,
                                    LimboFrameWithInt, LimboFrameWithEmpty>;
TYPED_TEST_SUITE(FrameTest, FrameTypes);
TYPED_TEST_SUITE(FrameDeathTest, FrameTypes);

constexpr auto OSEQ_INVALID = YGOpen::Proto::Duel::OSEQ_INVALID;

using namespace YGOpen::Duel;

constexpr std::array LOC_ALL{LOCATION_MAIN_DECK,     LOCATION_HAND,
                             LOCATION_MONSTER_ZONE,  LOCATION_SPELL_ZONE,
                             LOCATION_GRAVEYARD,     LOCATION_BANISHED,
                             LOCATION_EXTRA_DECK,    LOCATION_FIELD_ZONE,
                             LOCATION_PENDULUM_ZONE, LOCATION_SKILL_ZONE};

template<typename Frame>
auto expect_empty(Frame const& frame) noexcept -> void
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

TYPED_TEST(FrameTest, AccessThroughPileObjectWorks)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place p;
	p.set_loc(LOCATION_HAND);
	p.set_oseq(OSEQ_INVALID);
	auto const& c = frame.card_add(p);
	EXPECT_EQ(&c, &frame.card(p));
	auto const& pile = frame.pile(p);
	EXPECT_EQ(&pile, &frame.pile(Controller{p.con()}, Location{p.loc()}));
	EXPECT_EQ(&c, pile[p.seq()]);
	EXPECT_EQ(&frame.card(p), pile[p.seq()]);
}

TYPED_TEST(FrameTest, AccessThroughZoneObjectWorks)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place p;
	p.set_loc(LOCATION_MONSTER_ZONE);
	p.set_oseq(OSEQ_INVALID);
	auto const& zone = frame.zone(p);
	EXPECT_EQ(&zone,
	          &frame.zone(Controller{p.con()}, Location{p.loc()}, p.seq()));
	EXPECT_EQ(zone.card, nullptr);
	auto const& c1 = frame.card_add(p);
	EXPECT_EQ(&c1, &frame.card(p));
	EXPECT_EQ(&c1, zone.card);
	p.set_oseq(0);
	EXPECT_TRUE(zone.materials.empty());
	auto const& c2 = frame.card_add(p);
	EXPECT_FALSE(zone.materials.empty());
	EXPECT_EQ(&c2, &frame.card(p));
	EXPECT_EQ(&c2, zone.materials[p.oseq()]);
}

TYPED_TEST(FrameTest, RemovingCardsWork)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place p;
	auto run_loop = [&]()
	{
		for(auto const loc : LOC_ALL)
		{
			p.set_loc(loc);
			frame.card_add(p);
			ASSERT_TRUE(frame.has_card(p));
			frame.card_remove(p);
			EXPECT_FALSE(frame.has_card(p));
		}
	};
	p.set_oseq(OSEQ_INVALID);
	run_loop();
	p.set_oseq(0);
	run_loop();
	expect_empty(frame);
}

TYPED_TEST(FrameTest, TrivialCardMovesWork)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place p;
	p.set_loc(*LOC_ALL.cbegin());
	p.set_oseq(OSEQ_INVALID);
	auto* const c = &frame.card_add(p);
	auto it = LOC_ALL.cbegin() + 1;
	for(size_t i = 0; i < LOC_ALL.size() * 2; ++i, ++it)
	{
		if(it == LOC_ALL.cend())
			it = LOC_ALL.cbegin();
		auto const prev = p;
		ASSERT_TRUE(frame.has_card(prev));
		p.set_loc(*it);
		p.set_oseq(i >= LOC_ALL.size() && !is_pile(*it) ? 0 : OSEQ_INVALID);
		auto* const mc = &frame.card_move(prev, p);
		ASSERT_TRUE(frame.has_card(p));
		EXPECT_EQ(c, mc);
		EXPECT_EQ(c, &frame.card(p));
		EXPECT_FALSE(frame.has_card(prev));
	}
}

TYPED_TEST(FrameTest, OverlayCardMovesWork)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place p1;
	p1.set_loc(LOCATION_SPELL_ZONE);
	auto p2 = p1;
	p2.set_seq(1);
	auto* const c = &frame.card_add(p1);
	{
		auto* const mc = &frame.card_move(p1, p2);
		ASSERT_FALSE(frame.has_card(p1));
		ASSERT_TRUE(frame.has_card(p2));
		EXPECT_EQ(c, mc);
		EXPECT_EQ(c, &frame.card(p2));
	}
	{
		auto* const mc = &frame.card_move(p2, p1);
		ASSERT_TRUE(frame.has_card(p1));
		ASSERT_FALSE(frame.has_card(p2));
		EXPECT_EQ(c, mc);
		EXPECT_EQ(c, &frame.card(p1));
	}
	YGOpen::Proto::Duel::Place p3;
	p3.set_loc(LOCATION_GRAVEYARD);
	p3.set_oseq(OSEQ_INVALID);
	{
		auto* const mc = &frame.card_move(p1, p3);
		ASSERT_FALSE(frame.has_card(p1));
		ASSERT_TRUE(frame.has_card(p3));
		EXPECT_EQ(c, mc);
		EXPECT_EQ(c, &frame.card(p3));
	}
	{
		auto* const mc = &frame.card_move(p3, p1);
		ASSERT_TRUE(frame.has_card(p1));
		ASSERT_FALSE(frame.has_card(p3));
		EXPECT_EQ(c, mc);
		EXPECT_EQ(c, &frame.card(p1));
	}
	YGOpen::Proto::Duel::Place p4;
	p4.set_loc(LOCATION_PENDULUM_ZONE);
	p4.set_oseq(OSEQ_INVALID);
	{
		auto* const mc = &frame.card_move(p1, p4);
		ASSERT_FALSE(frame.has_card(p1));
		ASSERT_TRUE(frame.has_card(p4));
		EXPECT_EQ(c, mc);
		EXPECT_EQ(c, &frame.card(p4));
	}
	{
		auto* const mc = &frame.card_move(p4, p1);
		ASSERT_TRUE(frame.has_card(p1));
		ASSERT_FALSE(frame.has_card(p4));
		EXPECT_EQ(c, mc);
		EXPECT_EQ(c, &frame.card(p1));
	}
}

template<typename Frame>
auto basic_swap_test(Frame& frame, Location loc1, bool is_mat_1, Location loc2,
                     bool is_mat_2) noexcept -> void
{
	YGOpen::Proto::Duel::Place p1;
	p1.set_loc(loc1);
	p1.set_oseq(is_mat_1 ? 0 : OSEQ_INVALID);
	YGOpen::Proto::Duel::Place p2;
	p2.set_con(1);
	p2.set_loc(loc2);
	p2.set_oseq(is_mat_2 ? 0 : OSEQ_INVALID);
	auto* const c1 = &frame.card_add(p1);
	auto* const c2 = &frame.card_add(p2);
	EXPECT_NE(c1, c2);
	frame.card_swap(p1, p2);
	ASSERT_TRUE(frame.has_card(p1));
	ASSERT_TRUE(frame.has_card(p2));
	EXPECT_EQ(c1, &frame.card(p2));
	EXPECT_EQ(c2, &frame.card(p1));
	frame.card_swap(p2, p1);
	ASSERT_TRUE(frame.has_card(p1));
	ASSERT_TRUE(frame.has_card(p2));
	EXPECT_EQ(c1, &frame.card(p1));
	EXPECT_EQ(c2, &frame.card(p2));
}

TYPED_TEST(FrameTest, PileToPileSwapWorks)
{
	basic_swap_test(this->frame, LOCATION_MAIN_DECK, false, LOCATION_HAND,
	                false);
}

TYPED_TEST(FrameTest, PileToZoneSwapWorks)
{
	basic_swap_test(this->frame, LOCATION_EXTRA_DECK, false,
	                LOCATION_MONSTER_ZONE, false);
}

TYPED_TEST(FrameTest, ZoneToZoneSwapWorks)
{
	basic_swap_test(this->frame, LOCATION_MONSTER_ZONE, false,
	                LOCATION_SPELL_ZONE, false);
}

TYPED_TEST(FrameTest, OverlayToZoneSwapWorks)
{
	basic_swap_test(this->frame, LOCATION_MONSTER_ZONE, true,
	                LOCATION_SPELL_ZONE, false);
}

TYPED_TEST(FrameTest, OverlayToPileSwapWorks)
{
	basic_swap_test(this->frame, LOCATION_PENDULUM_ZONE, true,
	                LOCATION_GRAVEYARD, false);
}

TYPED_TEST(FrameTest, OverlayToOverlaySwapWorks)
{
	basic_swap_test(this->frame, LOCATION_SKILL_ZONE, true, LOCATION_FIELD_ZONE,
	                true);
}

TYPED_TEST(FrameTest, ZoneWithOverlaySwapWorks)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place p1;
	p1.set_loc(LOCATION_MONSTER_ZONE);
	p1.set_oseq(OSEQ_INVALID);
	auto po1 = p1;
	po1.set_oseq(0);
	auto po2 = po1;
	po2.set_con(1);
	auto p2 = po2;
	p2.set_oseq(OSEQ_INVALID);
	auto* const c1 = &frame.card_add(p1);
	auto* const co1 = &frame.card_add(po1);
	auto* const c2 = &frame.card_add(p2);
	auto* const co2 = &frame.card_add(po2);
	frame.card_swap(p1, p2);
	ASSERT_TRUE(frame.has_card(p1));
	ASSERT_TRUE(frame.has_card(po1));
	ASSERT_TRUE(frame.has_card(p2));
	ASSERT_TRUE(frame.has_card(po2));
	EXPECT_EQ(c1, &frame.card(p2));
	EXPECT_EQ(co1, &frame.card(po2));
	EXPECT_EQ(c2, &frame.card(p1));
	EXPECT_EQ(co2, &frame.card(po1));
	frame.card_swap(p2, p1);
	ASSERT_TRUE(frame.has_card(p1));
	ASSERT_TRUE(frame.has_card(po1));
	ASSERT_TRUE(frame.has_card(p2));
	ASSERT_TRUE(frame.has_card(po2));
	EXPECT_EQ(c1, &frame.card(p1));
	EXPECT_EQ(co1, &frame.card(po1));
	EXPECT_EQ(c2, &frame.card(p2));
	EXPECT_EQ(co2, &frame.card(po2));
}

TYPED_TEST(FrameTest, PileResizingWorks)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place place;
	place.set_loc(LOCATION_BANISHED);
	auto& pile = frame.pile(place);
	EXPECT_TRUE(pile.empty());
	constexpr size_t COUNT = 8U;
	frame.pile_resize(place, COUNT);
	EXPECT_EQ(pile.size(), COUNT);
	for(size_t i = 0U; i < COUNT; i++)
	{
		place.set_seq(i);
		EXPECT_TRUE(frame.has_card(place));
	}
	frame.pile_resize(place, 0U);
	EXPECT_TRUE(pile.empty());
	expect_empty(frame);
}

TYPED_TEST(FrameTest, PileSplicingWorks)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place place1;
	place1.set_loc(LOCATION_MAIN_DECK);
	place1.set_oseq(OSEQ_INVALID);
	YGOpen::Proto::Duel::Place place2;
	place2.set_loc(LOCATION_GRAVEYARD);
	place2.set_oseq(OSEQ_INVALID);
	constexpr size_t COUNT = 8U;
	constexpr size_t COUNT_HALF = COUNT / 2U;
	ASSERT_EQ(COUNT % 2U, 0U); // NOTE: Making sure the number is not odd.
	frame.pile_resize(place1, COUNT);
	auto const& p1 = frame.pile(place1);
	auto const& p2 = frame.pile(place2);
	auto const copy = p1;
	// Half of pile from beginning to exact same place (no visible effect).
	frame.pile_splice(place1, COUNT_HALF, place1, false);
	ASSERT_EQ(p1.size(), COUNT);
	ASSERT_TRUE(std::equal(p1.cbegin(), p1.cend(), copy.cbegin()));
	// Same as before, except reversed.
	frame.pile_splice(place1, COUNT_HALF, place1, true);
	ASSERT_EQ(p1.size(), COUNT);
	ASSERT_TRUE(std::equal(p1.cbegin(), p1.cbegin() + COUNT_HALF,
	                       copy.crbegin() + COUNT_HALF));
	// Splice the entire thing to a different pile while reversing.
	frame.pile_splice(place1, COUNT, place2, true);
	ASSERT_EQ(p1.size(), 0U);
	ASSERT_EQ(p2.size(), COUNT);
	ASSERT_TRUE(
		std::equal(p2.cbegin(), p2.cbegin() + COUNT_HALF, copy.crbegin()));
	ASSERT_TRUE(std::equal(p2.cbegin() + COUNT_HALF, p2.cend(), copy.cbegin()));
	// Move half from the beginning to the top of the pile while reversing.
	auto place3 = place2;
	place3.set_seq(COUNT_HALF);
	frame.pile_splice(place2, COUNT_HALF, place3, true);
	ASSERT_EQ(p2.size(), COUNT);
	ASSERT_TRUE(std::equal(p2.cbegin(), p2.cend(), copy.cbegin()));
}

TYPED_TEST(FrameTest, PileSwapWorks)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place place1;
	place1.set_loc(LOCATION_EXTRA_DECK);
	place1.set_oseq(OSEQ_INVALID);
	YGOpen::Proto::Duel::Place place2;
	place2.set_loc(LOCATION_BANISHED);
	place2.set_oseq(OSEQ_INVALID);
	frame.pile_resize(place1, 4U);
	frame.pile_resize(place2, 8U);
	auto const& pile1 = frame.pile(place1);
	auto const& pile2 = frame.pile(place2);
	auto const copy1 = pile1;
	auto const copy2 = pile2;
	frame.pile_swap(place1, place2);
	EXPECT_TRUE(std::equal(pile1.cbegin(), pile1.cend(), copy2.begin()));
	EXPECT_TRUE(std::equal(pile2.cbegin(), pile2.cend(), copy1.begin()));
	frame.pile_swap(place2, place1);
	EXPECT_TRUE(std::equal(pile1.cbegin(), pile1.cend(), copy1.begin()));
	EXPECT_TRUE(std::equal(pile2.cbegin(), pile2.cend(), copy2.begin()));
}

TYPED_TEST(FrameDeathTest, BadAccessViaCardDies)
{
	auto& frame = this->frame;
	YGOpen::Proto::Duel::Place place;
	place.set_loc(LOCATION_MONSTER_ZONE);
	(void)frame.card_add(place);
	place.set_loc(LOCATION_MAIN_DECK);
	(void)frame.card_add(place);
	// Bad controller range for Pile
	place.set_con(2);
	EXPECT_DEATH({ (void)frame.card(place); }, "");
	// Bad controller range for Zone
	place.set_loc(LOCATION_MONSTER_ZONE);
	EXPECT_DEATH({ (void)frame.card(place); }, "");
	// Bad range withing `materials` container for Zone
	place.set_con(0);
	place.set_oseq(1);
	EXPECT_DEATH({ (void)frame.card(place); }, "");
	// Zone's `card` is empty
	place.set_oseq(OSEQ_INVALID);
	EXPECT_DEATH({ (void)frame.card(place); }, "");
	// Bad range within Zone (n > zone_seq_limit(loc))
	place.set_seq(YGOpen::Client::zone_seq_lim(LOCATION_MONSTER_ZONE) + 1U);
	EXPECT_DEATH({ (void)frame.card(place); }, "");
	// Incorrect Zone/Pile location
	place.set_seq(0);
	place.set_loc(LOCATION_UNSPECIFIED);
	EXPECT_DEATH({ (void)frame.card(place); }, "");
	// NOTE: Any value `v` where `popcnt(v)>1` is true should fail.
	place.set_loc(0b11);
	EXPECT_DEATH({ (void)frame.card(place); }, "");
	// Bad range for Pile (n > place.size() - 1)
	place.set_seq(1);
	place.set_loc(LOCATION_MAIN_DECK);
	EXPECT_DEATH({ (void)frame.card(place); }, "");
}

TYPED_TEST(FrameDeathTest, BadPileAccessDies)
{
	// TODO
}

TYPED_TEST(FrameDeathTest, BadZoneAccessDies)
{
	// TODO
}

TYPED_TEST(FrameDeathTest, BadHasCardUsageDies)
{
	// TODO
}

TYPED_TEST(FrameDeathTest, BadAddCardUsageDies)
{
	// Adding card to a zone that is already occupied
	// TODO
}

TYPED_TEST(FrameDeathTest, BadRemoveCardUsageDies)
{
	// TODO
}

TYPED_TEST(FrameDeathTest, BadMoveCardUsageDies)
{
	// Moving card to a zone that is already occupied
	// TODO
}

TYPED_TEST(FrameDeathTest, BadSwapCardUsageDies)
{
	// TODO
}

TYPED_TEST(FrameDeathTest, BadPileResizeUsageDies)
{
	// TODO
}

TYPED_TEST(FrameDeathTest, BadPileSpliceUsageDies)
{
	// TODO
}

TYPED_TEST(FrameDeathTest, BadPileSwapUsageDies)
{
	// TODO
}

} // namespace
