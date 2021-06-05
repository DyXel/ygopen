/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_FRAME_HPP
#define YGOPEN_CLIENT_FRAME_HPP
#include <algorithm>
#include <array>
#include <cassert>
#include <forward_list>
#include <vector>

#include "ygopen/proto/duel/data.hpp"

namespace YGOpen::Client
{

namespace Detail
{

template<typename T>
class DefaultBuilder
{
	static_assert(std::is_default_constructible_v<T>);

public:
	explicit constexpr DefaultBuilder() noexcept = default;

	static constexpr auto build() noexcept -> T { return T{}; };
};

} // namespace Detail

enum FieldZoneLimit : size_t
{
	FIELD_ZONE_LIMIT_MONSTER = 7U,
	FIELD_ZONE_LIMIT_SPELL = 5U,
	FIELD_ZONE_LIMIT_FIELD = 1U,
	FIELD_ZONE_LIMIT_PENDULUM = 2U,
	FIELD_ZONE_LIMIT_SKILL = 1U,
};

template<typename Card, typename CardBuilder = Detail::DefaultBuilder<Card>>
class BasicFrame
{
public:
	using PileType = std::vector<Card*>;
	using PlaceType = Proto::Duel::Place;

	class Side
	{
	public:
		struct Zone
		{
			Card* card{};
			PileType materials;
		};

		constexpr auto operator[](Proto::Duel::Location loc) const noexcept
			-> Zone const*
		{
			return at_(*this, loc);
		}

		constexpr auto operator[](Proto::Duel::Location loc) noexcept -> Zone*
		{
			return at_(*this, loc);
		}

	private:
		friend BasicFrame;
		std::array<Zone, FIELD_ZONE_LIMIT_MONSTER> monster_;
		std::array<Zone, FIELD_ZONE_LIMIT_SPELL> spell_;
		std::array<Zone, FIELD_ZONE_LIMIT_FIELD> field_;
		std::array<Zone, FIELD_ZONE_LIMIT_PENDULUM> pendulum_;
		std::array<Zone, FIELD_ZONE_LIMIT_SKILL> skill_;

		// FIXME: Use std::span as return type if we ever move to >=C++20.
		template<typename T>
		static constexpr auto at_(T& t, Proto::Duel::Location loc) noexcept
			-> auto*
		{
			using namespace YGOpen::Proto::Duel;
			if(loc == LOCATION_MONSTER_ZONE)
				return t.monster_.data();
			if(loc == LOCATION_SPELL_ZONE)
				return t.spell_.data();
			if(loc == LOCATION_FIELD_ZONE)
				return t.field_.data();
			if(loc == LOCATION_PENDULUM_ZONE)
				return t.pendulum_.data();
			if(loc == LOCATION_SKILL_ZONE)
				return t.skill_.data();
#ifdef _MSC_VER
			__assume(0);
#else
			__builtin_unreachable();
#endif // _MSC_VER
		}
	};

	using FieldType = std::array<Side, Proto::Duel::Controller_ARRAYSIZE>;

	explicit constexpr BasicFrame(
		CardBuilder const& builder = CardBuilder()) noexcept
		: cards_(builder)
	{}

	// Const getters.

	constexpr auto builder() const noexcept -> CardBuilder const&
	{
		return cards_;
	}

	[[nodiscard]] constexpr auto has_card(PlaceType const& place) const noexcept
		-> bool
	{
		if(is_pile(place))
			return place.seq() < pile(place.con(), place.loc()).size();
		auto& z = zone(place);
		if(place.oseq() < 0)
			return z.card != nullptr;
		return static_cast<size_t>(place.oseq()) < z.materials.size();
	}

	constexpr auto pile(Proto::Duel::Controller con,
	                    Proto::Duel::Location loc) const noexcept
		-> const PileType&
	{
		return pile_(*this, con, loc);
	}

	constexpr auto card(PlaceType const& place) const noexcept -> Card const&
	{
		return card_(*this, place);
	}

	constexpr auto zone(Proto::Duel::Controller con, Proto::Duel::Location loc,
	                    uint32_t seq) const noexcept ->
		typename Side::Zone const&
	{
		return zone_(*this, con, loc, seq);
	}

	constexpr auto zone(PlaceType const& place) const noexcept ->
		typename Side::Zone const&
	{
		return zone(place.con(), place.loc(), place.seq());
	}

	// Non-const getters.

	constexpr auto builder() noexcept -> CardBuilder& { return cards_; }

	constexpr auto pile(Proto::Duel::Controller con,
	                    Proto::Duel::Location loc) noexcept -> PileType&
	{
		return pile_(*this, con, loc);
	}

	constexpr auto card(PlaceType const& place) noexcept -> Card&
	{
		return card_(*this, place);
	}

	constexpr auto zone(Proto::Duel::Controller con, Proto::Duel::Location loc,
	                    uint32_t seq) noexcept -> typename Side::Zone&
	{
		return zone_(*this, con, loc, seq);
	}

	constexpr auto zone(PlaceType const& place) noexcept -> typename Side::Zone&
	{
		return zone(place.con(), place.loc(), place.seq());
	}

	// Modifiers.

	constexpr auto card_add(PlaceType const& place) noexcept -> Card&
	{
		Card& c = construct_card();
		card_insert(place, c);
		assert(has_card(place));
		return c;
	}

	constexpr auto card_remove(PlaceType const& place) noexcept -> void
	{
		assert(has_card(place));
		destruct_card(card_erase(place));
	}

	constexpr auto card_move(PlaceType const& from,
	                         PlaceType const& to) noexcept -> Card&
	{
		assert(has_card(from));
		bool const is_from_pile = is_pile(from);
		bool const is_to_pile = is_pile(to);
		bool const is_from_not_xyz_mat = from.oseq() < 0;
		bool const is_to_not_xyz_mat = to.oseq() < 0;
		// Do not refer to a material location while also being a pile.
		assert(!is_from_pile || is_from_not_xyz_mat);
		assert(!is_to_pile || is_to_not_xyz_mat);
		Card* c = nullptr;
		if(is_from_pile && is_to_pile)
		{
			auto& p1 = pile(from.con(), from.loc());
			auto& p2 = pile(to.con(), to.loc());
			c = insert_at_(p2, to.seq(), take_at_(p1, from.seq()));
		}
		else if(is_from_pile && !is_to_pile)
		{
			auto& p = pile(from.con(), from.loc());
			c = take_at_(p, from.seq());
			auto& z = zone(to);
			if(is_to_not_xyz_mat)
			{
				assert(z.card == nullptr);
				z.card = c;
			}
			else
			{
				insert_at_(z.materials, to.oseq(), c);
			}
		}
		else if(!is_from_pile && is_to_pile)
		{
			auto& z = zone(from);
			if(is_from_not_xyz_mat)
				std::swap(z.card, c);
			else
				c = take_at_(z.materials, from.oseq());
			insert_at_(pile(to.con(), to.loc()), to.seq(), c);
		}
		else // !is_from_pile && !is_to_pile
		{
			auto& z1 = zone(from);
			auto& z2 = zone(to);
			if(is_from_not_xyz_mat && is_to_not_xyz_mat)
			{
				assert(z2.card == nullptr && z2.materials.empty());
				std::swap(c, z1.card);
				z2.card = c;
				std::swap(z2.materials, z1.materials);
			}
			else if(is_from_not_xyz_mat && !is_to_not_xyz_mat)
			{
				std::swap(c, z1.card);
				insert_at_(z2.materials, to.oseq(), c);
			}
			else if(!is_from_not_xyz_mat && is_to_not_xyz_mat)
			{
				assert(z2.card == nullptr);
				c = take_at_(z1.materials, from.oseq());
				z2.card = c;
			}
			else // !is_from_not_xyz_mat && !is_to_not_xyz_mat
			{
				c = take_at_(z1.materials, from.oseq());
				insert_at_(z2.materials, to.oseq(), c);
			}
		}
		assert(has_card(to));
		return *c;
	}

	template<typename InputIt>
	constexpr auto card_shuffle(InputIt previous, InputIt current,
	                            size_t count) noexcept -> void
	{
		// NOTE: Also assuming that shuffle is done for a single location.
		assert(!is_pile(previous->loc()));
		assert(count <= FIELD_ZONE_LIMIT_MONSTER);
		std::array<uint32_t, FIELD_ZONE_LIMIT_MONSTER> seqs{};
		for(size_t i = 0U; i < count; i++)
		{
			auto const& place = *previous++;
			assert(has_card(place));
			seqs[i] = place.seq();
		}
		for(size_t i = 0U; i < count; i++)
		{
			auto const& place = *current++;
			if(is_empty(place))
				continue;
			std::swap(zone(place.con(), place.loc(), seqs[i]), zone(place));
			auto const seq = place.seq();
			for(size_t j = 0U; j < count; j++)
			{
				if(seqs[j] != seq)
					continue;
				seqs[j] = seqs[i];
				break;
			}
			seqs[i] = seq;
		}
	}

	constexpr auto card_swap(PlaceType const& a, PlaceType const& b) noexcept
		-> void
	{
		assert(has_card(a));
		assert(has_card(b));
		bool const is_a_pile = is_pile(a);
		bool const is_b_pile = is_pile(b);
		bool const is_a_not_xyz_mat = a.oseq() < 0;
		bool const is_b_not_xyz_mat = b.oseq() < 0;
		// Do not refer to a material location while also being a pile.
		assert(!is_a_pile || is_a_not_xyz_mat);
		assert(!is_b_pile || is_b_not_xyz_mat);
		// Do not swap card with materials outside of the field.
		assert(!is_a_not_xyz_mat || !is_b_pile);
		assert(!is_b_not_xyz_mat || !is_a_pile);
		if(is_a_pile && is_b_pile)
		{
			auto& pa = pile(a.con(), a.loc());
			auto& pb = pile(b.con(), b.loc());
			std::swap(pa[a.seq()], pb[b.seq()]);
		}
		else if(is_a_pile && !is_b_pile)
		{
			auto& p = pile(a.con(), a.loc());
			auto& z = zone(b);
			if(is_b_not_xyz_mat)
				std::swap(p[a.seq()], z.card);
			else
				std::swap(p[a.seq()], z.materials[b.oseq()]);
		}
		else if(!is_a_pile && is_b_pile)
		{
			auto& z = zone(a);
			auto& p = pile(a.con(), a.loc());
			if(is_a_not_xyz_mat)
				std::swap(z.card, p[b.oseq()]);
			else
				std::swap(z.materials[a.oseq()], p[b.oseq()]);
		}
		else // !is_a_pile && !is_b_pile
		{
			auto& za = zone(a);
			auto& zb = zone(b);
			if(is_a_not_xyz_mat && is_b_not_xyz_mat)
			{
				std::swap(za.card, zb.card);
				std::swap(za.materials, zb.materials);
			}
			else if(is_a_not_xyz_mat && !is_b_not_xyz_mat)
			{
				std::swap(za.card, zb.materials[b.oseq()]);
			}
			else if(!is_a_not_xyz_mat && is_b_not_xyz_mat)
			{
				std::swap(za.materials[a.seq()], zb.card);
			}
			else // !is_a_not_xyz_mat && !is_b_not_xyz_mat
			{
				std::swap(za.materials[a.seq()], zb.materials[b.oseq()]);
			}
		}
	}

	constexpr auto pile_resize(PlaceType const& place, size_t count) noexcept
		-> void
	{
		assert(is_pile(place));
		auto& p = pile(place.con(), place.loc());
		if(p.size() < count)
		{
			for(size_t i = count - p.size(); i != 0; i--)
				p.push_back(&construct_card());
		}
		else if(p.size() > count)
		{
			for(size_t i = p.size() - count; i != 0; i--)
			{
				destruct_card(*p.back());
				p.pop_back();
			}
		}
		assert(p.size() == count);
	}

	constexpr auto pile_splice(PlaceType const& from, size_t count,
	                           PlaceType const& to, bool reverse) noexcept
		-> void
	{
		assert(is_pile(from));
		assert(is_pile(to));
		pile_splice_cache_.resize(count);
		auto& pile_from = pile(from.con(), from.loc());
		assert(count + from.seq() <= pile_from.size());
		{
			auto first = pile_from.cbegin() + from.seq();
			auto last = first + count;
			auto d_first = pile_splice_cache_.begin();
			if(reverse)
				std::reverse_copy(first, last, d_first);
			else
				std::copy(first, last, d_first);
			pile_from.erase(first, last);
		}
		{
			auto& pile_to = pile(to.con(), to.loc());
			pile_to.insert(pile_to.begin() + to.seq(),
			               pile_splice_cache_.cbegin(),
			               pile_splice_cache_.cend());
		}
	}

	constexpr auto pile_swap(PlaceType const& a, PlaceType const& b) noexcept
		-> void
	{
		assert(is_pile(a));
		assert(is_pile(b));
		std::swap(pile(a.con(), a.loc()), pile(b.con(), b.loc()));
	}

	constexpr auto clear() noexcept -> void
	{
		auto destruct_pile = [this](PileType& p)
		{
			for(auto* c : p)
				destruct_card(*c);
			p.clear();
		};
		auto destruct_zone_location = [&](auto& loc)
		{
			for(auto& zone : loc)
			{
				if(zone.card != nullptr)
					destruct_card(*zone.card);
				zone.card = nullptr;
				destruct_pile(zone.materials);
			}
		};
		for(auto& p : main_deck_)
			destruct_pile(p);
		for(auto& p : hand_)
			destruct_pile(p);
		for(auto& p : graveyard_)
			destruct_pile(p);
		for(auto& p : banished_)
			destruct_pile(p);
		for(auto& p : extra_deck_)
			destruct_pile(p);
		for(auto& side : field_)
		{
			destruct_zone_location(side.monster_);
			destruct_zone_location(side.spell_);
			destruct_zone_location(side.field_);
			destruct_zone_location(side.pendulum_);
			destruct_zone_location(side.skill_);
		}
	}

protected:
	// Provide easy access to field_ for users that extend this class.

	constexpr auto field() noexcept -> FieldType& { return field_; }

	// Methods that compose add and remove operations.

	constexpr auto card_insert(PlaceType const& place, Card& c) noexcept -> void
	{
		if(is_pile(place))
		{
			auto& p = pile(place.con(), place.loc());
			p.insert(p.begin() + place.seq(), &c);
			return;
		}
		auto& z = zone(place);
		if(place.oseq() < 0)
		{
			assert(z.card == nullptr);
			z.card = &c;
			return;
		}
		insert_at_(z.materials, place.oseq(), &c);
	}

	constexpr auto card_erase(PlaceType const& place) noexcept -> Card&
	{
		Card* c = nullptr;
		if(is_pile(place))
		{
			auto& p = pile(place.con(), place.loc());
			c = p[place.seq()];
			p.erase(p.begin() + place.seq());
		}
		else
		{
			auto& z = zone(place);
			if(place.oseq() < 0)
				std::swap(c, z.card);
			else
				c = take_at_(z.materials, place.oseq());
		}
		return *c;
	}

	constexpr auto construct_card() noexcept -> Card&
	{
		return cards_.m.emplace_front(cards_.build());
	}

	constexpr auto destruct_card(Card& c) noexcept -> void
	{
		// NOTE: std::forward_list::remove attempts to remove all elements that
		// match, but we know that they are unique so no need to traverse the
		// entire container after the first removal is performed.
		const auto end = cards_.m.cend();
		auto prev_it = cards_.m.before_begin();
		using Iter = typename decltype(cards_.m)::iterator;
		for(Iter it; (it = std::next(prev_it)) != end; prev_it++)
		{
			if(&*it != &c)
				continue;
			cards_.m.erase_after(prev_it);
			break;
		}
	}

private:
	using MultiPile = std::array<PileType, Proto::Duel::Controller_ARRAYSIZE>;

	// NOTE: Empty base optimization.
	struct Packed : public CardBuilder
	{
		std::forward_list<Card> m;
		explicit constexpr Packed(CardBuilder const& b) noexcept
			: CardBuilder(b), m()
		{}
	};
	Packed cards_;

	MultiPile main_deck_;
	MultiPile hand_;
	MultiPile graveyard_;
	MultiPile banished_;
	MultiPile extra_deck_;
	FieldType field_;

	PileType pile_splice_cache_;

	// Getters implementation.

	template<typename T>
	static constexpr auto pile_(T& t, Proto::Duel::Controller con,
	                            Proto::Duel::Location loc) noexcept -> auto&
	{
		using namespace YGOpen::Proto::Duel;
		if(loc == LOCATION_MAIN_DECK)
			return t.main_deck_[con];
		if(loc == LOCATION_HAND)
			return t.hand_[con];
		if(loc == LOCATION_GRAVEYARD)
			return t.graveyard_[con];
		if(loc == LOCATION_BANISHED)
			return t.banished_[con];
		if(loc == LOCATION_EXTRA_DECK)
			return t.extra_deck_[con];
#ifdef _MSC_VER
		__assume(0);
#else
		__builtin_unreachable();
#endif // _MSC_VER
	}

	template<typename T>
	static constexpr auto card_(T& t, PlaceType const& place) noexcept -> auto&
	{
		if(is_pile(place))
			return *t.pile(place.con(), place.loc())[place.seq()];
		auto& z = t.zone(place);
		if(place.oseq() < 0)
			return *z.card;
		return *z.materials[place.oseq()];
	}

	template<typename T>
	static constexpr auto zone_(T& t, Proto::Duel::Controller con,
	                            Proto::Duel::Location loc,
	                            uint32_t seq) noexcept -> auto&
	{
		assert(!is_pile(loc));
		return ((t.field_[con])[loc])[seq];
	}

	// Utility functions.

	static constexpr auto take_at_(PileType& p, size_t index) noexcept -> Card*
	{
		assert(p.size() > index);
		Card* c = p[index];
		assert(c != nullptr);
		p.erase(p.begin() + index);
		return c;
	}

	static constexpr auto insert_at_(PileType& p, size_t index,
	                                 Card* c) noexcept -> Card*
	{
		assert(p.size() >= index);
		assert(c != nullptr);
		p.insert(p.begin() + index, c);
		return c;
	}
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_FRAME_HPP
