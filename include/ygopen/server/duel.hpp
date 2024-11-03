/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_SERVER_DUEL_HPP
#define YGOPEN_SERVER_DUEL_HPP
#include <cstdint>
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

	virtual auto card_add(Proto::Duel::Place const& place, uint8_t owner_team,
	                      uint8_t owner_duelist, uint32_t code,
	                      uint32_t pos) noexcept -> void = 0;
	// TODO: Interface to add multiple cards at once?

	// Process callback is guaranteed to not be called before this.
	virtual auto start() noexcept -> void = 0;

	virtual auto submit_answer(Answer const& answer) noexcept -> void = 0;
};

} // namespace Server

} // namespace YGOpen

#endif // YGOPEN_SERVER_DUEL_HPP
