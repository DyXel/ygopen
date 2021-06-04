/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_CARD_HPP
#define YGOPEN_CLIENT_CARD_HPP
#include <cstdint>
#include <vector>

#include "ygopen/proto/duel/data.hpp"

namespace YGOpen::Client
{

template<template<typename /*Wrapped*/> typename Wrapper>
struct BasicCard
{
	using OwnerType = Proto::Duel::Controller;
	using IsPublicType = bool;
	using IsHiddenType = bool;
	using PositionType = Proto::Duel::Position;
	using CoverType = uint32_t;
	using StatusType = Proto::Duel::Status;
	using CodeType = CoverType;
	using AliasType = CodeType;
	using TypeType = Proto::Duel::Type;
	using LevelType = uint32_t;
	using XyzRankType = uint32_t;
	using AttributeType = Proto::Duel::Attribute;
	using RaceType = Proto::Duel::Race;
	using AtkDefType = int32_t;
	using PendLScaleType = uint32_t;
	using PendRScaleType = PendLScaleType;
	using LinkRateType = uint32_t;
	using LinkArrowType = Proto::Duel::LinkArrow;
	using CountersType = std::vector<Proto::Duel::Counter>;
	using EquippedType = Proto::Duel::Place;
	using RelationsType = std::vector<Proto::Duel::Place>;

	explicit constexpr BasicCard() noexcept = default;

	template<typename WrapperBuilder>
	explicit constexpr BasicCard(WrapperBuilder const& wb) noexcept
		: owner(wb.template build<OwnerType>())
		, is_public(wb.template build<IsPublicType>())
		, is_hidden(wb.template build<IsHiddenType>())
		, position(wb.template build<PositionType>())
		, cover(wb.template build<CoverType>())
		, status(wb.template build<StatusType>())
		, code(wb.template build<CodeType>())
		, alias(wb.template build<AliasType>())
		, type(wb.template build<TypeType>())
		, level(wb.template build<LevelType>())
		, xyz_rank(wb.template build<XyzRankType>())
		, attribute(wb.template build<AttributeType>())
		, race(wb.template build<RaceType>())
		, base_atk(wb.template build<AtkDefType>())
		, atk(wb.template build<AtkDefType>())
		, base_def(wb.template build<AtkDefType>())
		, def(wb.template build<AtkDefType>())
		, pend_l_scale(wb.template build<PendLScaleType>())
		, pend_r_scale(wb.template build<PendRScaleType>())
		, link_rate(wb.template build<LinkRateType>())
		, link_arrow(wb.template build<LinkArrowType>())
		, counters(wb.template build<CountersType>())
		, equipped(wb.template build<EquippedType>())
		, relations(wb.template build<RelationsType>())
	{}

	inline ~BasicCard() noexcept = default;
	BasicCard(const BasicCard&) = delete;
	constexpr BasicCard(BasicCard&&) noexcept = default;
	auto operator=(const BasicCard&) -> BasicCard& = delete;
	constexpr auto operator=(BasicCard&&) noexcept -> BasicCard& = default;

	// Query data.
	Wrapper<OwnerType> owner;
	Wrapper<IsPublicType> is_public;
	Wrapper<IsHiddenType> is_hidden;
	Wrapper<PositionType> position;
	Wrapper<CoverType> cover;
	Wrapper<StatusType> status;
	Wrapper<CodeType> code;
	Wrapper<AliasType> alias;
	Wrapper<TypeType> type;
	Wrapper<LevelType> level;
	Wrapper<XyzRankType> xyz_rank;
	Wrapper<AttributeType> attribute;
	Wrapper<RaceType> race;
	Wrapper<AtkDefType> base_atk;
	Wrapper<AtkDefType> atk;
	Wrapper<AtkDefType> base_def;
	Wrapper<AtkDefType> def;
	Wrapper<PendLScaleType> pend_l_scale;
	Wrapper<PendRScaleType> pend_r_scale;
	Wrapper<LinkRateType> link_rate;
	Wrapper<LinkArrowType> link_arrow;
	Wrapper<CountersType> counters;
	Wrapper<EquippedType> equipped;
	Wrapper<RelationsType> relations;
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_CARD_HPP
