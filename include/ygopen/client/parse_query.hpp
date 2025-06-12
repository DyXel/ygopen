/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_PARSE_QUERY_HPP
#define YGOPEN_CLIENT_PARSE_QUERY_HPP
#include <limits>
#include <ygopen/proto/duel/data.hpp>
#include <ygopen/proto/duel/msg.hpp>

// TODO: Maybe refactor this and separate Undoable parsing implementation and
//       "cached" implementation as well to reduce complexity? Also get rid of
//       strong-typed enum boilerplate. Doesn't seem to bring much value.

namespace YGOpen::Client
{

enum class QueryCacheHit : uint64_t
{
	UNSPECIFIED = 0x0,
#define X(NAME, Name, name, value) NAME = value,
#define EXPAND_ARRAY_LIKE_QUERIES
#define EXPAND_SEPARATE_LINK_DATA_QUERIES
#include <ygopen/client/queries.inl>
#undef EXPAND_SEPARATE_LINK_DATA_QUERIES
#undef EXPAND_ARRAY_LIKE_QUERIES
#undef X
};

constexpr auto operator&(QueryCacheHit lhs, QueryCacheHit rhs) noexcept
	-> QueryCacheHit
{
	return static_cast<QueryCacheHit>(
		static_cast<std::underlying_type<QueryCacheHit>::type>(lhs) &
		static_cast<std::underlying_type<QueryCacheHit>::type>(rhs));
}

constexpr auto operator~(QueryCacheHit rhs) noexcept -> QueryCacheHit
{
	return static_cast<QueryCacheHit>(
		~static_cast<std::underlying_type<QueryCacheHit>::type>(rhs));
}

constexpr auto operator|=(QueryCacheHit& lhs, QueryCacheHit rhs) noexcept
	-> QueryCacheHit&
{
	lhs = static_cast<QueryCacheHit>(
		static_cast<std::underlying_type<QueryCacheHit>::type>(lhs) |
		static_cast<std::underlying_type<QueryCacheHit>::type>(rhs));
	return lhs;
}

constexpr auto operator!(QueryCacheHit rhs) noexcept -> bool
{
	return rhs == QueryCacheHit::UNSPECIFIED;
}

namespace Detail
{

template<typename T, typename = void>
struct ValueTypeSelector
{
	using Type = T;
};

template<typename T>
struct ValueTypeSelector<T, std::void_t<typename T::ValueType>>
{
	using Type = typename T::ValueType;
};

template<typename T>
using ValueTypeOrT = typename ValueTypeSelector<T>::Type;

} // namespace Detail

template<bool use_cache = false, typename Frame>
[[nodiscard]] auto parse_query(Frame& frame,
                               Proto::Duel::Msg::Query const& query) noexcept
	-> QueryCacheHit
{
	auto hits = QueryCacheHit::UNSPECIFIED;
	assert(frame.has_card(query.place()));
	auto const& data = query.data();
	auto& card = frame.card(query.place());
#define X(NAME, Name, name, value_)                                     \
	if(data.has_##name())                                               \
	{                                                                   \
		using ValueType =                                               \
			Detail::ValueTypeOrT<typename Frame::CardType::Name##Type>; \
		auto const qv = ValueType{data.name().value()};                 \
		if constexpr(use_cache)                                         \
		{                                                               \
			if(card.name() == qv)                                       \
				hits |= (QueryCacheHit::NAME);                          \
			else                                                        \
				card.name() = qv;                                       \
		}                                                               \
		else                                                            \
		{                                                               \
			card.name() = qv;                                           \
		}                                                               \
	}
#define EXPAND_SEPARATE_LINK_DATA_QUERIES
#include <ygopen/client/queries.inl>
#undef EXPAND_SEPARATE_LINK_DATA_QUERIES
#undef X
	// TODO: Cache it up
	if(data.has_counters())
	{
		auto const& rf = data.counters().values();
		card.counters().assign(rf.cbegin(), rf.cend());
	}
	if(data.has_targets())
	{
		auto const& rf = data.targets().values();
		card.targets().assign(rf.cbegin(), rf.cend());
	}
	return hits;
}

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_PARSE_QUERY_HPP
