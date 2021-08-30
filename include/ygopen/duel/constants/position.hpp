/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_DUEL_CONSTANTS_POSITION_HPP
#define YGOPEN_DUEL_CONSTANTS_POSITION_HPP
#include <cstdint>

namespace YGOpen::Duel
{

enum Position : uint32_t
{
	POSITION_UNSPECIFIED = 0x0U,
	POSITION_FACE_UP_ATTACK = 0x1U,
	POSITION_FACE_DOWN_ATTACK = 0x2U,
	POSITION_FACE_UP_DEFENSE = 0x4U,
	POSITION_FACE_DOWN_DEFENSE = 0x8U,
};

} // namespace YGOpen::Duel

#endif // YGOPEN_DUEL_CONSTANTS_POSITION_HPP
