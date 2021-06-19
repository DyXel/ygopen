/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include "ygopen/codec/edo9300_ocgcore_decode.hpp"

#include "ygopen/proto/duel/answer.hpp"
#include "ygopen/proto/duel/msg.hpp"

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
	switch(request.t_case())
	{
	case Msg::Request::kSelectAttribute:
	{
		assert(answer.t_case() == Answer::kSelectAttribute);
		response_i(answer.select_attribute());
		break;
	}
	case Msg::Request::kSelectCard:
	{
		assert(answer.t_case() == Answer::kSelectCard);
		// TODO
		break;
	}
	case Msg::Request::kSelectCardCode:
	{
		assert(answer.t_case() == Answer::kSelectCardCode);
		response_i(answer.select_card_code());
		break;
	}
	case Msg::Request::kSelectEffect:
	{
		assert(answer.t_case() == Answer::kSelectEffect);
		response_i(answer.select_effect());
		break;
	}
	case Msg::Request::kSelectIdle:
	{
		assert(answer.t_case() == Answer::kSelectIdle);
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
			response_i(((s & 0xFFFFU) << 16U) & (t & 0xFFFFU));
			break;
		}
		default:
			break;
		}
		break;
	}
	case Msg::Request::kSelectNumber:
	{
		assert(answer.t_case() == Answer::kSelectNumber);
		response_i(answer.select_number());
		break;
	}
	case Msg::Request::kSelectPosition:
	{
		assert(answer.t_case() == Answer::kSelectPosition);
		response_i(answer.select_position());
		break;
	}
	case Msg::Request::kSelectRace:
	{
		assert(answer.t_case() == Answer::kSelectRace);
		response_i(answer.select_race());
		break;
	}
	case Msg::Request::kSelectRockPaperScissors:
	{
		assert(answer.t_case() == Answer::kSelectRockPaperScissors);
		// TODO: Make sure we match core constants.
		response_i(answer.select_rock_paper_scissors());
		break;
	}
	case Msg::Request::kSelectToChain:
	{
		assert(answer.t_case() == Answer::kSelectToChain);
		// TODO
		break;
	}
	case Msg::Request::kSelectYesNo:
	{
		assert(answer.t_case() == Answer::kSelectYesNo);
		response_i(answer.select_yes_no());
		break;
	}
	case Msg::Request::kSelectZone:
	{
		assert(answer.t_case() == Answer::kSelectZone);
		// TODO
		break;
	}
	case Msg::Request::kSort:
	{
		assert(answer.t_case() == Answer::kSort);
		// TODO
		break;
	}
	default:
		break;
	}
}

} // namespace YGOpen::Codec::Edo9300::OCGCore
