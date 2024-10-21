/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_CARD_HPP
#define YGOPEN_CLIENT_CARD_HPP
#include <ygopen/detail/config.hpp>

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
	requires std::default_initializable<typename T::StatusType>;
	requires std::default_initializable<typename T::CodeType>;
	requires std::default_initializable<typename T::AliasType>;
	requires std::default_initializable<typename T::TypeType>;
	requires std::default_initializable<typename T::LevelType>;
	requires std::default_initializable<typename T::XyzRankType>;
	requires std::default_initializable<typename T::AttributeType>;
	requires std::default_initializable<typename T::RaceType>;
	requires std::default_initializable<typename T::AtkDefType>;
	requires std::default_initializable<typename T::PendScalesType>;
	requires std::default_initializable<typename T::LinkRateType>;
	requires std::default_initializable<typename T::LinkArrowType>;
	requires std::default_initializable<typename T::CountersType>;
	requires std::default_initializable<typename T::EquippedType>;
	requires std::default_initializable<typename T::RelationsType>;
};
#endif // YGOPEN_HAS_CONCEPTS

template<YGOPEN_CONCEPT(CardTraits)>
class BasicCard
{
public:
	using OwnerType = typename CardTraits::OwnerType;
	using IsPublicType = typename CardTraits::IsPublicType;
	using IsHiddenType = typename CardTraits::IsHiddenType;
	using PositionType = typename CardTraits::PositionType;
	using StatusType = typename CardTraits::StatusType;
	using CodeType = typename CardTraits::CodeType;
	using TypeType = typename CardTraits::TypeType;
	using LevelType = typename CardTraits::LevelType;
	using XyzRankType = typename CardTraits::XyzRankType;
	using AttributeType = typename CardTraits::AttributeType;
	using RaceType = typename CardTraits::RaceType;
	using AtkDefType = typename CardTraits::AtkDefType;
	using PendScalesType = typename CardTraits::PendScalesType;
	using LinkRateType = typename CardTraits::LinkRateType;
	using LinkArrowType = typename CardTraits::LinkArrowType;
	using CountersType = typename CardTraits::CountersType;
	using EquippedType = typename CardTraits::EquippedType;
	using RelationsType = typename CardTraits::RelationsType;

	explicit constexpr BasicCard() noexcept
		: owner_()
		, is_public_()
		, is_hidden_()
		, position_()
		, cover_()
		, status_()
		, code_()
		, alias_()
		, type_()
		, level_()
		, xyz_rank_()
		, attribute_()
		, race_()
		, base_atk_()
		, atk_()
		, base_def_()
		, def_()
		, pend_l_scale_()
		, pend_r_scale_()
		, link_rate_()
		, link_arrow_()
		, counters_()
		, equipped_()
		, relations_() {};

	inline ~BasicCard() noexcept = default;
	BasicCard(BasicCard const&) = delete;
	constexpr BasicCard(BasicCard&&) noexcept = default;
	auto operator=(BasicCard const&) -> BasicCard& = delete;
	constexpr auto operator=(BasicCard&&) noexcept -> BasicCard& = default;

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

	[[nodiscard]] auto cover() const noexcept -> CodeType const&
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

	[[nodiscard]] auto alias() const noexcept -> CodeType const&
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

	[[nodiscard]] auto pend_l_scale() const noexcept -> PendScalesType const&
	{
		return pend_l_scale_;
	}

	[[nodiscard]] auto pend_r_scale() const noexcept -> PendScalesType const&
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

	[[nodiscard]] auto cover() noexcept -> CodeType& { return cover_; }

	[[nodiscard]] auto status() noexcept -> StatusType& { return status_; }

	[[nodiscard]] auto code() noexcept -> CodeType& { return code_; }

	[[nodiscard]] auto alias() noexcept -> CodeType& { return alias_; }

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

	[[nodiscard]] auto pend_l_scale() noexcept -> PendScalesType&
	{
		return pend_l_scale_;
	}

	[[nodiscard]] auto pend_r_scale() noexcept -> PendScalesType&
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
	CodeType cover_;
	StatusType status_;
	CodeType code_;
	CodeType alias_;
	TypeType type_;
	LevelType level_;
	XyzRankType xyz_rank_;
	AttributeType attribute_;
	RaceType race_;
	AtkDefType base_atk_;
	AtkDefType atk_;
	AtkDefType base_def_;
	AtkDefType def_;
	PendScalesType pend_l_scale_;
	PendScalesType pend_r_scale_;
	LinkRateType link_rate_;
	LinkArrowType link_arrow_;
	CountersType counters_;
	EquippedType equipped_;
	RelationsType relations_;
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_CARD_HPP
