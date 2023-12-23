/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_PROTO_DECK_HPP
#define YGOPEN_PROTO_DECK_HPP
// clang-format off
#include <deck.pb.h>
// clang-format on

namespace YGOpen::Proto
{

constexpr auto validate_deck_excessive_copies(
	Deck const& deck, DeckError::ExcessiveCopies& error) noexcept -> bool
{
	return false; // TODO
}

constexpr auto validate_deck_limits(Deck const& deck, DeckLimits const& limits,
                                    DeckError::OutOfLimit& error) noexcept
	-> bool
{
	return false; // TODO
}

} // namespace YGOpen::Proto

#endif // YGOPEN_PROTO_DECK_HPP
