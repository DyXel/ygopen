/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_PARSE_EVENT_HPP
#define YGOPEN_CLIENT_PARSE_EVENT_HPP
#include <ygopen/detail/config.hpp>
#include <ygopen/duel/constants/controller.hpp>
#include <ygopen/proto/duel/msg.hpp>

namespace YGOpen::Client
{

#ifdef YGOPEN_HAS_CONCEPTS
template<typename T, typename Concrete>
concept AssignableFromRepeated =
	requires(T t, google::protobuf::RepeatedPtrField<Concrete> m)
{
	t.assign(m.begin(), m.end());
};

template<typename T, typename Concrete>
concept PushableAndPopable = requires(T t, Concrete const& c)
{
	t.push_back(c);
	t.pop_back();
};

template<typename T, typename Concrete>
concept GettableAndSettable = requires(T t, Concrete c)
{
	Concrete{t};
	t = c;
};

template<typename T>
concept Frame = requires(T f, Proto::Duel::Place const& p)
{
	f.card_add(p);
	f.card_move(p, p);
	f.card_remove(p);
	f.card_swap(p, p);
	f.clear();
	f.pile_resize(p, size_t{});
	f.pile_splice(p, size_t{}, p, bool{});
};

template<typename T>
concept Board = requires(T b, Duel::Controller c)
{
	{
		b.blocked_zones()
	} noexcept -> AssignableFromRepeated<Proto::Duel::Place>;
	{
		b.chain_stack()
	} noexcept -> AssignableFromRepeated<Proto::Duel::Chain>;
	{
		b.chain_stack()
	} noexcept -> PushableAndPopable<Proto::Duel::Chain>;
	{
		b.frame()
	} noexcept -> Frame;
	{
		b.lp(c)
	} noexcept -> GettableAndSettable<uint32_t>;
	{
		b.phase()
	} noexcept -> GettableAndSettable<Duel::Phase>;
	{
		b.turn()
	} noexcept -> GettableAndSettable<uint32_t>;
	{
		b.turn_controller()
	} noexcept -> GettableAndSettable<Duel::Controller>;
};
#endif

template<YGOPEN_CONCEPT(Board)>
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
				board.turn() = static_cast<uint32_t>(tc);
			for(auto con : {CONTROLLER_0, CONTROLLER_1})
				board.lp(con) = state.lps(static_cast<int>(con));
			auto const& chains = state.chains();
			board.chain_stack().assign(chains.cbegin(), chains.cend());
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
			board.chain_stack().push_back(chain_stack.push());
			break;
		}
		case Msg::Event::ChainStack::kPop:
		{
			board.chain_stack().pop_back();
			break;
		}
		default:
			break;
		}
		break;
	}
	case Msg::Event::kNextTurn:
	{
		auto const next_turn = board.turn() + 1;
		board.turn() = next_turn;
		board.turn_controller() = Controller{event.next_turn()};
		break;
	}
	case Msg::Event::kLp:
	{
		auto const& lp = event.lp();
		auto& blp = board.lp(Controller{lp.controller()});
		auto lpv = uint32_t{board.lp(Controller{lp.controller()})};
		switch(lp.t_case())
		{
		case Msg::Event::LP::kBecome:
		{
			lpv = lp.become();
			break;
		}
		case Msg::Event::LP::kDamage:
		{
			lpv = (lpv < lp.damage()) ? 0 : lpv - lp.damage();
			break;
		}
		case Msg::Event::LP::kPay:
		{
			lpv = (lpv < lp.pay()) ? 0 : lpv - lp.pay();
			break;
		}
		case Msg::Event::LP::kRecover:
		{
			lpv = lpv + lp.recover();
			break;
		}
		default:
			break;
		}
		blp = lpv;
		break;
	}
	case Msg::Event::kNextPhase:
	{
		board.phase() = Phase{event.next_phase()};
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
		board.blocked_zones().assign(zones.cbegin(), zones.cend());
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
