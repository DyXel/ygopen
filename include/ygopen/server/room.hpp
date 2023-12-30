/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_SERVER_ROOM_HPP
#define YGOPEN_SERVER_ROOM_HPP
#include <array>
#include <google/protobuf/arena.h>
#include <set>
#include <ygopen/detail/config.hpp>
#include <ygopen/proto/deck.hpp>
#include <ygopen/proto/room.hpp>
#include <ygopen/proto/user.hpp>

namespace YGOpen::Server
{

using RoomState = YGOpen::Proto::Room::State;
using RoomEvent = YGOpen::Proto::Room::Event;
using RoomSignal = YGOpen::Proto::Room::Signal;

#ifdef YGOPEN_HAS_CONCEPTS
template<typename T>
concept Client = requires(T c, RoomEvent const& re, YGOpen::Proto::User& u)
{
	{
		c.send(re)
	} noexcept;
	{
		std::as_const(c).fill_user(u)
	} noexcept;
};

template<typename T>
concept DeckValidator =
	requires(T dv, std::string_view banlist_id, YGOpen::Proto::Deck const& d,
             YGOpen::Proto::DeckError& de)
{
	{
		std::as_const(dv).check(banlist_id, d, de)
	} noexcept -> std::same_as<bool>;
	// { // TODO: Check custom banlist?
	// 	std::as_const(dv).check(b, d, de)
	// } noexcept -> std::same_as<bool>;
};

template<typename T>
concept CoreDuelFactory = requires(T cdf)
{
	{
		std::as_const(cdf).make_duel(/*TODO*/)
	} noexcept /*-> TODO*/;
};
#endif // YGOPEN_HAS_CONCEPTS

template<YGOPEN_CONCEPT(Client) Client,
         YGOPEN_CONCEPT(DeckValidator) DeckValidator,
         YGOPEN_CONCEPT(CoreDuelFactory) CoreDuelFactory>
class BasicRoom
{
public:
	class InternalSignal;

	BasicRoom(DeckValidator const& deck_validator,
	          CoreDuelFactory const& core_duel_factory, Client& host,
	          YGOpen::Proto::Room::Options const& options) noexcept
		: deck_validator_(&deck_validator)
		, core_duel_factory_(&core_duel_factory)
		, options_(options)
		, state_(RoomState::STATE_HOSTING_CLOSING)
		, host_(&host)
		, teams_({})
	{
		for(auto const& t : teams_)
		{
			for(auto& s : t)
			{
				assert(s.deck_valid == false);
				assert(s.ready == false);
				assert(s.client == nullptr);
			}
		}
		clamp_options_();
		enter_state_(RoomState::STATE_CONFIGURING);
		enter(host);
	}

	// TODO: Construct room from composed teams and no host (matchmaking case)

	~BasicRoom() noexcept = default;
	BasicRoom(BasicRoom&) = delete;
	BasicRoom(BasicRoom const&) = delete;
	BasicRoom(BasicRoom&&) = delete;
	auto operator=(BasicRoom const&) -> BasicRoom& = delete;
	auto operator=(BasicRoom&&) -> BasicRoom& = delete;

	auto enter(Client& peer) noexcept -> void
	{
		auto find_empty_duelist_slot = [&]() noexcept -> DuelistSearch
		{
			auto const& max_duelists = options_.max_duelists();
			std::array<uint8_t, 2> duelist_count{0, 0};
			std::array<int8_t, 2> first_empty_pos{-1, -1};
			DuelistSearch search;
			for(auto const& t : teams_)
			{
				for(auto& s : t)
				{
					if(s.client != nullptr)
						duelist_count[search.team]++;
					else if(first_empty_pos[search.team] < 0)
						first_empty_pos[search.team] = search.pos;
					if(++search.pos == max_duelists[search.team])
						break;
				}
				search.team++;
				search.pos = 0;
			}
			if(first_empty_pos[0] < 0 && first_empty_pos[1] < 0)
				return search;
			if((duelist_count[0] <= duelist_count[1] &&
			    !(first_empty_pos[0] < 0)) ||
			   first_empty_pos[1] < 0)
			{
				assert(!(first_empty_pos[0] < 0));
				search.team = 0;
				search.pos = static_cast<uint8_t>(first_empty_pos[0]);
			}
			else
			{
				assert(!(first_empty_pos[1] < 0));
				search.team = 1;
				search.pos = static_cast<uint8_t>(first_empty_pos[1]);
			}
			search.slot = &teams_[search.team][search.pos];
			return search;
		};
		switch(state_)
		{
		case RoomState::STATE_CONFIGURING:
		{
			auto* e = event_();
			auto* es = e->mutable_configuring()->mutable_entering_state();
			*es->mutable_options() = options_;
			{
				uint8_t team = 0;
				uint8_t pos = 0;
				for(auto const& t : teams_)
				{
					for(auto& s : t)
					{
						if(s.client != nullptr)
						{
							auto& duelist = *es->add_duelists();
							duelist.set_team(team);
							duelist.set_pos(pos);
							s.client->fill_user(*duelist.mutable_user());
							duelist.set_is_host(s.client == host_);
							duelist.set_is_ready(s.ready);
						}
						pos++;
					}
					team++;
					pos = 0;
				}
			}
			es->set_spectator_count(spectators_.size());
			peer.send(*e);
			auto const search = find_empty_duelist_slot();
			auto* e2 = event_();
			if(search.slot != nullptr)
			{
				search.slot->client = &peer;
				auto* msg = e2->mutable_configuring()->mutable_duelist_enters();
				msg->set_team(search.team);
				msg->set_pos(search.pos);
				peer.fill_user(*msg->mutable_user());
				msg->set_is_host(host_ == &peer);
				msg->set_is_ready(false);
			}
			else
			{
				// NOTE: You can't become a host while initially joining as
				//       spectator, but you can be a host that changes to it.
				assert(host_ != &peer);
				spectators_.insert(&peer);
				e2->set_spectator_count(spectators_.size());
			}
			send_all_(*e2);
			break;
		}
		case RoomState::STATE_HOSTING_CLOSING:
			break;
		case RoomState::State_INT_MIN_SENTINEL_DO_NOT_USE_:
		case RoomState::State_INT_MAX_SENTINEL_DO_NOT_USE_:
			YGOPEN_UNREACHABLE();
		}
	}

	auto visit(InternalSignal const& signal) noexcept -> void;

	auto visit(Client& peer, RoomSignal const& signal) noexcept -> void
	{
		auto const& max_duelists = options_.max_duelists();
		auto find_peer_duelist_slot = [&]() noexcept -> DuelistSearch
		{
			DuelistSearch search;
			for(auto& t : teams_)
			{
				for(auto& s : t)
				{
					if(s.client == &peer)
					{
						search.slot = &s;
						return search;
					}
					if(++search.pos == max_duelists[search.team])
						break;
				}
				search.team++;
				search.pos = 0;
			}
			return search;
		};
		auto const signal_case = signal.t_case();
		switch(state_)
		{
		case RoomState::STATE_CONFIGURING:
		{
			if(signal_case != RoomSignal::kConfiguring)
				break;
			auto const& s = signal.configuring();
			switch(s.t_case())
			{
			case RoomSignal::Configuring::kUpdateDeck:
			{
				auto const search = find_peer_duelist_slot();
				if(search.slot == nullptr)
					break;
				auto const& deck = s.update_deck();
				auto* e = event_();
				auto* ds = e->mutable_configuring()->mutable_deck_status();
				auto* de = ds->mutable_deck_error();
				// TODO: Handle custom banlist.
				if(deck_validator_->check(options_.banlist_id(), deck, *de))
				{
					ds->clear_deck_error();
					search.slot->deck = deck;
					search.slot->deck_valid = true;
				}
				else
				{
					search.slot->deck = {};
					search.slot->deck_valid = false;
				}
				peer.send(*e);
				break;
			}
			case RoomSignal::Configuring::kReadyStatus:
			{
				auto const search = find_peer_duelist_slot();
				if(search.slot == nullptr)
					break;
				bool const new_status = s.ready_status();
				if(new_status == search.slot->ready)
					break;
				if(new_status && !search.slot->deck_valid)
				{
					// TODO: Send message saying that you need a valid deck
					// before readying up.
				}
				search.slot->ready = new_status;
				auto* e = event_();
				auto* msg = e->mutable_configuring()->mutable_update_duelist();
				msg->set_team(search.team);
				msg->set_pos(search.pos);
				peer.fill_user(*msg->mutable_user());
				msg->set_is_host(host_ == &peer);
				msg->set_is_ready(new_status);
				send_all_(*e);
				break;
			}
			case RoomSignal::Configuring::T_NOT_SET:
				break;
			}
			break;
		}
		case RoomState::State_INT_MIN_SENTINEL_DO_NOT_USE_:
		case RoomState::State_INT_MAX_SENTINEL_DO_NOT_USE_:
			YGOPEN_UNREACHABLE();
		}
	}

private:
	struct DuelistSlot
	{
		bool deck_valid : 1;
		bool ready : 1;
		Client* client;
		YGOpen::Proto::Deck deck;

		constexpr DuelistSlot() noexcept
			: deck_valid(false), ready(false), client(nullptr), deck()
		{}
	};

	struct DuelistSearch
	{
		uint8_t team = 0;
		uint8_t pos = 0;
		DuelistSlot* slot = nullptr;
	};

	using Team =
		std::array<DuelistSlot, YGOpen::Proto::Room::MAX_DUELISTS_PER_TEAM>;

	using Teams = std::array<Team, 2>;

	DeckValidator const* deck_validator_;
	CoreDuelFactory const* core_duel_factory_;
	YGOpen::Proto::Room::Options options_;
	RoomState state_;

	Client const* host_;
	Teams teams_;
	std::set<Client*> spectators_;

	google::protobuf::Arena arena_;

	auto event_() noexcept -> RoomEvent*
	{
		auto* re = google::protobuf::Arena::CreateMessage<RoomEvent>(&arena_);
		assert(re != nullptr);
		return re;
	}

	auto send_team_(uint8_t team, RoomEvent const& event) noexcept -> void
	{
		assert(event.t_case() != 0);
		assert(team < teams_.size());
		for(auto& s : teams_[team])
			if(s.client != nullptr)
				s.client->send(event);
	}

	auto send_spectators_(RoomEvent const& event) noexcept -> void
	{
		assert(event.t_case() != 0);
		for(auto* s : spectators_)
			s->send(event);
	}

	auto send_all_(RoomEvent const& event) noexcept -> void
	{
		assert(event.t_case() != 0);
		send_team_(0, event);
		send_team_(1, event);
		send_spectators_(event);
	}

	auto clamp_options_() noexcept -> void
	{
		auto& max_duelists = *options_.mutable_max_duelists();
		max_duelists.Resize(teams_.size(), 1);
		for(auto& v : max_duelists)
			v = std::clamp<uint32_t>(v, 1, std::tuple_size_v<Team>);
	}

	auto exit_current_state_() noexcept -> void
	{
		switch(state_)
		{
		case RoomState::STATE_HOSTING_CLOSING:
			break;
		case RoomState::State_INT_MIN_SENTINEL_DO_NOT_USE_:
		case RoomState::State_INT_MAX_SENTINEL_DO_NOT_USE_:
			YGOPEN_UNREACHABLE();
		}
	}

	auto enter_state_(RoomState new_state) noexcept -> void
	{
		assert(state_ != new_state);
		exit_current_state_();
		state_ = new_state;
		switch(state_)
		{
		case RoomState::STATE_CONFIGURING:
		{
			// TODO: If we came from matchmake case:
			//         * Decide a new host.
			//         * Re-check Client decks.

			// TODO: Set all ready statuses to false.
			auto* e = event_();
			e->mutable_configuring()->mutable_entering_state();
			send_all_(*e);
			break;
		}
		case RoomState::State_INT_MIN_SENTINEL_DO_NOT_USE_:
		case RoomState::State_INT_MAX_SENTINEL_DO_NOT_USE_:
			YGOPEN_UNREACHABLE();
		}
	}
};

} // namespace YGOpen::Server

#endif // YGOPEN_SERVER_ROOM_HPP
