/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_SERVER_DUEL_HPP
#define YGOPEN_SERVER_DUEL_HPP
#include <functional>
#include <ygopen/proto/duel/data.hpp>

namespace YGOpen
{

namespace Proto::Duel
{

class Answer;
class Msg;
class Place;

} // namespace Proto::Duel

namespace Server
{

class IDuel
{
public:
	using Msg = Proto::Duel::Msg;
	using Answer = Proto::Duel::Answer;
	using ProcessCallback = std::function<auto(Msg const&)->void>;

	struct Options // Whoever constructs a duel should take this as parameter.
	{
		ProcessCallback process_cb;
		Proto::Duel::Options duel_options;
	};

	virtual ~IDuel() noexcept = default;

	virtual auto add_card(uint32_t code, uint8_t team, uint8_t duelist,
	                      Proto::Duel::Place const& place) noexcept -> void;

	// Process callback is guaranteed to not be called before this.
	virtual auto start_duel() -> void;

	virtual auto submit_answer(Answer const& answer) noexcept -> void;
};

} // namespace Server

} // namespace YGOpen

#endif // YGOPEN_SERVER_DUEL_HPP
