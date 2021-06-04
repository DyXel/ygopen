/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_PROTO_DUEL_ENUM_HPP
#define YGOPEN_PROTO_DUEL_ENUM_HPP
#include <duel_enum.pb.h>

namespace YGOpen::Proto::Duel
{

// Disable arithmetic operators for enums that are to be treated as bitfields.
// NOTE: << and >> with signed integers is bad, disable them too.

template<typename T>
struct DisableArithOps
{
	static const bool yes = false;
};

template<typename T>
using DisArithOpsT = typename std::enable_if_t<DisableArithOps<T>::yes, T>;

template<>
struct DisableArithOps<Attribute>
{
	static const bool yes = true;
};
template<>
struct DisableArithOps<LinkArrow>
{
	static const bool yes = true;
};
template<>
struct DisableArithOps<Location>
{
	static const bool yes = true;
};
template<>
struct DisableArithOps<Phase>
{
	static const bool yes = true;
};
template<>
struct DisableArithOps<Position>
{
	static const bool yes = true;
};
template<>
struct DisableArithOps<Race>
{
	static const bool yes = true;
};
template<>
struct DisableArithOps<Reason>
{
	static const bool yes = true;
};
template<>
struct DisableArithOps<Status>
{
	static const bool yes = true;
};
template<>
struct DisableArithOps<Type>
{
	static const bool yes = true;
};

template<typename T>
auto operator+=(T&, T) -> DisArithOpsT<T>& = delete;
template<typename T>
auto operator-=(T&, T) -> DisArithOpsT<T>& = delete;
template<typename T>
auto operator*=(T&, T) -> DisArithOpsT<T>& = delete;
template<typename T>
auto operator/=(T&, T) -> DisArithOpsT<T>& = delete;
template<typename T>
auto operator%=(T&, T) -> DisArithOpsT<T>& = delete;
template<typename T>
auto operator<<=(T&, T) -> DisArithOpsT<T>& = delete;
template<typename T>
auto operator>>=(T&, T) -> DisArithOpsT<T>& = delete;

template<typename T>
auto operator+(T, T) -> DisArithOpsT<T> = delete;
template<typename T>
auto operator-(T, T) -> DisArithOpsT<T> = delete;
template<typename T>
auto operator*(T, T) -> DisArithOpsT<T> = delete;
template<typename T>
auto operator/(T, T) -> DisArithOpsT<T> = delete;
template<typename T>
auto operator<<(T, T) -> DisArithOpsT<T> = delete;
template<typename T>
auto operator>>(T, T) -> DisArithOpsT<T> = delete;

constexpr int OSEQ_INVALID = -1;

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

} // namespace YGOpen::Proto::Duel

#endif // YGOPEN_PROTO_DUEL_ENUM_HPP
