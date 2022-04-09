/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_DUEL_CONSTANTS_LOCATION_HPP
#define YGOPEN_DUEL_CONSTANTS_LOCATION_HPP
#include <cstdint>

namespace YGOpen::Duel
{

enum Location : uint32_t
{
	LOCATION_UNSPECIFIED = 0x0U,
	LOCATION_MAIN_DECK = 0x1U,
	LOCATION_HAND = 0x2U,
	LOCATION_MONSTER_ZONE = 0x4U,
	LOCATION_SPELL_ZONE = 0x8U,
	LOCATION_GRAVEYARD = 0x10U,
	LOCATION_BANISHED = 0x20U,
	LOCATION_EXTRA_DECK = 0x40U,
	// 	LOCATION_OVERLAY       = 0x80U,
	LOCATION_FIELD_ZONE = 0x100U,
	LOCATION_PENDULUM_ZONE = 0x200U,
	LOCATION_SKILL_ZONE = 0x400U,
};

// Check if Location loc is unspecified.
constexpr auto is_empty(Location loc) noexcept -> bool
{
	return loc == LOCATION_UNSPECIFIED;
}

// Check if Location loc corresponds to a pile.
constexpr auto is_pile(Location loc) noexcept -> bool
{
	return (loc & (LOCATION_MAIN_DECK | LOCATION_HAND | LOCATION_GRAVEYARD |
	               LOCATION_BANISHED | LOCATION_EXTRA_DECK)) != 0U;
}

// Check if Location loc corresponds to zone.
constexpr auto is_zone(Location loc) noexcept -> bool
{
	return (loc &
	        (LOCATION_MONSTER_ZONE | LOCATION_SPELL_ZONE | LOCATION_FIELD_ZONE |
	         LOCATION_PENDULUM_ZONE | LOCATION_SKILL_ZONE)) != 0U;
}

} // namespace YGOpen::Duel

#endif // YGOPEN_DUEL_CONSTANTS_LOCATION_HPP
