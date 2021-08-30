/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_DUEL_CONSTANTS_ATTRIBUTE_HPP
#define YGOPEN_DUEL_CONSTANTS_ATTRIBUTE_HPP
#include <cstdint>

namespace YGOpen::Duel
{

enum Attribute : uint32_t
{
	ATTRIBUTE_UNSPECIFIED = 0x0U,
	ATTRIBUTE_EARTH = 0x1U,
	ATTRIBUTE_WATER = 0x2U,
	ATTRIBUTE_FIRE = 0x4U,
	ATTRIBUTE_WIND = 0x8U,
	ATTRIBUTE_LIGHT = 0x10U,
	ATTRIBUTE_DARK = 0x20U,
	ATTRIBUTE_DIVINE = 0x40U,
};

} // namespace YGOpen::Duel

#endif // YGOPEN_DUEL_CONSTANTS_ATTRIBUTE_HPP
