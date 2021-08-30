/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_DUEL_CONSTANTS_PHASE_HPP
#define YGOPEN_DUEL_CONSTANTS_PHASE_HPP
#include <cstdint>

namespace YGOpen::Duel
{

enum Phase : uint32_t
{
	PHASE_UNSPECIFIED = 0x0U,
	PHASE_DRAW = 0x1U,
	PHASE_STAND_BY = 0x2U,
	PHASE_MAIN_1 = 0x4U,
	PHASE_BATTLE_START = 0x8U,
	PHASE_BATTLE_BATTLE_STEP = 0x10U,
	PHASE_BATTLE_DAMAGE = 0x20U,
	PHASE_BATTLE_DAMAGE_CALC = 0x40U,
	PHASE_BATTLE = 0x80U,
	PHASE_MAIN_2 = 0x100U,
	PHASE_END = 0x200U,
};

} // namespace YGOpen::Duel

#endif // YGOPEN_DUEL_CONSTANTS_PHASE_HPP
