/*
 * Copyright (c) 2025, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_FRAME_LIMBO_HPP
#define YGOPEN_CLIENT_FRAME_LIMBO_HPP
#include <deque>
#include <variant>
#include <ygopen/client/frame.hpp>

// TODO: Unit test undo specialized operations.

namespace YGOpen::Client
{

template<typename Card, typename CardBuilder = Detail::DefaultBuilder<Card>>
class LimboFrame : public BasicFrame<Card, CardBuilder>
{
public:
	using BaseFrame = BasicFrame<Card, CardBuilder>;
	using PileType = typename BaseFrame::PileType;
	using PlaceType = typename BaseFrame::PlaceType;
	using FieldType = typename BaseFrame::FieldType;

	explicit constexpr LimboFrame(CardBuilder const& builder = CardBuilder())
		: BaseFrame(builder), op_(0U), processed_op_(0U)
	{}

	// Specialized operations.

	// Forget operations that happened after current operation. If at end (live)
	// then this is a no-op.
	constexpr auto erase_history() noexcept -> void
	{
		if(op_ == processed_op_)
			return;
		assert(op_ <= operations_.size());
		assert(op_ < processed_op_);
		// TODO: Destruct limbo cards.
		operations_.resize(op_);
		processed_op_ = op_;
	}

	constexpr auto card_add(PlaceType const& place) noexcept -> Card&
	{
		if(advance_op_())
		{
			auto& c = BaseFrame::card_add(place);
			emplace_op_<CardOp>().card = &c;
			return c;
		}
		auto& c = *current_op_as_card_();
		BaseFrame::card_insert(place, c);
		assert(this->has_card(place));
		return c;
	}

	constexpr auto card_remove(PlaceType const& place) noexcept -> void
	{
		assert(this->has_card(place));
		auto& c = BaseFrame::card_erase(place);
		if(advance_op_())
		{
			emplace_op_<CardOp>().card = &c;
			return;
		}
		assert(current_op_as_card_() == &c);
	}

	constexpr auto pile_resize(PlaceType const& place, size_t count) noexcept
		-> void
	{
		assert(is_pile(place));
		bool processing = advance_op_();
		auto& p = this->pile(place);
		auto& rsz_op = [&]() -> ResizeOp&
		{
			if(processing)
				return emplace_op_<ResizeOp>();
			assert(operations_.size() >= op_);
			assert(std::holds_alternative<ResizeOp>(operations_[op_ - 1]));
			return std::get<ResizeOp>(operations_[op_ - 1]);
		}();
		rsz_op.prev_size = p.size();
		if(p.size() < count)
		{
			if(processing)
			{
				for(size_t i = count - p.size(); i != 0; i--)
					p.push_back(&BaseFrame::construct_card());
			}
			else
			{
				p.insert(p.end(), rsz_op.slice.cbegin(), rsz_op.slice.cend());
			}
		}
		else if(p.size() > count)
		{
			size_t diff = p.size() - count;
			rsz_op.slice.resize(diff);
			std::copy(p.cbegin() + diff, p.cend(), rsz_op.slice.begin());
			p.resize(count);
		}
		assert(p.size() == count);
		this->verify_pile_(p);
	}

	constexpr auto clear() noexcept -> void
	{
		bool processing = advance_op_();
		auto& clear_op = [&]() -> ClearOp&
		{
			if(processing)
				return emplace_op_<ClearOp>();
			assert(operations_.size() >= op_);
			assert(std::holds_alternative<ClearOp>(operations_[op_ - 1]));
			return std::get<ClearOp>(operations_[op_ - 1]);
		}();
		swap_clear_op_(clear_op);
	}

	// Undo operations.

	constexpr auto undo_card_add(PlaceType const& place) noexcept -> Card&
	{
		assert(this->has_card(place));
		auto& c = BaseFrame::card_erase(place);
		assert(current_op_as_card_() == &c);
		regress_op_();
		return c;
	}

	constexpr auto undo_card_remove(PlaceType const& place) noexcept -> Card&
	{
		auto& c = *current_op_as_card_();
		BaseFrame::card_insert(place, c);
		regress_op_();
		assert(this->has_card(place));
		return c;
	}

	constexpr auto undo_pile_resize(PlaceType const& place,
	                                [[maybe_unused]] size_t count) noexcept
		-> void
	{
		assert(is_pile(place));
		auto& p = this->pile(place);
		auto& rsz_op = [&]() -> ResizeOp&
		{
			assert(operations_.size() >= op_);
			assert(std::holds_alternative<ResizeOp>(operations_[op_ - 1]));
			return std::get<ResizeOp>(operations_[op_ - 1]);
		}();
		if(p.size() < rsz_op.prev_size)
		{
			p.insert(p.end(), rsz_op.slice.cbegin(), rsz_op.slice.cend());
		}
		else if(p.size() > rsz_op.prev_size)
		{
			size_t diff = p.size() - rsz_op.prev_size;
			rsz_op.slice.resize(diff);
			std::copy(p.cbegin() + p.size() - diff, p.cend(),
			          rsz_op.slice.begin());
			p.resize(rsz_op.prev_size);
		}
		regress_op_();
		assert(p.size() == rsz_op.prev_size);
		this->verify_pile_(p);
	}

	constexpr auto undo_clear() noexcept -> void
	{
		auto& clear_op = [&]() -> ClearOp&
		{
			assert(operations_.size() >= op_);
			assert(std::holds_alternative<ClearOp>(operations_[op_ - 1]));
			return std::get<ClearOp>(operations_[op_ - 1]);
		}();
		swap_clear_op_(clear_op);
		regress_op_();
	}

	template<typename InputIt>
	constexpr auto undo_card_shuffle(InputIt previous, InputIt current,
	                                 size_t count) noexcept -> void
	{
		assert(false); // TODO
	}

private:
	using MultiPile = std::array<PileType, Duel::CONTROLLER_ARRAY_SIZE>;

	struct CardOp
	{
		Card* card;
	};

	struct ResizeOp
	{
		size_t prev_size;
		PileType slice;
	};

	struct ClearOp
	{
		MultiPile main_deck;
		MultiPile hand;
		MultiPile graveyard;
		MultiPile banished;
		MultiPile extra_deck;
		FieldType field;
	};

	using Ops = std::variant<CardOp, ResizeOp, ClearOp>;
	using OperationCountType = std::deque<Ops>::size_type;

	OperationCountType op_;
	OperationCountType processed_op_;
	std::deque<Ops> operations_;

	constexpr auto advance_op_() noexcept -> bool
	{
		return (op_++ == processed_op_) ? (processed_op_++, true) : false;
	}

	constexpr auto regress_op_() noexcept -> void
	{
		assert(op_ > 0);
		op_--;
	}

	constexpr auto swap_clear_op_(ClearOp& clear_op) noexcept -> void
	{
		using namespace YGOpen::Duel;
		auto swap_multi_pile = [this](Location loc, MultiPile& p)
		{
			for(auto con : {CONTROLLER_0, CONTROLLER_1})
				std::swap(this->pile(con, loc), p[con]);
		};
		swap_multi_pile(LOCATION_MAIN_DECK, clear_op.main_deck);
		swap_multi_pile(LOCATION_HAND, clear_op.hand);
		swap_multi_pile(LOCATION_GRAVEYARD, clear_op.graveyard);
		swap_multi_pile(LOCATION_BANISHED, clear_op.banished);
		swap_multi_pile(LOCATION_EXTRA_DECK, clear_op.extra_deck);
		std::swap(this->field(), clear_op.field);
	}

	constexpr auto current_op_as_card_() noexcept -> Card*&
	{
		assert(operations_.size() >= op_);
		assert(std::holds_alternative<CardOp>(operations_[op_ - 1]));
		return std::get<CardOp>(operations_[op_ - 1]).card;
	}

	template<typename T>
	constexpr auto emplace_op_() noexcept -> T&
	{
		return std::get<T>(operations_.emplace_back(std::in_place_type_t<T>{}));
	}
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_FRAME_LIMBO_HPP
