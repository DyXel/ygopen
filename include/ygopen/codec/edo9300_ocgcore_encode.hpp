/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CODEC_EDO9300_OCGCORE_ENCODE_HPP
#define YGOPEN_CODEC_EDO9300_OCGCORE_ENCODE_HPP
#include "encode_common.hpp"

namespace YGOpen::Codec::Edo9300::OCGCore
{

auto encode_one_query(uint8_t const* data,
                      Proto::Duel::Msg_Query_Data& q) noexcept
	-> EncodeOneQueryResult;

auto encode_one(google::protobuf::Arena& arena, uint8_t const* data) noexcept
	-> EncodeOneResult;

auto encode_one_special(google::protobuf::Arena& arena, IEncodeContext& context,
                        uint8_t const* data) noexcept -> EncodeOneResult;

} // namespace YGOpen::Codec::Edo9300::OCGCore

#endif // YGOPEN_CODEC_EDO9300_OCGCORE_ENCODE_HPP
