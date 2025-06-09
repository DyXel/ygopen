/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_SERVER_BASIC_ENCODE_CONTEXT_HPP
#define YGOPEN_SERVER_BASIC_ENCODE_CONTEXT_HPP
#include <map>
#include <vector>
#include <ygopen/client/board.hpp>
#include <ygopen/client/card.hpp>
#include <ygopen/client/default_card_traits.hpp>
#include <ygopen/client/frame.hpp>
#include <ygopen/client/parse_event.hpp>
#include <ygopen/client/parse_query.hpp>
#include <ygopen/codec/encode_common.hpp>
#include <ygopen/proto/duel/data.hpp>
#include <ygopen/proto/duel/msg.hpp>

// Common context needed for parsing events and queries. Necessary evil due to
// the design of core messages. Here's hoping one day I can delete this file.

namespace YGOpen::Server
{

class BasicEncodeContext final : public YGOpen::Codec::IEncodeContext
{
public:
	using Con = YGOpen::Codec::IEncodeContext::Con;
	using Loc = YGOpen::Codec::IEncodeContext::Loc;
	using Place = YGOpen::Codec::IEncodeContext::Place;

	auto pile_size(Con con, Loc loc) const noexcept -> size_t override
	{
		return board_.frame().pile(con, loc).size();
	}

	auto get_match_win_reason() const noexcept -> uint32_t override
	{
		return match_win_reason_;
	}

	auto has_xyz_mat(Place const& p) const noexcept -> bool override
	{
		return !board_.frame().zone(p).materials.empty();
	}

	auto get_xyz_left(Place const& left) const noexcept -> Place override
	{
		return left_.find(left)->second;
	}

	auto match_win_reason(uint32_t reason) noexcept -> void override
	{
		match_win_reason_ = reason;
	}

	auto xyz_mat_defer(Place const& place) noexcept -> void override
	{
		deferred_.emplace_back(place);
	}

	auto take_deferred_xyz_mat() noexcept -> std::vector<Place> override
	{
		return std::exchange(deferred_, {});
	}

	auto xyz_left(Place const& left, Place const& from) noexcept
		-> void override
	{
		left_[left] = from;
	}

	auto parse(YGOpen::Proto::Duel::Msg const& msg) noexcept -> void
	{
		if(msg.t_case() == YGOpen::Proto::Duel::Msg::kEvent)
			parse_event(board_, msg.event());
		for(auto const& query : msg.queries())
			static_cast<void>(
				YGOpen::Client::parse_query(board_.frame(), query));
	}

private:
	using CardType =
		YGOpen::Client::BasicCard<YGOpen::Client::DefaultCardTraits>;

	struct BoardTraits
	{
		using BlockedZonesType = std::vector<YGOpen::Proto::Duel::Place>;
		using ChainStackType = std::vector<YGOpen::Proto::Duel::Chain>;
		using FrameType = YGOpen::Client::BasicFrame<CardType>;
		using LPType = uint32_t;
		using PhaseType = YGOpen::Duel::Phase;
		using TurnControllerType = YGOpen::Duel::Controller;
		using TurnType = uint32_t;
	};

	using BoardType = YGOpen::Client::BasicBoard<BoardTraits>;

	BoardType board_;

	uint32_t match_win_reason_;
	std::map<Place, Place, YGOpen::Proto::Duel::PlaceLess> left_;
	std::vector<Place> deferred_;
};

} // namespace YGOpen::Server

#endif // YGOPEN_SERVER_BASIC_ENCODE_CONTEXT_HPP
