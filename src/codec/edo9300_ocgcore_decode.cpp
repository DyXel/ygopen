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
		out.resize(sizeof(uint32_t));
		auto* ptr = out.data();
		write(ptr, static_cast<uint32_t>(v));
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
		// TODO
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
