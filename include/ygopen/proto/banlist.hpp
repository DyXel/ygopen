/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_PROTO_BANLIST_HPP
#define YGOPEN_PROTO_BANLIST_HPP
// clang-format off
#include <banlist.pb.h>
// clang-format on

namespace YGOpen::Proto
{

constexpr auto validate_banlist(Banlist const& banlist,
                                BanlistError& error) noexcept -> bool
{
	return true; // TODO
}

} // namespace YGOpen::Proto

#endif // YGOPEN_PROTO_BANLIST_HPP
