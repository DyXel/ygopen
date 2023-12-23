/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_PROTO_USER_HPP
#define YGOPEN_PROTO_USER_HPP
#include <string_view>
// clang-format off
#include <user.pb.h>
// clang-format on

namespace YGOpen::Proto
{

[[nodiscard]] constexpr auto validate_user_id(char const* data,
                                              size_t size) noexcept -> bool
{
	return false; // TODO
}

[[nodiscard]] constexpr auto validate_user_name(std::string_view name) noexcept
	-> bool
{
	return false; // TODO
}

} // namespace YGOpen::Proto

#endif // YGOPEN_PROTO_USER_HPP
