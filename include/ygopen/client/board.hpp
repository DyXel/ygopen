/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_BOARD_HPP
#define YGOPEN_CLIENT_BOARD_HPP
#include <ygopen/detail/config.hpp>
#include <ygopen/duel/constants/controller.hpp>

namespace YGOpen::Client
{

#ifdef YGOPEN_HAS_CONCEPTS
template<typename T>
concept BoardTraits = requires(T)
{
	requires std::default_initializable<typename T::BlockedZonesType>;
	requires std::default_initializable<typename T::ChainStackType>;
	requires std::default_initializable<typename T::FrameType>;
	requires std::default_initializable<typename T::LPType>;
	requires std::default_initializable<typename T::PhaseType>;
	requires std::default_initializable<typename T::TurnControllerType>;
	requires std::default_initializable<typename T::TurnType>;
};
#endif // YGOPEN_HAS_CONCEPTS

template<YGOPEN_CONCEPT(BoardTraits)>
class BasicBoard
{
public:
	using BlockedZonesType = typename BoardTraits::BlockedZonesType;
	using ChainStackType = typename BoardTraits::ChainStackType;
	using FrameType = typename BoardTraits::FrameType;
	using LPType = typename BoardTraits::LPType;
	using PhaseType = typename BoardTraits::PhaseType;
	using TurnControllerType = typename BoardTraits::TurnControllerType;
	using TurnType = typename BoardTraits::TurnType;

	explicit constexpr BasicBoard() noexcept
		: frame_()
		, chain_stack_()
		, turn_()
		, turn_controller_()
		, lp_()
		, phase_()
		, blocked_zones_(){};

	// Const getters.

	[[nodiscard]] constexpr auto frame() const noexcept -> FrameType const&
	{
		return frame_;
	}

	[[nodiscard]] constexpr auto chain_stack() const noexcept
		-> ChainStackType const&
	{
		return chain_stack_;
	}

	[[nodiscard]] constexpr auto turn() const noexcept -> TurnType const&
	{
		return turn_;
	}

	[[nodiscard]] constexpr auto turn_controller() const noexcept
		-> TurnControllerType const&
	{
		return turn_controller_;
	}

	[[nodiscard]] constexpr auto lp(YGOpen::Duel::Controller con) const noexcept
		-> LPType const&
	{
		return lp_[static_cast<size_t>(con)];
	}

	[[nodiscard]] constexpr auto phase() const noexcept -> PhaseType const&
	{
		return phase_;
	}

	[[nodiscard]] constexpr auto blocked_zones() const noexcept
		-> BlockedZonesType const&
	{
		return blocked_zones_;
	}

	// Non-const getters.

	[[nodiscard]] constexpr auto frame() noexcept -> FrameType&
	{
		return frame_;
	}

	[[nodiscard]] constexpr auto chain_stack() noexcept -> ChainStackType&
	{
		return chain_stack_;
	}

	[[nodiscard]] constexpr auto turn() noexcept -> TurnType& { return turn_; }

	[[nodiscard]] constexpr auto turn_controller() noexcept
		-> TurnControllerType&
	{
		return turn_controller_;
	}

	[[nodiscard]] constexpr auto lp(YGOpen::Duel::Controller con) noexcept
		-> LPType&
	{
		auto const who = static_cast<size_t>(con);
		assert(who <= lp_.size());
		return lp_[who];
	}

	[[nodiscard]] constexpr auto phase() noexcept -> PhaseType&
	{
		return phase_;
	}

	[[nodiscard]] constexpr auto blocked_zones() noexcept -> BlockedZonesType&
	{
		return blocked_zones_;
	}

private:
	FrameType frame_;
	ChainStackType chain_stack_;
	TurnType turn_;
	TurnControllerType turn_controller_;
	std::array<LPType, 2U> lp_;
	PhaseType phase_;
	BlockedZonesType blocked_zones_;
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_BOARD_HPP
