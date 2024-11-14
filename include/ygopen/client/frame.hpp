/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
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
#include <ygopen/detail/config.hpp>
#include <ygopen/duel/constants/controller.hpp>
#include <ygopen/duel/constants/location.hpp>
#include <ygopen/proto/duel/data.hpp>

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

	[[nodiscard]] static constexpr auto build() noexcept -> T { return T{}; };
};

enum FieldZoneLimit : size_t
{
	FIELD_ZONE_LIMIT_MONSTER = 7U,
	FIELD_ZONE_LIMIT_SPELL = 5U,
	FIELD_ZONE_LIMIT_FIELD = 1U,
	FIELD_ZONE_LIMIT_PENDULUM = 2U,
	FIELD_ZONE_LIMIT_SKILL = 1U,
};

} // namespace Detail

[[nodiscard]] constexpr auto zone_seq_lim(YGOpen::Duel::Location loc) noexcept
	-> size_t
{
	using namespace YGOpen::Duel;
	if(loc == LOCATION_MONSTER_ZONE)
		return Detail::FIELD_ZONE_LIMIT_MONSTER;
	if(loc == LOCATION_SPELL_ZONE)
		return Detail::FIELD_ZONE_LIMIT_SPELL;
	if(loc == LOCATION_FIELD_ZONE)
		return Detail::FIELD_ZONE_LIMIT_FIELD;
	if(loc == LOCATION_PENDULUM_ZONE)
		return Detail::FIELD_ZONE_LIMIT_PENDULUM;
	if(loc == LOCATION_SKILL_ZONE)
		return Detail::FIELD_ZONE_LIMIT_SKILL;
	YGOPEN_UNREACHABLE();
}

template<typename Card, typename CardBuilder = Detail::DefaultBuilder<Card>>
class BasicFrame
{
public:
	using CardType = Card;
	using PileType = std::vector<CardType*>;
	using PlaceType = Proto::Duel::Place;

	class Side
	{
	public:
		struct Zone
		{
			CardType* card{};
			PileType materials;
		};

		[[nodiscard]] constexpr auto operator[](
			Duel::Location loc) const noexcept -> Zone const*
		{
			return at_(*this, loc);
		}

		[[nodiscard]] constexpr auto operator[](Duel::Location loc) noexcept
			-> Zone*
		{
			return at_(*this, loc);
		}

	private:
		friend BasicFrame;

		template<size_t Size>
		using ZoneArray = std::array<Zone, Size>;

		ZoneArray<zone_seq_lim(YGOpen::Duel::LOCATION_MONSTER_ZONE)> monster_;
		ZoneArray<zone_seq_lim(YGOpen::Duel::LOCATION_SPELL_ZONE)> spell_;
		ZoneArray<zone_seq_lim(YGOpen::Duel::LOCATION_FIELD_ZONE)> field_;
		ZoneArray<zone_seq_lim(YGOpen::Duel::LOCATION_PENDULUM_ZONE)> pendulum_;
		ZoneArray<zone_seq_lim(YGOpen::Duel::LOCATION_SKILL_ZONE)> skill_;

		// FIXME: Use std::span as return type if we ever move to >=C++20.
		template<typename T>
		static constexpr auto at_(T& t, Duel::Location loc) noexcept -> auto*
		{
			assert(is_zone(loc));
			using namespace YGOpen::Duel;
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
			YGOPEN_UNREACHABLE();
		}
	};

	using FieldType = std::array<Side, Duel::CONTROLLER_ARRAY_SIZE>;

	explicit constexpr BasicFrame(
		CardBuilder const& builder = CardBuilder()) noexcept
		: cards_(builder)
	{}

	// Const getters.

	[[nodiscard]] constexpr auto builder() const noexcept -> CardBuilder const&
	{
		return cards_;
	}

	[[nodiscard]] constexpr auto has_card(PlaceType const& place) const noexcept
		-> bool
	{
		assert(!is_empty(place));
		if(is_pile(place))
			return place.seq() < pile(place).size();
		assert(is_zone(place));
		assert(place.seq() < zone_seq_lim(get_loc(place)));
		auto& z = zone(place);
		if(place.oseq() < 0)
			return z.card != nullptr;
		return static_cast<size_t>(place.oseq()) < z.materials.size();
	}

	[[nodiscard]] constexpr auto card(PlaceType const& place) const noexcept
		-> CardType const&
	{
		return card_(*this, place);
	}

	[[nodiscard]] constexpr auto pile(Duel::Controller con, Duel::Location loc)
		const noexcept -> PileType const&
	{
		return pile_(*this, con, loc);
	}

	[[nodiscard]] constexpr auto pile(PlaceType const& place) const noexcept
		-> PileType const&
	{
		return pile_(*this, get_con(place), get_loc(place));
	}

	[[nodiscard]] constexpr auto zone(Duel::Controller con, Duel::Location loc,
	                                  uint32_t seq) const noexcept ->
		typename Side::Zone const&
	{
		return zone_(*this, con, loc, seq);
	}

	[[nodiscard]] constexpr auto zone(PlaceType const& place) const noexcept ->
		typename Side::Zone const&
	{
		return zone(get_con(place), get_loc(place), place.seq());
	}

	// Non-const getters.

	[[nodiscard]] constexpr auto builder() noexcept -> CardBuilder&
	{
		return cards_;
	}

	[[nodiscard]] constexpr auto card(PlaceType const& place) noexcept
		-> CardType&
	{
		return card_(*this, place);
	}

	[[nodiscard]] constexpr auto pile(Duel::Controller con,
	                                  Duel::Location loc) noexcept -> PileType&
	{
		return pile_(*this, con, loc);
	}

	[[nodiscard]] constexpr auto pile(PlaceType const& place) noexcept
		-> PileType&
	{
		return pile_(*this, get_con(place), get_loc(place));
	}

	[[nodiscard]] constexpr auto zone(Duel::Controller con, Duel::Location loc,
	                                  uint32_t seq) noexcept ->
		typename Side::Zone&
	{
		return zone_(*this, con, loc, seq);
	}

	[[nodiscard]] constexpr auto zone(PlaceType const& place) noexcept ->
		typename Side::Zone&
	{
		return zone(get_con(place), get_loc(place), place.seq());
	}

	// Modifiers.

	constexpr auto card_add(PlaceType const& place) noexcept -> CardType&
	{
		CardType& c = construct_card();
		card_insert(place, c);
		return c;
	}

	constexpr auto card_remove(PlaceType const& place) noexcept -> void
	{
		assert(has_card(place));
		destruct_card(card_erase(place));
	}

	constexpr auto card_move(PlaceType const& from,
	                         PlaceType const& to) noexcept -> CardType&
	{
		assert(has_card(from));
		bool const is_from_pile = is_pile(from);
		bool const is_to_pile = is_pile(to);
		bool const is_from_not_mat = from.oseq() < 0;
		bool const is_to_not_mat = to.oseq() < 0;
		// Do not refer to a material location while also being a pile.
		assert(!is_from_pile || is_from_not_mat);
		assert(!is_to_pile || is_to_not_mat);
		CardType* c = nullptr;
		if(is_from_pile && is_to_pile)
		{
			auto& p1 = pile(from);
			auto& p2 = pile(to);
			c = insert_at_(p2, to.seq(), take_at_(p1, from.seq()));
		}
		else if(is_from_pile && !is_to_pile)
		{
			auto& p = pile(from);
			c = take_at_(p, from.seq());
			auto& z = zone(to);
			if(is_to_not_mat)
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
			if(is_from_not_mat)
				std::swap(z.card, c);
			else
				c = take_at_(z.materials, from.oseq());
			insert_at_(pile(to), to.seq(), c);
		}
		else // !is_from_pile && !is_to_pile
		{
			auto& z1 = zone(from);
			auto& z2 = zone(to);
			if(is_from_not_mat && is_to_not_mat)
			{
				assert(z2.card == nullptr && z2.materials.empty());
				std::swap(c, z1.card);
				z2.card = c;
				std::swap(z2.materials, z1.materials);
			}
			else if(is_from_not_mat && !is_to_not_mat)
			{
				std::swap(c, z1.card);
				insert_at_(z2.materials, to.oseq(), c);
			}
			else if(!is_from_not_mat && is_to_not_mat)
			{
				assert(z2.card == nullptr);
				c = take_at_(z1.materials, from.oseq());
				z2.card = c;
			}
			else // !is_from_not_mat && !is_to_not_mat
			{
				c = take_at_(z1.materials, from.oseq());
				insert_at_(z2.materials, to.oseq(), c);
			}
		}
		return *c;
	}

	template<typename InputIt>
	constexpr auto card_shuffle(InputIt previous, InputIt current,
	                            size_t count) noexcept -> void
	{
		// NOTE: Also assuming that shuffle is done for a single location.
		assert(!is_pile(*previous));
		constexpr size_t UPPER_BOUND =
			zone_seq_lim(YGOpen::Duel::LOCATION_MONSTER_ZONE);
		assert(count <= UPPER_BOUND);
		std::array<uint32_t, UPPER_BOUND> seqs{};
		for(size_t i = 0U; i < count; i++)
		{
			auto const& p = *previous++;
			assert(has_card(p));
			seqs[i] = p.seq();
		}
		for(size_t i = 0U; i < count; i++)
		{
			auto const& p = *current++;
			if(is_empty(p))
				continue;
			std::swap(zone(get_con(p), get_loc(p), seqs[i]), zone(p));
			auto const seq = p.seq();
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

	constexpr auto card_swap(PlaceType const& a,
	                         PlaceType const& b) noexcept -> void
	{
		assert(has_card(a));
		assert(has_card(b));
		bool const is_a_pile = is_pile(a);
		bool const is_b_pile = is_pile(b);
		bool const is_a_not_mat = a.oseq() < 0;
		bool const is_b_not_mat = b.oseq() < 0;
		// Do not refer to a material location while also being a pile.
		assert(!is_a_pile || is_a_not_mat);
		assert(!is_b_pile || is_b_not_mat);
		// Do not swap card with materials outside of field zones.
		assert(is_a_pile || !is_a_not_mat || zone(a).materials.empty() ||
		       !is_b_pile);
		assert(is_b_pile || !is_b_not_mat || zone(b).materials.empty() ||
		       !is_a_pile);
		if(is_a_pile && is_b_pile)
		{
			auto& pa = pile(a);
			auto& pb = pile(b);
			std::swap(pa[a.seq()], pb[b.seq()]);
		}
		else if(is_a_pile && !is_b_pile)
		{
			auto& p = pile(a);
			auto& z = zone(b);
			if(is_b_not_mat)
				std::swap(p[a.seq()], z.card);
			else
				std::swap(p[a.seq()], z.materials[b.oseq()]);
		}
		else if(!is_a_pile && is_b_pile)
		{
			auto& z = zone(a);
			auto& p = pile(b);
			if(is_a_not_mat)
				std::swap(z.card, p[b.seq()]);
			else
				std::swap(z.materials[a.oseq()], p[b.seq()]);
		}
		else // !is_a_pile && !is_b_pile
		{
			auto& za = zone(a);
			auto& zb = zone(b);
			if(is_a_not_mat && is_b_not_mat)
			{
				std::swap(za.card, zb.card);
				std::swap(za.materials, zb.materials);
			}
			else if(is_a_not_mat && !is_b_not_mat)
			{
				std::swap(za.card, zb.materials[b.oseq()]);
			}
			else if(!is_a_not_mat && is_b_not_mat)
			{
				std::swap(za.materials[a.seq()], zb.card);
			}
			else // !is_a_not_mat && !is_b_not_mat
			{
				std::swap(za.materials[a.seq()], zb.materials[b.oseq()]);
			}
		}
	}

	constexpr auto pile_resize(PlaceType const& place,
	                           size_t count) noexcept -> void
	{
		assert(is_pile(place));
		auto& p = pile(place);
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
		verify_pile_(p);
	}

	constexpr auto pile_splice(PlaceType const& from, size_t count,
	                           PlaceType const& to,
	                           bool reverse) noexcept -> void
	{
		assert(is_pile(from));
		assert(is_pile(to));
		pile_splice_cache_.resize(count);
		auto& pile_from = pile(from);
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
			verify_pile_(pile_from);
		}
		{
			auto& pile_to = pile(to);
			pile_to.insert(pile_to.begin() + to.seq(),
			               pile_splice_cache_.cbegin(),
			               pile_splice_cache_.cend());
			verify_pile_(pile_to);
		}
	}

	constexpr auto pile_swap(PlaceType const& a,
	                         PlaceType const& b) noexcept -> void
	{
		assert(is_pile(a));
		assert(is_pile(b));
		std::swap(pile(a), pile(b));
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

	[[nodiscard]] constexpr auto field() noexcept -> FieldType&
	{
		return field_;
	}

	// Methods that compose add and remove operations.

	constexpr auto card_insert(PlaceType const& place,
	                           CardType& c) noexcept -> void
	{
		if(is_pile(place))
		{
			insert_at_(pile(place), place.seq(), &c);
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

	constexpr auto card_erase(PlaceType const& place) noexcept -> CardType&
	{
		CardType* c = nullptr;
		if(is_pile(place))
		{
			auto& p = pile(place);
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

	[[nodiscard]] constexpr auto construct_card() noexcept -> CardType&
	{
		return cards_.m.emplace_front(cards_.build());
	}

	constexpr auto destruct_card(CardType& c) noexcept -> void
	{
		// NOTE: std::forward_list::remove attempts to remove all elements that
		// match, but we know that they are unique so no need to traverse the
		// entire container after the first removal is performed.
		bool destructed = false;
		auto const end = cards_.m.cend();
		auto prev_it = cards_.m.before_begin();
		using Iter = typename decltype(cards_.m)::iterator;
		for(Iter it; (it = std::next(prev_it)) != end; prev_it++)
		{
			if(&*it != &c)
				continue;
			cards_.m.erase_after(prev_it);
			destructed = true;
			break;
		}
		assert(destructed);
	}

	// Debug functions.

	static constexpr auto verify_pile_(PileType const& p) noexcept -> void
	{
		for(auto c : p)
			assert(c != nullptr);
	}

private:
	using MultiPile = std::array<PileType, Duel::CONTROLLER_ARRAY_SIZE>;

	// NOTE: Empty base optimization.
	struct Packed : public CardBuilder
	{
		std::forward_list<CardType> m;
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
	static constexpr auto pile_(T& t, Duel::Controller con,
	                            Duel::Location loc) noexcept -> auto&
	{
		assert(con <= 1);
		assert(is_pile(loc));
		using namespace YGOpen::Duel;
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
		YGOPEN_UNREACHABLE();
	}

	template<typename T>
	static constexpr auto card_(T& t, PlaceType const& place) noexcept -> auto&
	{
		assert(!is_empty(place));
		assert(t.has_card(place));
		if(is_pile(place))
			return *t.pile(place)[place.seq()];
		auto& z = t.zone(place);
		if(place.oseq() < 0)
			return *z.card;
		return *z.materials[place.oseq()];
	}

	template<typename T>
	static constexpr auto zone_(T& t, Duel::Controller con, Duel::Location loc,
	                            uint32_t seq) noexcept -> auto&
	{
		assert(con <= 1);
		assert(seq < zone_seq_lim(loc));
		return ((t.field_[con])[loc])[seq];
	}

	// Utility functions.

	static constexpr auto take_at_(PileType& p,
	                               size_t index) noexcept -> CardType*
	{
		assert(p.size() > index);
		CardType* c = p[index];
		assert(c != nullptr);
		p.erase(p.begin() + index);
		return c;
	}

	static constexpr auto insert_at_(PileType& p, size_t index,
	                                 CardType* c) noexcept -> CardType*
	{
		assert(p.size() >= index);
		assert(c != nullptr);
		p.insert(p.begin() + index, c);
		return c;
	}
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_FRAME_HPP
