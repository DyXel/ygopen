/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_UNDO_PARSE_EVENT
#define YGOPEN_CLIENT_UNDO_PARSE_EVENT
#include <ygopen/detail/config.hpp>
#include <ygopen/proto/duel/msg.hpp>

namespace YGOpen::Client
{

#ifdef YGOPEN_HAS_CONCEPTS
template<typename T>
concept UndoableBoard = requires(T b)
{
	b.frame();
};
#endif

namespace Detail
{

// TODO: Use std::ranges::reverse_view if we ever move to C++20
template<typename C>
class ReverseView
{
public:
	constexpr ReverseView(C const& c) noexcept : c_(&c) {}

	constexpr auto begin() const noexcept { return c_->rbegin(); }
	constexpr auto end() const noexcept { return c_->rend(); }

private:
	C const* c_;
};

} // namespace Detail

template<YGOPEN_CONCEPT(UndoableBoard)>
auto undo_parse_event(UndoableBoard& board,
                      Proto::Duel::Msg::Event const& event) noexcept -> void
{
	using namespace YGOpen::Duel;
	using namespace YGOpen::Proto::Duel;
	auto reverse = [](auto const& container)
	{
		return Detail::ReverseView{container};
	};
	switch(event.t_case())
	{
	case Msg::Event::kBoard:
	{
		auto const& eb = event.board();
		auto& frame = board.frame();
		switch(eb.t_case())
		{
		case Msg::Event::Board::kExchange:
		{
			auto const& exchange = eb.exchange();
			for(auto const& place : reverse(exchange.add().places()))
				frame.undo_card_add(place); // Conceptify
			for(auto const& place : reverse(exchange.remove().places()))
				frame.undo_card_remove(place);
			for(auto const& op : reverse(exchange.resize().ops()))
				frame.undo_pile_resize(op.place(), op.count());
			break;
		}
		case Msg::Event::Board::kState:
		{
			auto const& state = eb.state();
			for(auto const& op : reverse(state.resize().ops()))
				frame.undo_pile_resize(op.place(), op.count());
			for(auto const& place : reverse(state.add().places()))
				frame.undo_card_add(place);
			frame.undo_clear();
			auto const& chains = state.chains();
			board.chain_stack().undo_assign(chains.cbegin(), chains.cend());
			for(auto con : {CONTROLLER_1, CONTROLLER_0})
				board.lp(con).undo();
			if(auto tc = state.turn_counter(); tc >= 0)
				board.turn().undo();
			break;
		}
		default:
			break;
		}
		break;
	}
	case Msg::Event::kCard:
	{
		auto const& card = event.card();
		auto& frame = board.frame();
		switch(card.t_case())
		{
		case Msg::Event::Card::kAdd:
		{
			for(auto const& place : reverse(card.add().places()))
				frame.undo_card_add(place);
			break;
		}
		case Msg::Event::Card::kExchange:
		{
			for(auto const& op : reverse(card.exchange().ops()))
				frame.card_swap(op.place_a(), op.place_b());
			break;
		}
		case Msg::Event::Card::kMove:
		{
			for(auto const& op : reverse(card.move().ops()))
				frame.card_move(op.new_place(), op.old_place());
			break;
		}
		case Msg::Event::Card::kRemove:
		{
			for(auto const& place : reverse(card.remove().places()))
				frame.undo_card_remove(place);
			break;
		}
		case Msg::Event::Card::kShuffle:
		{
			auto const& shuffle = card.shuffle();
			auto const& prev = shuffle.previous_places();
			auto const& curr = shuffle.current_places();
			frame.undo_card_shuffle(prev.cbegin(), curr.cbegin(), prev.size());
			break;
		}
		default:
			break;
		}
		break;
	}
	case Msg::Event::kChainStack:
	{
		auto const& chain_stack = event.chain_stack();
		switch(chain_stack.t_case())
		{
		case Msg::Event::ChainStack::kPush:
		{
			board.chain_stack().undo_push_back(chain_stack.push());
			break;
		}
		case Msg::Event::ChainStack::kPop:
		{
			board.chain_stack().undo_pop_back();
			break;
		}
		default:
			break;
		}
		break;
	}
	case Msg::Event::kNextTurn:
	{
		auto const next_turn = uint32_t(board.turn()) + 1;
		board.turn().undo();
		board.turn_controller().undo();
		break;
	}
	case Msg::Event::kLp:
	{
		auto const& lp = event.lp();
		board.lp(Controller{lp.controller()}).undo();
		break;
	}
	case Msg::Event::kNextPhase:
	{
		board.phase().undo();
		break;
	}
	case Msg::Event::kPile:
	{
		auto const& pile = event.pile();
		auto& frame = board.frame();
		switch(pile.t_case())
		{
		case Msg::Event::Pile::kSplice:
		{
			for(auto const& op : reverse(pile.splice().ops()))
				frame.undo_pile_splice(op.from(), op.count(), op.to(), op.reverse());
			break;
		}
		case Msg::Event::Pile::kExchange:
		{
			for(auto const& op : reverse(pile.exchange().ops()))
				frame.pile_swap(op.place_a(), op.place_b());
			break;
		}
		default:
			break;
		}
		break;
	}
	case Msg::Event::kZoneBlock:
	{
		auto const& zones = event.zone_block().zones();
		board.blocked_zones().undo_assign(zones.cbegin(), zones.cend());
		break;
	}
	case Msg::Event::kFinish:
	case Msg::Event::kMeta:
	default:
		break;
	}
}

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_UNDO_PARSE_EVENT
