/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <cstdint>
#include <vector>
#include <ygopen/duel/constants_fwd.hpp>
#include <ygopen/proto/duel/data.hpp>

namespace YGOpen::Client
{

namespace Detail
{

// Admitely, this is a bit overkill, but writing it down documents the pattern
// for reuse later.

template<uint64_t Query>
struct DefaultCardTrait
{
	using Type = void;
};

template<>
struct DefaultCardTrait<0x1>
{
	using Type = uint32_t;
};

template<>
struct DefaultCardTrait<0x2>
{
	using Type = YGOpen::Duel::Position;
};

template<>
struct DefaultCardTrait<0x4>
{
	using Type = typename DefaultCardTrait<0x1>::Type;
};

template<>
struct DefaultCardTrait<0x8>
{
	using Type = YGOpen::Duel::Type;
};

template<>
struct DefaultCardTrait<0x10>
{
	using Type = int32_t;
};

template<>
struct DefaultCardTrait<0x20>
{
	using Type = uint32_t;
};

template<>
struct DefaultCardTrait<0x40>
{
	using Type = YGOpen::Duel::Attribute;
};

template<>
struct DefaultCardTrait<0x80>
{
	using Type = YGOpen::Duel::Race;
};

template<>
struct DefaultCardTrait<0x100>
{
	using Type = int32_t;
};

template<>
struct DefaultCardTrait<0x200>
{
	using Type = typename DefaultCardTrait<0x100>::Type;
};

template<>
struct DefaultCardTrait<0x400>
{
	using Type = typename DefaultCardTrait<0x100>::Type;
};

template<>
struct DefaultCardTrait<0x800>
{
	using Type = typename DefaultCardTrait<0x100>::Type;
};

template<>
struct DefaultCardTrait<0x4000>
{
	using Type = YGOpen::Proto::Duel::Place;
};

template<>
struct DefaultCardTrait<0x8000>
{
	using Type = std::vector<YGOpen::Proto::Duel::Place>;
};

template<>
struct DefaultCardTrait<0x20000>
{
	using Type = std::vector<YGOpen::Proto::Duel::Counter>;
};

template<>
struct DefaultCardTrait<0x40000>
{
	using Type = YGOpen::Duel::Controller;
};

template<>
struct DefaultCardTrait<0x80000>
{
	using Type = YGOpen::Duel::Status;
};

template<>
struct DefaultCardTrait<0x100000>
{
	using Type = bool;
};

template<>
struct DefaultCardTrait<0x200000>
{
	using Type = uint32_t;
};

template<>
struct DefaultCardTrait<0x400000>
{
	using Type = typename DefaultCardTrait<0x200000>::Type;
};

template<>
struct DefaultCardTrait<0x1000000>
{
	using Type = bool;
};

template<>
struct DefaultCardTrait<0x2000000>
{
	using Type = typename DefaultCardTrait<0x1>::Type;
};

template<>
struct DefaultCardTrait<0x100000000>
{
	using Type = uint32_t;
};

template<>
struct DefaultCardTrait<0x200000000>
{
	using Type = YGOpen::Duel::LinkArrow;
};

} // namespace Detail

struct DefaultCardTraits
{
#define X(NAME, Name, name, value) \
	using Name##Type = typename Detail::DefaultCardTrait<(value)>::Type;
#define EXPAND_ARRAY_LIKE_QUERIES
#define EXPAND_SEPARATE_LINK_DATA_QUERIES
#include <ygopen/client/queries.inl>
#undef EXPAND_SEPARATE_LINK_DATA_QUERIES
#undef EXPAND_ARRAY_LIKE_QUERIES
#undef X
};

} // namespace YGOpen::Client
