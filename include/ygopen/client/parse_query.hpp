/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_PARSE_QUERY_HPP
#define YGOPEN_CLIENT_PARSE_QUERY_HPP
#include <limits>
#include <ygopen/proto/duel/msg.hpp>

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

constexpr auto operator&(QueryCacheHit lhs,
                         QueryCacheHit rhs) noexcept -> QueryCacheHit
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

constexpr auto operator|=(QueryCacheHit& lhs,
                          QueryCacheHit rhs) noexcept -> QueryCacheHit&
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
[[nodiscard]] auto parse_query(Frame& frame,
                               Proto::Duel::Msg::Query const& query) noexcept
	-> QueryCacheHit
{
	auto hits = QueryCacheHit::UNSPECIFIED;
	assert(frame.has_card(query.place()));
	auto const& data = query.data();
	auto& card = frame.card(query.place());
	// NOLINTNEXTLINE: No reflection :(
#define X(t, v, q)                                         \
	do                                                     \
	{                                                      \
		if(data.has_##v())                                 \
		{                                                  \
			using ValueType = typename Frame::CardType::t; \
			auto const qcv = ValueType{data.v().value()};  \
			if constexpr(use_cache)                        \
			{                                              \
				if(card.v() == qcv)                        \
					hits |= (QueryCacheHit::q);            \
				else                                       \
					card.v() = qcv;                        \
			}                                              \
			else                                           \
			{                                              \
				card.v() = qcv;                            \
			}                                              \
		}                                                  \
	} while(0)
	X(OwnerType, owner, OWNER);
	X(IsPublicType, is_public, IS_PUBLIC);
	X(IsHiddenType, is_hidden, IS_HIDDEN);
	X(PositionType, position, POSITION);
	X(CodeType, cover, COVER);
	X(StatusType, status, STATUS);
	X(CodeType, code, CODE);
	X(CodeType, alias, ALIAS);
	X(TypeType, type, TYPE);
	X(LevelType, level, LEVEL);
	X(XyzRankType, xyz_rank, XYZ_RANK);
	X(AttributeType, attribute, ATTRIBUTE);
	X(RaceType, race, RACE);
	X(AtkDefType, base_atk, BASE_ATK);
	X(AtkDefType, atk, ATK);
	X(AtkDefType, base_def, BASE_DEF);
	X(AtkDefType, def, DEF);
	X(PendScalesType, pend_l_scale, PEND_L_SCALE);
	X(PendScalesType, pend_r_scale, PEND_R_SCALE);
	X(LinkRateType, link_rate, LINK_RATE);
	X(LinkArrowType, link_arrow, LINK_ARROW);
#undef X
	if(data.has_counters())
	{
		auto const& rf = data.counters().values();
		card.counters().assign(rf.cbegin(), rf.cend());
	}
	if(data.has_equipped())
	{
		card.equipped() = data.equipped().value();
	}
	if(data.has_relations())
	{
		auto const& rf = data.relations().values();
		card.relations().assign(rf.cbegin(), rf.cend());
	}
	return hits;
}

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_PARSE_QUERY_HPP
