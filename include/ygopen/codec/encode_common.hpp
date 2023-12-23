/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CODEC_ENCODE_COMMON_HPP
#define YGOPEN_CODEC_ENCODE_COMMON_HPP
#include <cstddef> // size_t
#include <cstdint>
#include <vector>
#include <ygopen/duel/constants_fwd.hpp>

namespace google::protobuf
{

class Arena;

} // namespace google::protobuf

namespace YGOpen
{

namespace Proto::Duel
{

class Msg;
class Msg_Query_Data;
class Place;

} // namespace Proto::Duel

namespace Codec
{

struct EncodeOneQueryResult
{
	// "View" into the QUERY_OVERLAY_CARD array of the parsed query, if any.
	// Non-0 count means we have an array. Each card code is a uint32_t.
	uint32_t overlays_count;
	uint8_t const* overlays_ptr;
	size_t bytes_read;
};

struct EncodeOneResult
{
	enum class State
	{
		// The encoder doesn't know the message type.
		// `bytes_read` has a unspecified value.
		// `msg` is null.
		UNKNOWN,
		// Buffer was correctly encoded.
		// `bytes_read` has non-0 value.
		// `msg` is non-null.
		OK,
		// The encoder knows the message type but will not encode it because
		// encoding this message alone yields no useful information.
		// `bytes_read` has number of bytes that were read or skipped.
		// `msg` is null.
		SWALLOWED,
	} state;
	size_t bytes_read;
	Proto::Duel::Msg* msg;
};

class IEncodeContext
{
public:
	using Con = Duel::Controller;
	using Loc = Duel::Location;
	using Place = Proto::Duel::Place;

	// Get the number of cards in a certain pile.
	[[nodiscard]] virtual auto pile_size(Con, Loc) const noexcept -> size_t = 0;

	// Get the reason (if any) why the entire match was won.
	[[nodiscard]] virtual auto get_match_win_reason() const noexcept
		-> uint32_t = 0;

	// Check if a certain place on the field has materials.
	[[nodiscard]] virtual auto has_xyz_mat(Place const&) const noexcept
		-> bool = 0;

	// Get the Place set by `xyz_left`.
	[[nodiscard]] virtual auto get_xyz_left(Place const&) const noexcept
		-> Place = 0;

	// Set a reason why the entire match was won.
	virtual auto match_win_reason(uint32_t) noexcept -> void = 0;

	// A card is trying to attach itself as material to a place that is not in
	// the field.
	virtual auto xyz_mat_defer(Place const&) noexcept -> void = 0;

	// Get all places set with `xyz_mat_defer` and clear internal list.
	virtual auto take_deferred_xyz_mat() noexcept -> std::vector<Place> = 0;

	// A card that had xyz materials left the field to a different place.
	virtual auto xyz_left(Place const&, Place const&) noexcept -> void = 0;

protected:
	~IEncodeContext() noexcept = default;
};

} // namespace Codec

} // namespace YGOpen

#endif // YGOPEN_CODEC_ENCODE_COMMON_HPP
