/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <functional>
#include <gtest/gtest.h>
#include <string_view>
#include <tuple>
#include <vector>
#include <ygopen/server/room.hpp>

namespace
{

using Ev = YGOpen::Server::RoomEvent;
using EvRef = Ev const&;

using EvCase = Ev::TCase;
using EvConfCase = Ev::Configuring::TCase;

// auto wrap(Client& clients...) -> clients... // TODO

using Sig = YGOpen::Server::RoomSignal;

class Client
{
public:
	using Expectation = std::function<void(EvRef)>;

	Client(std::string_view name) noexcept
	{
		user_.set_name(name.data(), name.size());
	}

	auto send(EvRef e) noexcept -> void { events_received_.push_back(e); }

	auto fill_user(YGOpen::Proto::User& user) const noexcept { user = user_; }

	auto expect(Expectation f) noexcept -> void { expectations_.push_back(f); }

	auto expect(EvConfCase ecc) noexcept -> void
	{
		expectations_.push_back(
			[ecc](EvRef e)
			{
				ASSERT_EQ(e.t_case(), EvCase::kConfiguring);
				ASSERT_EQ(e.configuring().t_case(), ecc);
			});
	}

	~Client()
	{
		SCOPED_TRACE("For " + user_.name());
		EXPECT_EQ(events_received_.size(), expectations_.size());
		if(events_received_.empty())
			return;
		size_t i = 0;
		for(auto& f : expectations_)
		{
			f(events_received_[i]);
			// Lets try to run as many expectations as possible.
			if(++i == events_received_.size())
				return;
		}
	}

private:
	YGOpen::Proto::User user_;
	std::vector<Ev> events_received_;
	std::vector<Expectation> expectations_;
};

class DeckValidator
{
public:
	DeckValidator(bool check_status = true) : check_status_(check_status) {}

	auto check(std::string_view banlist_id, YGOpen::Proto::Deck const& deck,
	           YGOpen::Proto::DeckError& error) const noexcept -> bool
	{
		return check_status_;
	}

private:
	bool check_status_;
} dv;

class CoreDuelFactory
{
public:
	auto make_duel(/*TODO*/) const noexcept -> void {}
} cdf;

using TestRoom =
	YGOpen::Server::BasicRoom<Client, DeckValidator, CoreDuelFactory>;

TEST(ServerRoom, ConstructionWithHostWorks)
{
	Client c("Client");
	c.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_entering_state());
			ASSERT_TRUE(e.configuring().entering_state().has_options());
		});
	c.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_duelist_enters());
			auto& de = e.configuring().duelist_enters();
			EXPECT_EQ(de.team(), 0);
			EXPECT_EQ(de.pos(), 0);
			EXPECT_TRUE(de.is_host());
		});
	auto room = TestRoom{dv, cdf, c, {}};
}

TEST(ServerRoom, ConstructionOptionsClampingWorks)
{
	Client c("Client");
	YGOpen::Proto::Room::Options options;
	options.add_max_duelists(99);
	// NOTE: Only setting one on purpose, to test that the room fills the
	//       missing value for the second team.
	c.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_entering_state());
			ASSERT_TRUE(e.configuring().entering_state().has_options());
			auto& opts = e.configuring().entering_state().options();
			EXPECT_EQ(opts.max_duelists_size(), 2);
			EXPECT_EQ(opts.max_duelists(0), 3);
			EXPECT_EQ(opts.max_duelists(1), 1);
		});
	c.expect(EvConfCase::kDuelistEnters);
	auto room = TestRoom{dv, cdf, c, options};
}

TEST(ServerRoom, ConfiguringStateJoining1V1Works)
{
	YGOpen::Proto::Room::Options options;
	options.add_max_duelists(1);
	options.add_max_duelists(1);
	Client c1("Client1");
	c1.expect(EvConfCase::kEnteringState);
	c1.expect(EvConfCase::kDuelistEnters);
	auto room = TestRoom{dv, cdf, c1, options};
	Client c2("Client2");
	c2.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_entering_state());
			ASSERT_TRUE(e.configuring().entering_state().has_options());
			auto& ds = e.configuring().entering_state().duelists();
			ASSERT_EQ(ds.size(), 1);
			auto& d = ds[0];
			EXPECT_EQ(d.team(), 0);
			EXPECT_EQ(d.pos(), 0);
			EXPECT_TRUE(d.has_user());
			EXPECT_EQ(d.user().name(), "Client1");
			EXPECT_TRUE(d.is_host());
		});
	auto exp = [](EvRef e)
	{
		ASSERT_TRUE(e.has_configuring());
		ASSERT_TRUE(e.configuring().has_duelist_enters());
		auto& de = e.configuring().duelist_enters();
		EXPECT_EQ(de.team(), 1);
		EXPECT_EQ(de.pos(), 0);
		EXPECT_EQ(de.user().name(), "Client2");
		EXPECT_FALSE(de.is_host());
	};
	c2.expect(exp);
	c1.expect(exp);
	room.enter(c2);
}

TEST(ServerRoom, ConfiguringStateJoining3V3Works)
{
	YGOpen::Proto::Room::Options options;
	options.add_max_duelists(3);
	options.add_max_duelists(3);
	Client c1("Client1");
	c1.expect(EvConfCase::kEnteringState);
	c1.expect(EvConfCase::kDuelistEnters);
	auto room = TestRoom{dv, cdf, c1, options};
	int team = 1, pos = 0;
	std::vector<Client> clients;
	clients.reserve(5);
	for(int i = 0; i < 5; i++)
	{
		auto const name = "Client" + std::to_string(i + 2);
		auto& c = clients.emplace_back(name);
		c.expect([dc = i + 1](EvRef e) { // dc == duelist_count
			SCOPED_TRACE("with dc == " + std::to_string(dc));
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_entering_state());
			ASSERT_TRUE(e.configuring().entering_state().has_options());
			auto& ds = e.configuring().entering_state().duelists();
			ASSERT_EQ(ds.size(), dc);
			// TODO: Unhardcode this...
			std::vector<std::vector<std::pair<int, int>>> slots = {
				// dc == 1
				{{0, 0}},
				// dc == 2
				{{0, 0}, {1, 0}},
				// dc == 3
				{
					{0, 0},
					{0, 1},
					{1, 0},
				},
				// dc == 4
				{
					{0, 0},
					{0, 1},
					{1, 0},
					{1, 1},
				},
				// dc == 5
				{
					{0, 0},
					{0, 1},
					{0, 2},
					{1, 0},
					{1, 1},
				}};
			for(int j = 0; j < dc; j++)
			{
				auto& d = ds[j];
				auto [t, p] = slots.at(dc - 1).at(j);
				EXPECT_EQ(d.team(), t);
				EXPECT_EQ(d.pos(), p);
			}
		});
		auto duelist_enters_expect = [team, pos, name](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_duelist_enters());
			auto& de = e.configuring().duelist_enters();
			EXPECT_EQ(de.team(), team);
			EXPECT_EQ(de.pos(), pos);
			EXPECT_EQ(de.user().name(), name);
			EXPECT_FALSE(de.is_host());
		};
		c1.expect(duelist_enters_expect);
		for(auto& c : clients)
			c.expect(duelist_enters_expect);
		if(team == 1)
		{
			team = 0;
			pos++;
		}
		else
		{
			team++;
		}
		room.enter(c);
	}
}

TEST(ServerRoom, ConfiguringStateJoiningFullWorks)
{
	YGOpen::Proto::Room::Options options;
	options.add_max_duelists(1);
	options.add_max_duelists(1);
	Client c1("Client1");
	c1.expect(EvConfCase::kEnteringState);
	c1.expect(EvConfCase::kDuelistEnters);
	auto room = TestRoom{dv, cdf, c1, options};
	Client c2("Client2");
	c2.expect(EvConfCase::kEnteringState);
	c2.expect(EvConfCase::kDuelistEnters);
	c1.expect(EvConfCase::kDuelistEnters);
	room.enter(c2);
	Client c3("Client3");
	c3.expect(EvConfCase::kEnteringState);
	auto exp1 = [](EvRef e)
	{
		ASSERT_TRUE(e.has_spectator_count());
		ASSERT_EQ(e.spectator_count(), 1);
	};
	c1.expect(exp1);
	c2.expect(exp1);
	c3.expect(exp1);
	room.enter(c3);
	Client c4("Client4");
	c4.expect(EvConfCase::kEnteringState);
	auto exp2 = [](EvRef e)
	{
		ASSERT_TRUE(e.has_spectator_count());
		ASSERT_EQ(e.spectator_count(), 2);
	};
	c1.expect(exp2);
	c2.expect(exp2);
	c3.expect(exp2);
	c4.expect(exp2);
	room.enter(c4);
}

TEST(ServerRoom, ConfiguringUpdatingInvalidDeckWorks)
{
	DeckValidator falsy_dv(false);
	Client c("Client");
	c.expect(EvConfCase::kEnteringState);
	c.expect(EvConfCase::kDuelistEnters);
	auto room = TestRoom{falsy_dv, cdf, c, {}};
	Sig s1;
	s1.mutable_configuring()->mutable_update_deck();
	room.visit(c, s1);
	c.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_deck_status());
			EXPECT_TRUE(e.configuring().deck_status().has_deck_error());
		});
}

TEST(ServerRoom, ConfiguringUpdatingValidDeckWorks)
{
	Client c("Client");
	c.expect(EvConfCase::kEnteringState);
	c.expect(EvConfCase::kDuelistEnters);
	auto room = TestRoom{dv, cdf, c, {}};
	Sig s1;
	s1.mutable_configuring()->mutable_update_deck();
	room.visit(c, s1);
	c.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_deck_status());
			EXPECT_FALSE(e.configuring().deck_status().has_deck_error());
		});
}

TEST(ServerRoom, ConfiguringReadyingWithValidDeckWorks)
{
	Client c("Client");
	c.expect(EvConfCase::kEnteringState);
	c.expect(EvConfCase::kDuelistEnters);
	auto room = TestRoom{dv, cdf, c, {}};
	Sig s1;
	s1.mutable_configuring()->mutable_update_deck();
	room.visit(c, s1);
	c.expect(EvConfCase::kDeckStatus);
	Sig s2;
	// Becoming ready
	s2.mutable_configuring()->set_ready_status(true);
	room.visit(c, s2);
	c.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_update_duelist());
			auto& ud = e.configuring().update_duelist();
			EXPECT_EQ(ud.team(), 0);
			EXPECT_EQ(ud.pos(), 0);
			EXPECT_TRUE(ud.is_ready());
		});
	// Becoming not-ready
	s2.mutable_configuring()->set_ready_status(false);
	room.visit(c, s2);
	c.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_update_duelist());
			auto& ud = e.configuring().update_duelist();
			EXPECT_FALSE(ud.is_ready());
		});
}

TEST(ServerRoom, ConfiguringNonHostReadyingWorks)
{
	Client c1("Client1");
	c1.expect(EvConfCase::kEnteringState);
	c1.expect(EvConfCase::kDuelistEnters);
	auto room = TestRoom{dv, cdf, c1, {}};
	Sig s1;
	s1.mutable_configuring()->mutable_update_deck();
	room.visit(c1, s1);
	c1.expect(EvConfCase::kDeckStatus);
	Sig s2;
	s2.mutable_configuring()->set_ready_status(true);
	room.visit(c1, s2);
	c1.expect(EvConfCase::kUpdateDuelist);
	Client c2("Client2");
	c2.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_entering_state());
			ASSERT_TRUE(e.configuring().entering_state().has_options());
			auto& ds = e.configuring().entering_state().duelists();
			ASSERT_EQ(ds.size(), 1);
			auto& d = ds[0];
			EXPECT_EQ(d.team(), 0);
			EXPECT_EQ(d.pos(), 0);
			EXPECT_TRUE(d.has_user());
			EXPECT_EQ(d.user().name(), "Client1");
			EXPECT_TRUE(d.is_ready());
		});
	c2.expect(EvConfCase::kDuelistEnters);
	c1.expect(EvConfCase::kDuelistEnters);
	room.enter(c2);
	room.visit(c2, s1);
	c2.expect(EvConfCase::kDeckStatus);
	room.visit(c2, s2);
	auto exp = [](EvRef e)
	{
		ASSERT_TRUE(e.has_configuring());
		ASSERT_TRUE(e.configuring().has_update_duelist());
		auto& ud = e.configuring().update_duelist();
		EXPECT_TRUE(ud.has_user());
		EXPECT_EQ(ud.user().name(), "Client2");
		EXPECT_TRUE(ud.is_ready());
	};
	c1.expect(exp);
	c2.expect(exp);
}

} // namespace
