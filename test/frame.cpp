/*
 * Copyright (c) 2022, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
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

template<typename Frame>
constexpr auto expect_empty(Frame const& frame) noexcept -> void
{
	using namespace YGOpen::Duel;
	using namespace YGOpen::Client;
	for(uint8_t con = 0; con <= 1; con++)
	{
		for(uint32_t loc = LOCATION_MAIN_DECK; loc <= LOCATION_SKILL_ZONE;
		    loc <<= 1U)
		{
			if(loc == 0x80U) // NOTE: We don't care about LOCATION_OVERLAY.
				continue;
			YGOpen::Proto::Duel::Place p;
			p.set_con(con);
			p.set_loc(loc);
			if(is_pile(p))
			{
				EXPECT_FALSE(frame.has_card(p));
				continue;
			}
			for(uint32_t seq = 1; seq < zone_seq_lim(Location{loc}); seq++)
			{
				p.set_seq(seq);
				for(int32_t oseq = -1; oseq <= 0; oseq++)
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

} // namespace
