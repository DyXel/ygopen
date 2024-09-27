/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <ygopen/client/board.hpp>
#include <ygopen/client/frame_limbo.hpp>
#include <ygopen/client/parse_event.hpp>
#include <ygopen/client/undo_parse_event.hpp>
#include <ygopen/client/undoable.hpp>
#include <ygopen/proto/duel/msg.hpp>

namespace
{

using namespace YGOpen::Client;
using namespace YGOpen::Duel;
using namespace YGOpen::Proto::Duel;

struct TestBoardTraits // TODO: Undoable values.
{
	using BlockedZonesType = BasicUndoable<std::vector<Place>>;
	using ChainStackType = BasicUndoable<std::vector<Chain>>;
	using FrameType = LimboFrame<int>;
	using LPType = BasicUndoable<uint32_t>;
	using PhaseType = BasicUndoable<Phase>;
	using TurnControllerType = BasicUndoable<Controller>;
	using TurnType = BasicUndoable<uint32_t>;
};

class ParseEventTest : public ::testing::Test
{
protected:
	Msg::Event e;
	BasicBoard<TestBoardTraits> b;

	auto SetUp() -> void override
	{
		// b.frame().pile_resize(10);
	}
};

TEST_F(ParseEventTest, BoardExchangeWorks)
{
	parse_event(b, e);
	undo_parse_event(b, e);
}

TEST_F(ParseEventTest, BoardStateWorks)
{
	parse_event(b, e);
	undo_parse_event(b, e);
}

} // namespace
