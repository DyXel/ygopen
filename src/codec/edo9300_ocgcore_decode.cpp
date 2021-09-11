/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include "ygopen/codec/edo9300_ocgcore_decode.hpp"

#include <ygopen/duel/constants/phase.hpp>
#include <ygopen/proto/duel/answer.hpp>
#include <ygopen/proto/duel/msg.hpp>

namespace YGOpen::Codec::Edo9300::OCGCore
{

namespace
{

template<typename T>
constexpr void write(uint8_t*& ptr, T value) noexcept
{
	std::memcpy(ptr, &value, sizeof(T));
	ptr += sizeof(T);
}

} // namespace

auto decode_one_answer(Proto::Duel::Msg::Request const& request,
                       Proto::Duel::Answer const& answer,
                       std::vector<uint8_t>& out) noexcept -> void
{
	auto response_i = [&](auto v) constexpr noexcept
	{
		out.resize(sizeof(int32_t));
		auto* ptr = out.data();
		write(ptr, static_cast<int32_t>(v));
	};
	using namespace Proto::Duel;
	switch(answer.t_case())
	{
	case Answer::kSelectAttribute:
	{
		response_i(answer.select_attribute());
		break;
	}
	case Answer::kSelectCard:
	{
		auto const& select_card = answer.select_card();
		switch(select_card.t_case())
		{
		case Answer::SelectCard::kCancel:
		case Answer::SelectCard::kFinish:
		{
			response_i(-1);
			break;
		}
		case Answer::SelectCard::kCounters:
		{
			assert(request.t_case() == Msg::Request::kSelectCard);
			auto const& rsc = request.select_card();
			assert(rsc.t_case() == Msg::Request::SelectCard::kCounters);
			auto const card_count = rsc.counters().cards_size();
			// TODO: Finish this.
			break;
		}
		case Answer::SelectCard::kIndexes:
		{
			auto const& indexes = select_card.indexes().values();
			assert(indexes.size() > 0);
			bool const is_select_unselect = [&]()
			{
				assert(request.t_case() == Msg::Request::kSelectCard);
				auto const& rsc = request.select_card();
				return rsc.t_case() == Msg::Request::SelectCard::kRecursive;
			}();
			if(is_select_unselect)
			{
				out.resize(sizeof(int32_t) * 2U);
				auto* ptr = out.data();
				write<int32_t>(ptr, 1);
				write(ptr, static_cast<int32_t>(*indexes.begin()));
			}
			else
			{
				auto const idx_sz = indexes.size();
				out.resize((sizeof(int32_t) * 2U) + (sizeof(int16_t) * idx_sz));
				auto* ptr = out.data();
				write<int32_t>(ptr, 1); // t == 1 so each index is int16_t
				write(ptr, static_cast<int32_t>(idx_sz));
				for(auto const index : indexes)
					write(ptr, static_cast<int16_t>(index));
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case Answer::kSelectCardCode:
	{
		response_i(answer.select_card_code());
		break;
	}
	case Answer::kSelectEffect:
	{
		response_i(answer.select_effect());
		break;
	}
	case Answer::kSelectIdle:
	{
		assert(request.t_case() == Msg::Request::kSelectIdle);
		auto const& select_idle = answer.select_idle();
		switch(select_idle.t_case())
		{
		case Answer::SelectIdle::kShuffle:
		{
			response_i(8);
			break;
		}
		case Answer::SelectIdle::kPhase:
		{
			using namespace YGOpen::Duel;
			auto const phase = select_idle.phase();
			if(request.select_idle().is_battle_cmd())
			{
				if((phase & PHASE_MAIN_2) != 0)
					response_i(2);
				if((phase & PHASE_END) != 0)
					response_i(3);
			}
			else
			{
				if((phase & PHASE_BATTLE) != 0)
					response_i(6);
				else if((phase & PHASE_END) != 0)
					response_i(7);
			}
			break;
		}
		case Answer::SelectIdle::kCardAction:
		{
			auto const& card_action = select_idle.card_action();
			auto const act = card_action.action();
			uint32_t t = 0; // Type of action in the core.
			uint32_t s = card_action.index();
			assert(act != Answer::SelectIdle::ACTION_SHUFFLE);
			assert(act != Answer::SelectIdle::ACTION_PHASE);
			if(act == Answer::SelectIdle::ACTION_ACTIVATE)
				t = (request.select_idle().is_battle_cmd()) ? 0U : 5U;
			else if(act == Answer::SelectIdle::ACTION_SUMMON)
				t = 0;
			else if(act == Answer::SelectIdle::ACTION_SPSUMMON)
				t = 1;
			else if(act == Answer::SelectIdle::ACTION_REPOSITION)
				t = 2;
			else if(act == Answer::SelectIdle::ACTION_MSET)
				t = 3;
			else if(act == Answer::SelectIdle::ACTION_SSET)
				t = 4;
			else // act == Answer::SelectIdle::ACTION_ATTACK
				t = 1;
			response_i(((s & 0xFFFFU) << 16U) | (t & 0xFFFFU));
			break;
		}
		default:
			break;
		}
		break;
	}
	case Answer::kSelectNumber:
	{
		response_i(answer.select_number());
		break;
	}
	case Answer::kSelectPosition:
	{
		response_i(answer.select_position());
		break;
	}
	case Answer::kSelectRace:
	{
		response_i(answer.select_race());
		break;
	}
	case Answer::kSelectRockPaperScissors:
	{
		// TODO: Make sure we match core constants.
		response_i(answer.select_rock_paper_scissors());
		break;
	}
	case Answer::kSelectToChain:
	{
		auto const& select_to_chain = answer.select_to_chain();
		switch(select_to_chain.t_case())
		{
		case Answer::SelectToChain::kNoOp:
		{
			response_i(-1);
			break;
		}
		case Answer::SelectToChain::kIndex:
		{
			response_i(select_to_chain.index());
			break;
		}
		default:
			break;
		}
		break;
	}
	case Answer::kSelectYesNo:
	{
		response_i(answer.select_yes_no());
		break;
	}
	case Answer::kSelectZone:
	{
		auto const& places = answer.select_zone().places();
		out.resize(sizeof(int8_t) * 3U * places.size());
		auto* ptr = out.data();
		for(const auto& p : places)
		{
			write(ptr, static_cast<int8_t>(p.con()));
			write(ptr, static_cast<int8_t>(p.loc()));
			write(ptr, static_cast<int8_t>(p.seq()));
		}
		break;
	}
	case Answer::kSort:
	{
		// TODO
		break;
	}
	default:
		break;
	}
}

} // namespace YGOpen::Codec::Edo9300::OCGCore
