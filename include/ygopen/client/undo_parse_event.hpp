/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_UNDO_PARSE_EVENT
#define YGOPEN_CLIENT_UNDO_PARSE_EVENT
#include <utility>
#include <ygopen/proto/duel/msg.hpp>

namespace YGOpen::Client
{

template<typename Board>
auto undo_parse_event(Board& board, Proto::Duel::Msg const& msg) noexcept
	-> void
{
	// TODO
}

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_UNDO_PARSE_EVENT
