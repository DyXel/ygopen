/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_DUEL_CONSTANTS_CONTROLLER_HPP
#define YGOPEN_DUEL_CONSTANTS_CONTROLLER_HPP

namespace YGOpen::Duel
{

enum Controller : int
{
	CONTROLLER_0 = 0,
	CONTROLLER_1 = 1,
	CONTROLLER_ARRAY_SIZE, // NOTE: Leave always as last.
};

} // namespace YGOpen::Duel

#endif // YGOPEN_DUEL_CONSTANTS_CONTROLLER_HPP
