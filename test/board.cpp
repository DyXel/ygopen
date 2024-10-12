/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <ygopen/client/board.hpp>

namespace
{

struct TestBoardTraits
{
	using BlockedZonesType = int;
	using ChainStackType = int;
	using FrameType = int;
	using LPType = int;
	using PhaseType = int;
	using TurnControllerType = int;
	using TurnType = int;
};

class BoardTest : public ::testing::Test
{
protected:
	static constexpr int VALUE = 123;
	YGOpen::Client::BasicBoard<TestBoardTraits> b;
};

TEST_F(BoardTest, BlockedZonesGettersWorks)
{
	ASSERT_EQ(b.blocked_zones(), 0);
	b.blocked_zones() = VALUE;
	ASSERT_EQ(std::as_const(b).blocked_zones(), VALUE);
}

TEST_F(BoardTest, ChainStackGettersWorks)
{
	ASSERT_EQ(b.chain_stack(), 0);
	b.chain_stack() = VALUE;
	ASSERT_EQ(std::as_const(b).chain_stack(), VALUE);
}

TEST_F(BoardTest, FrameGettersWorks)
{
	ASSERT_EQ(b.frame(), 0);
	b.frame() = VALUE;
	ASSERT_EQ(std::as_const(b).frame(), VALUE);
}

TEST_F(BoardTest, LPGettersWorks)
{
	using namespace YGOpen::Duel;
	for(auto con : {CONTROLLER_0, CONTROLLER_1})
	{
		ASSERT_EQ(b.lp(con), 0);
		b.lp(con) = VALUE;
		ASSERT_EQ(std::as_const(b).lp(con), VALUE);
	}
}

TEST_F(BoardTest, PhaseGettersWorks)
{
	ASSERT_EQ(b.phase(), 0);
	b.phase() = VALUE;
	ASSERT_EQ(std::as_const(b).phase(), VALUE);
}

TEST_F(BoardTest, TurnControllerGettersWorks)
{
	ASSERT_EQ(b.turn_controller(), 0);
	b.turn_controller() = VALUE;
	ASSERT_EQ(std::as_const(b).turn_controller(), VALUE);
}

TEST_F(BoardTest, TurnGettersWorks)
{
	ASSERT_EQ(b.turn(), 0);
	b.turn() = VALUE;
	ASSERT_EQ(std::as_const(b).turn(), VALUE);
}

} // namespace
