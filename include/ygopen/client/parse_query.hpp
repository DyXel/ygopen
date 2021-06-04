/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_PARSE_QUERY_HPP
#define YGOPEN_CLIENT_PARSE_QUERY_HPP
#include <limits>

#include "ygopen/proto/duel/msg.hpp"

namespace YGOpen::Client
{

enum class QueryCacheHit : unsigned
{
	UNSPECIFIED = 0x0U,
	OWNER = 0x1U,
	IS_PUBLIC = 0x2U,
	IS_HIDDEN = 0x4U,
	POSITION = 0x8U,
	COVER = 0x10U,
	STATUS = 0x20U,
	CODE = 0x40U,
	ALIAS = 0x80U,
	TYPE = 0x100U,
	LEVEL = 0x200U,
	XYZ_RANK = 0x400U,
	ATTRIBUTE = 0x800U,
	RACE = 0x1000U,
	BASE_ATK = 0x2000U,
	ATK = 0x4000U,
	BASE_DEF = 0x8000U,
	DEF = 0x10000U,
	PEND_L_SCALE = 0x20000U,
	PEND_R_SCALE = 0x40000U,
	LINK_RATE = 0x80000U,
	LINK_ARROW = 0x100000U,
	COUNTERS = 0x200000U,
	EQUIPPED = 0x400000U,
	RELATIONS = 0x800000U,
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

template<bool use_cache = false, typename Frame>
auto parse_query(Frame& frame, Proto::Duel::Msg::Query const& query) noexcept
	-> QueryCacheHit
{
	auto hits = QueryCacheHit::UNSPECIFIED;
	assert(frame.has_card(query.place()));
	auto const& data = query.data();
	auto& card = frame.card(query.place());
	// NOLINTNEXTLINE: No reflection :(
#define X(v, q)                                      \
	do                                               \
	{                                                \
		if(data.has_##v())                           \
		{                                            \
			if constexpr(use_cache)                  \
			{                                        \
				if(card.v.get() == data.v().value()) \
					hits |= (QueryCacheHit::q);      \
				else                                 \
					card.v.set(data.v().value());    \
			}                                        \
			else                                     \
			{                                        \
				card.v.set(data.v().value());        \
			}                                        \
		}                                            \
	} while(0)
	X(owner, OWNER);
	X(is_public, IS_PUBLIC);
	X(is_hidden, IS_HIDDEN);
	X(position, POSITION);
	X(cover, COVER);
	X(status, STATUS);
	X(code, CODE);
	X(alias, ALIAS);
	X(type, TYPE);
	X(level, LEVEL);
	X(xyz_rank, XYZ_RANK);
	X(attribute, ATTRIBUTE);
	X(race, RACE);
	X(base_atk, BASE_ATK);
	X(atk, ATK);
	X(base_def, BASE_DEF);
	X(def, DEF);
	X(pend_l_scale, PEND_L_SCALE);
	X(pend_r_scale, PEND_R_SCALE);
	X(link_rate, LINK_RATE);
	X(link_arrow, LINK_ARROW);
	if(data.has_counters())
	{
		auto const& rf = data.counters().values();
		card.counters.set({rf.cbegin(), rf.cend()});
	}
	if(data.has_equipped())
	{
		card.equipped.set(data.equipped().value());
	}
	if(data.has_relations())
	{
		auto const& rf = data.relations().values();
		card.relations.set({rf.cbegin(), rf.cend()});
	}
#undef X
	return hits;
}

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_PARSE_QUERY_HPP
