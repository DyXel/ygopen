/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_DUEL_CONSTANTS_LINK_ARROW_HPP
#define YGOPEN_DUEL_CONSTANTS_LINK_ARROW_HPP
#include <cstdint>

namespace YGOpen::Duel
{

enum LinkArrow : uint32_t
{
	LINK_ARROW_UNSPECIFIED = 0x0U,
	LINK_ARROW_BOTTOM_LEFT = 0x1U,
	LINK_ARROW_BOTTOM = 0x2U,
	LINK_ARROW_BOTTOM_RIGHT = 0x4U,
	LINK_ARROW_LEFT = 0x8U,
	LINK_ARROW_RIGHT = 0x20U,
	LINK_ARROW_TOP_LEFT = 0x40U,
	LINK_ARROW_TOP = 0x80U,
	LINK_ARROW_TOP_RIGHT = 0x100U,
};

} // namespace YGOpen::Duel

#endif // YGOPEN_DUEL_CONSTANTS_LINK_ARROW_HPP
