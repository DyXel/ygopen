/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_PARSE_EVENT_HPP
#define YGOPEN_CLIENT_PARSE_EVENT_HPP
#include <utility>
#include <ygopen/duel/constants/controller.hpp>
#include <ygopen/proto/duel/msg.hpp>

namespace YGOpen::Client
{

template<typename Board>
auto parse_event(Board& board, Proto::Duel::Msg::Event const& event) noexcept
	-> void
{
	using namespace YGOpen::Duel;
	using namespace YGOpen::Proto::Duel;
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
			for(auto const& op : exchange.resize().ops())
				frame.pile_resize(op.place(), op.count());
			for(auto const& place : exchange.remove().places())
				frame.card_remove(place);
			for(auto const& place : exchange.add().places())
				frame.card_add(place);
			break;
		}
		case Msg::Event::Board::kState:
		{
			auto const& state = eb.state();
			if(auto tc = state.turn_counter(); tc >= 0)
				board.turn().set(static_cast<uint32_t>(tc));
			for(auto con : {CONTROLLER_0, CONTROLLER_1})
				board.lp(con).set(state.lps(static_cast<int>(con)));
			auto const& chains = state.chains();
			board.chain_stack().set({chains.cbegin(), chains.cend()});
			frame.clear();
			for(auto const& place : state.add().places())
				frame.card_add(place);
			for(auto const& op : state.resize().ops())
				frame.pile_resize(op.place(), op.count());
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
			for(auto const& place : card.add().places())
				frame.card_add(place);
			break;
		}
		case Msg::Event::Card::kExchange:
		{
			for(auto const& op : card.exchange().ops())
				frame.card_swap(op.place_a(), op.place_b());
			break;
		}
		case Msg::Event::Card::kMove:
		{
			for(auto const& op : card.move().ops())
				frame.card_move(op.old_place(), op.new_place());
			break;
		}
		case Msg::Event::Card::kRemove:
		{
			for(auto const& place : card.remove().places())
				frame.card_remove(place);
			break;
		}
		case Msg::Event::Card::kShuffle:
		{
			auto const& shuffle = card.shuffle();
			auto const& prev = shuffle.previous_places();
			auto const& curr = shuffle.current_places();
			frame.card_shuffle(prev.cbegin(), curr.cbegin(), prev.size());
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
			auto const& prev_chains = board.chain_stack().get();
			std::vector<Proto::Duel::Chain> new_chains{prev_chains.size() + 1U};
			std::copy(prev_chains.cbegin(), prev_chains.cend(),
			          new_chains.begin());
			new_chains.back() = chain_stack.push();
			board.chain_stack().set(std::move(new_chains));
			break;
		}
		case Msg::Event::ChainStack::kPop:
		{
			auto const& prev_chains = board.chain_stack().get();
			std::vector<Proto::Duel::Chain> new_chains{prev_chains.size() - 1U};
			std::copy(prev_chains.cbegin(), prev_chains.cend() - 1U,
			          new_chains.begin());
			board.chain_stack().set(std::move(new_chains));
			break;
		}
		default:
			break;
		}
		break;
	}
	case Msg::Event::kNextTurn:
	{
		board.turn().set(board.turn().get() + 1);
		board.turn_controller().set(Controller{event.next_turn()});
		break;
	}
	case Msg::Event::kLp:
	{
		auto const& lp = event.lp();
		auto& blp = board.lp(Controller{lp.controller()});
		switch(lp.t_case())
		{
		case Msg::Event::LP::kBecome:
		{
			blp.set(lp.become());
			break;
		}
		case Msg::Event::LP::kDamage:
		{
			blp.set((blp.get() < lp.damage()) ? 0 : blp.get() - lp.damage());
			break;
		}
		case Msg::Event::LP::kPay:
		{
			blp.set((blp.get() < lp.pay()) ? 0 : blp.get() - lp.pay());
			break;
		}
		case Msg::Event::LP::kRecover:
		{
			blp.set(blp.get() + lp.recover());
			break;
		}
		default:
			break;
		}
		break;
	}
	case Msg::Event::kNextPhase:
	{
		board.phase().set(Phase{event.next_phase()});
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
			for(auto const& op : pile.splice().ops())
				frame.pile_splice(op.from(), op.count(), op.to(), op.reverse());
			break;
		}
		case Msg::Event::Pile::kExchange:
		{
			for(auto const& op : pile.exchange().ops())
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
		board.blocked_zones().set({zones.cbegin(), zones.cend()});
		break;
	}
	case Msg::Event::kFinish:
	case Msg::Event::kMeta:
	default:
		break;
	}
}

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_PARSE_EVENT_HPP
