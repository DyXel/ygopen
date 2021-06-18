/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CODEC_EDO9300_OCGCORE_DECODE_HPP
#define YGOPEN_CODEC_EDO9300_OCGCORE_DECODE_HPP
#include "decode_common.hpp"

namespace YGOpen::Codec::Edo9300::OCGCore
{

auto decode_one_answer(Proto::Duel::Msg_Request const& request,
                       Proto::Duel::Answer const& answer,
                       std::vector<uint8_t>& out) noexcept -> void;

} // namespace YGOpen::Codec::Edo9300::OCGCore

#endif // YGOPEN_CODEC_EDO9300_OCGCORE_DECODE_HPP
