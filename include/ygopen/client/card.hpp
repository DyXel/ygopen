/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_CARD_HPP
#define YGOPEN_CLIENT_CARD_HPP
#include <cstdint>
#include <vector>
#include <ygopen/detail/config.hpp>
#include <ygopen/duel/constants_fwd.hpp>
#include <ygopen/proto/duel/data.hpp>

namespace YGOpen::Client
{

#ifdef YGOPEN_HAS_CONCEPTS
template<typename T>
concept CardTraits = requires(T)
{
	requires std::default_initializable<typename T::OwnerType>;
	requires std::default_initializable<typename T::IsPublicType>;
	requires std::default_initializable<typename T::IsHiddenType>;
	requires std::default_initializable<typename T::PositionType>;
	requires std::default_initializable<typename T::CoverType>;
	requires std::default_initializable<typename T::StatusType>;
	requires std::default_initializable<typename T::CodeType>;
	requires std::default_initializable<typename T::AliasType>;
	requires std::default_initializable<typename T::TypeType>;
	requires std::default_initializable<typename T::LevelType>;
	requires std::default_initializable<typename T::XyzRankType>;
	requires std::default_initializable<typename T::AttributeType>;
	requires std::default_initializable<typename T::RaceType>;
	requires std::default_initializable<typename T::AtkDefType>;
	requires std::default_initializable<typename T::PendLScaleType>;
	requires std::default_initializable<typename T::PendRScaleType>;
	requires std::default_initializable<typename T::LinkRateType>;
	requires std::default_initializable<typename T::LinkArrowType>;
	requires std::default_initializable<typename T::CountersType>;
	requires std::default_initializable<typename T::EquippedType>;
	requires std::default_initializable<typename T::RelationsType>;
};
#endif // YGOPEN_HAS_CONCEPTS

template<YGOPEN_CONCEPT(CardTraits)>
class BasicCard2
{
public:
	using OwnerType = typename CardTraits::OwnerType;
	using IsPublicType = typename CardTraits::IsPublicType;
	using IsHiddenType = typename CardTraits::IsHiddenType;
	using PositionType = typename CardTraits::PositionType;
	using CoverType = typename CardTraits::CoverType;
	using StatusType = typename CardTraits::StatusType;
	using CodeType = typename CardTraits::CodeType;
	using AliasType = typename CardTraits::AliasType;
	using TypeType = typename CardTraits::TypeType;
	using LevelType = typename CardTraits::LevelType;
	using XyzRankType = typename CardTraits::XyzRankType;
	using AttributeType = typename CardTraits::AttributeType;
	using RaceType = typename CardTraits::RaceType;
	using AtkDefType = typename CardTraits::AtkDefType;
	using PendLScaleType = typename CardTraits::PendLScaleType;
	using PendRScaleType = typename CardTraits::PendRScaleType;
	using LinkRateType = typename CardTraits::LinkRateType;
	using LinkArrowType = typename CardTraits::LinkArrowType;
	using CountersType = typename CardTraits::CountersType;
	using EquippedType = typename CardTraits::EquippedType;
	using RelationsType = typename CardTraits::RelationsType;

	explicit constexpr BasicCard2() noexcept = default;

	inline ~BasicCard2() noexcept = default;
	BasicCard2(BasicCard2 const&) = delete;
	constexpr BasicCard2(BasicCard2&&) noexcept = default;
	auto operator=(BasicCard2 const&) -> BasicCard2& = delete;
	constexpr auto operator=(BasicCard2&&) noexcept -> BasicCard2& = default;

	// Const getters.

	[[nodiscard]] auto owner() const noexcept -> OwnerType const&
	{
		return owner_;
	}

	[[nodiscard]] auto is_public() const noexcept -> IsPublicType const&
	{
		return is_public_;
	}

	[[nodiscard]] auto is_hidden() const noexcept -> IsHiddenType const&
	{
		return is_hidden_;
	}

	[[nodiscard]] auto position() const noexcept -> PositionType const&
	{
		return position_;
	}

	[[nodiscard]] auto cover() const noexcept -> CoverType const&
	{
		return cover_;
	}

	[[nodiscard]] auto status() const noexcept -> StatusType const&
	{
		return status_;
	}

	[[nodiscard]] auto code() const noexcept -> CodeType const&
	{
		return code_;
	}

	[[nodiscard]] auto alias() const noexcept -> AliasType const&
	{
		return alias_;
	}

	[[nodiscard]] auto type() const noexcept -> TypeType const&
	{
		return type_;
	}

	[[nodiscard]] auto level() const noexcept -> LevelType const&
	{
		return level_;
	}

	[[nodiscard]] auto xyz_rank() const noexcept -> XyzRankType const&
	{
		return xyz_rank_;
	}

	[[nodiscard]] auto attribute() const noexcept -> AttributeType const&
	{
		return attribute_;
	}

	[[nodiscard]] auto race() const noexcept -> RaceType const&
	{
		return race_;
	}

	[[nodiscard]] auto base_atk() const noexcept -> AtkDefType const&
	{
		return base_atk_;
	}

	[[nodiscard]] auto atk() const noexcept -> AtkDefType const&
	{
		return atk_;
	}

	[[nodiscard]] auto base_def() const noexcept -> AtkDefType const&
	{
		return base_def_;
	}

	[[nodiscard]] auto def() const noexcept -> AtkDefType const&
	{
		return def_;
	}

	[[nodiscard]] auto pend_l_scale() const noexcept -> PendLScaleType const&
	{
		return pend_l_scale_;
	}

	[[nodiscard]] auto pend_r_scale() const noexcept -> PendRScaleType const&
	{
		return pend_r_scale_;
	}

	[[nodiscard]] auto link_rate() const noexcept -> LinkRateType const&
	{
		return link_rate_;
	}

	[[nodiscard]] auto link_arrow() const noexcept -> LinkArrowType const&
	{
		return link_arrow_;
	}

	[[nodiscard]] auto counters() const noexcept -> CountersType const&
	{
		return counters_;
	}

	[[nodiscard]] auto equipped() const noexcept -> EquippedType const&
	{
		return equipped_;
	}

	[[nodiscard]] auto relations() const noexcept -> RelationsType const&
	{
		return relations_;
	}

	// Non-const getters.

	[[nodiscard]] auto owner() noexcept -> OwnerType& { return owner_; }

	[[nodiscard]] auto is_public() noexcept -> IsPublicType&
	{
		return is_public_;
	}

	[[nodiscard]] auto is_hidden() noexcept -> IsHiddenType&
	{
		return is_hidden_;
	}

	[[nodiscard]] auto position() noexcept -> PositionType&
	{
		return position_;
	}

	[[nodiscard]] auto cover() noexcept -> CoverType& { return cover_; }

	[[nodiscard]] auto status() noexcept -> StatusType& { return status_; }

	[[nodiscard]] auto code() noexcept -> CodeType& { return code_; }

	[[nodiscard]] auto alias() noexcept -> AliasType& { return alias_; }

	[[nodiscard]] auto type() noexcept -> TypeType& { return type_; }

	[[nodiscard]] auto level() noexcept -> LevelType& { return level_; }

	[[nodiscard]] auto xyz_rank() noexcept -> XyzRankType& { return xyz_rank_; }

	[[nodiscard]] auto attribute() noexcept -> AttributeType&
	{
		return attribute_;
	}

	[[nodiscard]] auto race() noexcept -> RaceType& { return race_; }

	[[nodiscard]] auto base_atk() noexcept -> AtkDefType& { return base_atk_; }

	[[nodiscard]] auto atk() noexcept -> AtkDefType& { return atk_; }

	[[nodiscard]] auto base_def() noexcept -> AtkDefType& { return base_def_; }

	[[nodiscard]] auto def() noexcept -> AtkDefType& { return def_; }

	[[nodiscard]] auto pend_l_scale() noexcept -> PendLScaleType&
	{
		return pend_l_scale_;
	}

	[[nodiscard]] auto pend_r_scale() noexcept -> PendRScaleType&
	{
		return pend_r_scale_;
	}

	[[nodiscard]] auto link_rate() noexcept -> LinkRateType&
	{
		return link_rate_;
	}

	[[nodiscard]] auto link_arrow() noexcept -> LinkArrowType&
	{
		return link_arrow_;
	}

	[[nodiscard]] auto counters() noexcept -> CountersType&
	{
		return counters_;
	}

	[[nodiscard]] auto equipped() noexcept -> EquippedType&
	{
		return equipped_;
	}

	[[nodiscard]] auto relations() noexcept -> RelationsType&
	{
		return relations_;
	}

private:
	OwnerType owner_;
	IsPublicType is_public_;
	IsHiddenType is_hidden_;
	PositionType position_;
	CoverType cover_;
	StatusType status_;
	CodeType code_;
	AliasType alias_;
	TypeType type_;
	LevelType level_;
	XyzRankType xyz_rank_;
	AttributeType attribute_;
	RaceType race_;
	AtkDefType base_atk_;
	AtkDefType atk_;
	AtkDefType base_def_;
	AtkDefType def_;
	PendLScaleType pend_l_scale_;
	PendRScaleType pend_r_scale_;
	LinkRateType link_rate_;
	LinkArrowType link_arrow_;
	CountersType counters_;
	EquippedType equipped_;
	RelationsType relations_;
};

template<template<typename /*Wrapped*/> typename Wrapper>
struct BasicCard
{
	using OwnerType = Duel::Controller;
	using IsPublicType = bool;
	using IsHiddenType = bool;
	using PositionType = Duel::Position;
	using CoverType = uint32_t;
	using StatusType = Duel::Status;
	using CodeType = CoverType;
	using AliasType = CodeType;
	using TypeType = Duel::Type;
	using LevelType = uint32_t;
	using XyzRankType = uint32_t;
	using AttributeType = Duel::Attribute;
	using RaceType = Duel::Race;
	using AtkDefType = int32_t;
	using PendLScaleType = uint32_t;
	using PendRScaleType = PendLScaleType;
	using LinkRateType = uint32_t;
	using LinkArrowType = Duel::LinkArrow;
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
	BasicCard(BasicCard const&) = delete;
	constexpr BasicCard(BasicCard&&) noexcept = default;
	auto operator=(BasicCard const&) -> BasicCard& = delete;
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
