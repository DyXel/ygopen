/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_PROTO_DUEL_DATA_HPP
#define YGOPEN_PROTO_DUEL_DATA_HPP
// clang-format off
#include "enum.hpp"
#include <duel_data.pb.h>
// clang-format on

namespace YGOpen::Proto::Duel
{

// Check equivalence only of protobuf-exposed methods.
struct PlaceLess
{
	auto operator()(const Place& lhs, const Place& rhs) const noexcept -> bool
	{
		if(lhs.con() < rhs.con())
			return true;
		if(rhs.con() < lhs.con())
			return false;

		if(lhs.loc() < rhs.loc())
			return true;
		if(rhs.loc() < lhs.loc())
			return false;

		if(lhs.seq() < rhs.seq())
			return true;
		if(rhs.seq() < lhs.seq())
			return false;

		if(lhs.oseq() < rhs.oseq())
			return true;
		return false;
	}
};

// See enum.hpp `is_empty`.
inline auto is_empty(const Place& place) noexcept -> bool
{
	return is_empty(place.loc());
}

// See enum.hpp `is_pile`.
inline auto is_pile(const Place& place) noexcept -> bool
{
	return is_pile(place.loc());
}

} // namespace YGOpen::Proto::Duel

#endif // YGOPEN_PROTO_DUEL_DATA_HPP
