/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_DUEL_CONSTANTS_STATUS_HPP
#define YGOPEN_DUEL_CONSTANTS_STATUS_HPP
#include <cstdint>

namespace YGOpen::Duel
{

enum Status : uint32_t
{
	STATUS_UNSPECIFIED = 0x0U,
	STATUS_DISABLED = 0x1U,
	STATUS_TO_ENABLE = 0x2U,
	STATUS_TO_DISABLE = 0x4U,
	STATUS_PROC_COMPLETE = 0x8U,
	STATUS_SET_TURN = 0x10U,
	STATUS_NO_LEVEL = 0x20U,
	STATUS_BATTLE_RESULT = 0x40U,
	STATUS_SPSUMMON_STEP = 0x80U,
	STATUS_FORM_CHANGED = 0x100U,
	STATUS_SUMMONING = 0x200U,
	STATUS_EFFECT_ENABLED = 0x400U,
	STATUS_SUMMON_TURN = 0x800U,
	STATUS_DESTROY_CONFIRMED = 0x1000U,
	STATUS_LEAVE_CONFIRMED = 0x2000U,
	STATUS_BATTLE_DESTROYED = 0x4000U,
	STATUS_COPYING_EFFECT = 0x8000U,
	STATUS_CHAINING = 0x10000U,
	STATUS_SUMMON_DISABLED = 0x20000U,
	STATUS_ACTIVATE_DISABLED = 0x40000U,
	STATUS_EFFECT_REPLACED = 0x80000U,
	STATUS_FUTURE_FUSION = 0x100000U,
	STATUS_ATTACK_CANCELED = 0x200000U,
	STATUS_INITIALIZING = 0x400000U,
	// 	STATUS_ACTIVATED         = 0x800000U,
	STATUS_JUST_POS = 0x1000000U,
	STATUS_CONTINUOUS_POS = 0x2000000U,
	STATUS_FORBIDDEN = 0x4000000U,
	STATUS_ACT_FROM_HAND = 0x8000000U,
	STATUS_OPPO_BATTLE = 0x10000000U,
	STATUS_FLIP_SUMMON_TURN = 0x20000000U,
	STATUS_SPSUMMON_TURN = 0x40000000U,
};

} // namespace YGOpen::Duel

#endif // YGOPEN_DUEL_CONSTANTS_STATUS_HPP
