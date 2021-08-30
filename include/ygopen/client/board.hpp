/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_BOARD_HPP
#define YGOPEN_CLIENT_BOARD_HPP
#include <ygopen/duel/constants_fwd.hpp>
#include <ygopen/proto/duel/msg.hpp>

namespace YGOpen::Client
{

template<typename Frame, template<typename /*Wrapped*/> typename Wrapper>
class BasicBoard
{
public:
	using ChainStackType = std::vector<Proto::Duel::Chain>;
	using TurnType = uint32_t;
	using TurnControllerType = Duel::Controller;
	using LPType = uint32_t;
	using PhaseType = Duel::Phase;
	using BlockedZonesType = std::vector<Proto::Duel::Place>;

	explicit constexpr BasicBoard() noexcept = default;

	template<typename CardBuilder>
	explicit constexpr BasicBoard(CardBuilder const& cb) noexcept
		: frame_(cb){};

	template<typename CardBuilder, typename WrapperBuilder>
	explicit constexpr BasicBoard(CardBuilder const& cb,
	                              WrapperBuilder const& wb) noexcept
		: frame_(cb)
		, chain_stack_(wb.template build<ChainStackType>())
		, turn_(wb.template build<TurnType>())
		, turn_controller_(wb.template build<TurnControllerType>())
		, lp_({wb.template build<LPType>(), wb.template build<LPType>()})
		, phase_(wb.template build<PhaseType>())
		, blocked_zones_(wb.template build<BlockedZonesType>()){};

	// Const getters.

	constexpr auto frame() const noexcept -> Frame const& { return frame_; }

	constexpr auto chain_stack() const noexcept
		-> Wrapper<ChainStackType> const&
	{
		return chain_stack_;
	}

	constexpr auto turn() const noexcept -> Wrapper<TurnType> const&
	{
		return turn_;
	}

	constexpr auto turn_controller() const noexcept
		-> Wrapper<TurnControllerType> const&
	{
		return turn_controller_;
	}

	constexpr auto lp(YGOpen::Duel::Controller con) const noexcept
		-> Wrapper<LPType> const&
	{
		return lp_[static_cast<size_t>(con)];
	}

	constexpr auto phase() const noexcept -> Wrapper<PhaseType> const&
	{
		return phase_;
	}

	constexpr auto blocked_zones() const noexcept
		-> Wrapper<BlockedZonesType> const&
	{
		return blocked_zones_;
	}

	// Non-const getters.

	constexpr auto frame() noexcept -> Frame& { return frame_; }

	constexpr auto chain_stack() noexcept -> Wrapper<ChainStackType>&
	{
		return chain_stack_;
	}

	constexpr auto turn() noexcept -> Wrapper<TurnType>& { return turn_; }

	constexpr auto turn_controller() noexcept -> Wrapper<TurnControllerType>&
	{
		return turn_controller_;
	}

	constexpr auto lp(YGOpen::Duel::Controller con) noexcept -> Wrapper<LPType>&
	{
		return lp_[static_cast<size_t>(con)];
	}

	constexpr auto phase() noexcept -> Wrapper<PhaseType>& { return phase_; }

	constexpr auto blocked_zones() noexcept -> Wrapper<BlockedZonesType>&
	{
		return blocked_zones_;
	}

private:
	Frame frame_;
	Wrapper<ChainStackType> chain_stack_;
	Wrapper<TurnType> turn_;
	Wrapper<TurnControllerType> turn_controller_;
	std::array<Wrapper<LPType>, 2U> lp_;
	Wrapper<PhaseType> phase_;
	Wrapper<BlockedZonesType> blocked_zones_;
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_BOARD_HPP
