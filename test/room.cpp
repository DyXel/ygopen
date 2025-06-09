/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <functional>
#include <gtest/gtest.h>
#include <random>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>
#include <ygopen/server/room.hpp>

namespace
{

using Ev = YGOpen::Server::RoomEvent;
using EvRef = Ev const&;

using EvCase = Ev::TCase;
using EvConfCase = Ev::Configuring::TCase;
using EvDFTCase = Ev::DecidingFirstTurn::TCase;

using Sig = YGOpen::Server::RoomSignal;

class Client
{
public:
	using Expectation = std::function<void(EvRef)>;

	explicit Client(std::string_view name) noexcept
	{
		user_.set_name(name.data(), name.size());
	}

	Client(Client const&) = delete;
	Client(Client&&) noexcept = default;
	auto operator=(Client const&) -> Client& = delete;
	auto operator=(Client&&) noexcept -> Client& = delete;

	auto send(EvRef e) noexcept -> void { events_received_.push_back(e); }

	auto fill_user(YGOpen::Proto::User& user) const noexcept { user = user_; }

	auto expect(Expectation f) noexcept -> void
	{
		expectations_.emplace_back(std::move(f));
	}

	auto expect(EvConfCase ec) noexcept -> void
	{
		expectations_.emplace_back(
			[ec](EvRef e)
			{
				ASSERT_EQ(e.t_case(), EvCase::kConfiguring);
				ASSERT_EQ(e.configuring().t_case(), ec);
			});
	}

	auto expect(EvDFTCase ec) noexcept -> void
	{
		expectations_.emplace_back(
			[ec](EvRef e)
			{
				ASSERT_EQ(e.t_case(), EvCase::kDecidingFirstTurn);
				ASSERT_EQ(e.deciding_first_turn().t_case(), ec);
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

template<typename... Cs>
[[nodiscard]] auto wrap(Cs&... clients)
{
	class Clients
	{
	public:
		explicit Clients(Cs&... clients) : clients_(clients...) {}

		auto expect(Client::Expectation f) noexcept -> void
		{
			std::apply([f](Cs&... cs) { (cs.expect(f), ...); }, clients_);
		}

		auto expect(EvConfCase ec) noexcept -> void
		{
			std::apply([ec](Cs&... cs) { (cs.expect(ec), ...); }, clients_);
		}

		auto expect(EvDFTCase ec) noexcept -> void
		{
			std::apply([ec](Cs&... cs) { (cs.expect(ec), ...); }, clients_);
		}

	private:
		std::tuple<Cs&...> clients_;
	};
	return Clients(clients...);
}

class DeckValidator
{
public:
	explicit DeckValidator(bool check_status = true) noexcept
		: check_status_(check_status)
	{}

	auto check(std::string_view /*unused*/,
	           YGOpen::Proto::Deck const& /*unused*/,
	           YGOpen::Proto::DeckError& /*unused*/) const noexcept -> bool
	{
		return check_status_;
	}

private:
	bool check_status_;
} const dv;

class CoreDuelFactory
{
public:
	auto make_duel(/*TODO*/) const noexcept -> void {}
} const cdf;

class RandomNumberGenerator
{
public:
	using result_type = unsigned;

	explicit RandomNumberGenerator(std::vector<result_type> results) noexcept
		: results_(std::move(results)), current_(0)
	{}

	constexpr static auto min() -> result_type { return 0; }
	constexpr static auto max() -> result_type { return ~result_type{}; }

	auto operator()() -> result_type
	{
		if(current_ < results_.size())
			return results_[current_++];
		return min();
	}

private:
	std::vector<result_type> results_;
	size_t current_;
} const rng{{}};
static_assert(RandomNumberGenerator::min() != RandomNumberGenerator::max());

struct TestRoomTraits
{
	using ClientType = Client;
	using DeckValidatorType = DeckValidator;
	using CoreDuelFactoryType = CoreDuelFactory;
	using RNGType = RandomNumberGenerator;
};

using TestRoom = YGOpen::Server::BasicRoom<TestRoomTraits>;

auto default_room_with_host(Client& c,
                            YGOpen::Proto::Room::Options options) noexcept
	-> TestRoom
{
	return TestRoom{dv, cdf, rng, c, std::move(options)};
}

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
			auto const& de = e.configuring().duelist_enters();
			EXPECT_EQ(de.team(), 0);
			EXPECT_EQ(de.pos(), 0);
			EXPECT_TRUE(de.is_host());
		});
	auto room = default_room_with_host(c, {});
}

TEST(ServerRoom, ConstructionOptionsClampingWorks)
{
	Client c("Client");
	YGOpen::Proto::Room::Options options;
	options.add_max_duelists(99); // NOLINT: Testing clamping
	// NOTE: Only setting one on purpose, to test that the room fills the
	//       missing value for the second team.
	c.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_entering_state());
			ASSERT_TRUE(e.configuring().entering_state().has_options());
			auto const& opts = e.configuring().entering_state().options();
			EXPECT_EQ(opts.max_duelists_size(), 2);
			EXPECT_EQ(opts.max_duelists(0), 3);
			EXPECT_EQ(opts.max_duelists(1), 1);
			using FTDM = YGOpen::Proto::Room::FirstTurnDecideMethod;
			EXPECT_EQ(opts.ftdm(), FTDM::FTDM_HOST_CHOOSES);
		});
	c.expect(EvConfCase::kDuelistEnters);
	auto room = default_room_with_host(c, options);
}

TEST(ServerRoom, ConfiguringStateJoining1V1Works)
{
	YGOpen::Proto::Room::Options options;
	options.add_max_duelists(1);
	options.add_max_duelists(1);
	Client c1("Client1");
	c1.expect(EvConfCase::kEnteringState);
	c1.expect(EvConfCase::kDuelistEnters);
	auto room = default_room_with_host(c1, options);
	Client c2("Client2");
	c2.expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_entering_state());
			ASSERT_TRUE(e.configuring().entering_state().has_options());
			auto const& ds = e.configuring().entering_state().duelists();
			ASSERT_EQ(ds.size(), 1);
			auto const& d = ds[0];
			EXPECT_EQ(d.team(), 0);
			EXPECT_EQ(d.pos(), 0);
			EXPECT_TRUE(d.has_user());
			EXPECT_EQ(d.user().name(), "Client1");
			EXPECT_TRUE(d.is_host());
		});
	wrap(c1, c2).expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_duelist_enters());
			auto const& de = e.configuring().duelist_enters();
			EXPECT_EQ(de.team(), 1);
			EXPECT_EQ(de.pos(), 0);
			EXPECT_EQ(de.user().name(), "Client2");
			EXPECT_FALSE(de.is_host());
		});
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
	auto room = default_room_with_host(c1, options);
	int team = 1;
	int pos = 0;
	std::vector<Client> clients;
	constexpr int MAX_CLIENTS = 6;
	clients.reserve(MAX_CLIENTS - 1);
	for(int i = 0; i < MAX_CLIENTS - 1; i++)
	{
		auto const name = "Client" + std::to_string(i + 2);
		auto& c = clients.emplace_back(name);
		c.expect([dc = i + 1](EvRef e) { // dc == duelist_count
			SCOPED_TRACE("with dc == " + std::to_string(dc));
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_entering_state());
			ASSERT_TRUE(e.configuring().entering_state().has_options());
			auto const& ds = e.configuring().entering_state().duelists();
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
				auto const& d = ds[j];
				auto [t, p] = slots.at(dc - 1).at(j);
				EXPECT_EQ(d.team(), t);
				EXPECT_EQ(d.pos(), p);
			}
		});
		auto duelist_enters_expect = [team, pos, name](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_duelist_enters());
			auto const& de = e.configuring().duelist_enters();
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
	auto room = default_room_with_host(c1, options);
	Client c2("Client2");
	c2.expect(EvConfCase::kEnteringState);
	c2.expect(EvConfCase::kDuelistEnters);
	c1.expect(EvConfCase::kDuelistEnters);
	room.enter(c2);
	Client c3("Client3");
	c3.expect(EvConfCase::kEnteringState);
	wrap(c1, c2, c3)
		.expect(
			[](EvRef e)
			{
				ASSERT_TRUE(e.has_spectator_count());
				ASSERT_EQ(e.spectator_count(), 1);
			});
	room.enter(c3);
	Client c4("Client4");
	c4.expect(EvConfCase::kEnteringState);
	wrap(c1, c2, c3, c4)
		.expect(
			[](EvRef e)
			{
				ASSERT_TRUE(e.has_spectator_count());
				ASSERT_EQ(e.spectator_count(), 2);
			});
	room.enter(c4);
}

TEST(ServerRoom, ConfiguringUpdatingInvalidDeckWorks)
{
	DeckValidator falsy_dv(false);
	Client c("Client");
	c.expect(EvConfCase::kEnteringState);
	c.expect(EvConfCase::kDuelistEnters);
	auto room = TestRoom{falsy_dv, cdf, rng, c, {}};
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
	auto room = default_room_with_host(c, {});
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
	auto room = default_room_with_host(c, {});
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
			auto const& ud = e.configuring().update_duelist();
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
			auto const& ud = e.configuring().update_duelist();
			EXPECT_FALSE(ud.is_ready());
		});
}

TEST(ServerRoom, ConfiguringNonHostReadyingWorks)
{
	Client c1("Client1");
	c1.expect(EvConfCase::kEnteringState);
	c1.expect(EvConfCase::kDuelistEnters);
	auto room = default_room_with_host(c1, {});
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
			auto const& ds = e.configuring().entering_state().duelists();
			ASSERT_EQ(ds.size(), 1);
			auto const& d = ds[0];
			EXPECT_EQ(d.team(), 0);
			EXPECT_EQ(d.pos(), 0);
			EXPECT_TRUE(d.has_user());
			EXPECT_EQ(d.user().name(), "Client1");
			EXPECT_TRUE(d.is_ready());
		});
	wrap(c1, c2).expect(EvConfCase::kDuelistEnters);
	room.enter(c2);
	room.visit(c2, s1);
	c2.expect(EvConfCase::kDeckStatus);
	room.visit(c2, s2);
	wrap(c1, c2).expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_configuring());
			ASSERT_TRUE(e.configuring().has_update_duelist());
			auto const& ud = e.configuring().update_duelist();
			EXPECT_TRUE(ud.has_user());
			EXPECT_EQ(ud.user().name(), "Client2");
			EXPECT_TRUE(ud.is_ready());
		});
}

TEST(ServerRoom, ConfiguringStartDuelWorks)
{
	Client c1("Client1");
	c1.expect(EvConfCase::kEnteringState);
	c1.expect(EvConfCase::kDuelistEnters);
	auto room = default_room_with_host(c1, {});
	Sig s1;
	s1.mutable_configuring()->mutable_update_deck();
	room.visit(c1, s1);
	c1.expect(EvConfCase::kDeckStatus);
	Sig s2;
	s2.mutable_configuring()->set_ready_status(true);
	room.visit(c1, s2);
	c1.expect(EvConfCase::kUpdateDuelist);
	Client c2("Client2");
	c2.expect(EvConfCase::kEnteringState);
	wrap(c1, c2).expect(EvConfCase::kDuelistEnters);
	room.enter(c2);
	room.visit(c2, s1);
	c2.expect(EvConfCase::kDeckStatus);
	room.visit(c2, s2);
	wrap(c1, c2).expect(EvConfCase::kUpdateDuelist);
	Sig s3;
	s3.mutable_configuring()->set_start_dueling(true);
	room.visit(c1, s3);
	wrap(c1, c2).expect(EvDFTCase::kEnteringState);
	c1.expect(EvDFTCase::kDecideFirstTurn);
}

class ServerRoomDecidingFirstTurn : public ::testing::Test
{
protected:
	Client c1{"Client1"};
	Client c2{"Client2"};

	auto SetUp() -> void override
	{
		s1_.mutable_configuring()->mutable_update_deck();
		s2_.mutable_configuring()->set_ready_status(true);
		s3_.mutable_configuring()->set_start_dueling(true);
		c1.expect(EvConfCase::kEnteringState);
		c1.expect(EvConfCase::kDuelistEnters);
	}

	[[nodiscard]] auto emplace_room(RandomNumberGenerator rng,
	                                YGOpen::Proto::Room::Options options)
		-> TestRoom&
	{
		room_.emplace(dv, cdf, std::move(rng), c1, std::move(options));
		auto& room = *room_;
		c1.expect(EvConfCase::kDeckStatus);
		room.visit(c1, s1_);
		c1.expect(EvConfCase::kUpdateDuelist);
		room.visit(c1, s2_);
		c2.expect(EvConfCase::kEnteringState);
		wrap(c1, c2).expect(EvConfCase::kDuelistEnters);
		room.enter(c2);
		c2.expect(EvConfCase::kDeckStatus);
		room.visit(c2, s1_);
		wrap(c1, c2).expect(EvConfCase::kUpdateDuelist);
		room.visit(c2, s2_);
		wrap(c1, c2).expect(EvDFTCase::kEnteringState);
		room.visit(c1, s3_);
		return room;
	}

private:
	Sig s1_, s2_, s3_;
	std::optional<TestRoom> room_;
};

TEST_F(ServerRoomDecidingFirstTurn, HostDecidingWorks)
{
	auto& room = emplace_room(rng, {});
	c1.expect(EvDFTCase::kDecideFirstTurn);
	Sig s;
	s.mutable_deciding_first_turn()->set_team_going_first(1);
	room.visit(c1, s);
	wrap(c1, c2).expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_deciding_first_turn());
			ASSERT_TRUE(e.deciding_first_turn().has_result());
			auto const& result = e.deciding_first_turn().result();
			EXPECT_EQ(result.team_going_first(), 1);
		});
}

TEST_F(ServerRoomDecidingFirstTurn, RPSWorks)
{
	YGOpen::Proto::Room::Options options;
	options.set_ftdm(YGOpen::Proto::Room::FTDM_RPS);
	auto& room = emplace_room(rng, options);
	// TODO
}

TEST_F(ServerRoomDecidingFirstTurn, DiceRollWorks)
{
	RandomNumberGenerator die_rolls({0, 0, 5, 5, 9, 9, 0, 1}); // NOLINT
	YGOpen::Proto::Room::Options options;
	options.set_ftdm(YGOpen::Proto::Room::FTDM_DICE);
	static_cast<void>(emplace_room(std::move(die_rolls), options));
	wrap(c1, c2).expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_deciding_first_turn());
			ASSERT_TRUE(e.deciding_first_turn().has_result());
			auto const& result = e.deciding_first_turn().result();
			ASSERT_TRUE(result.has_dice());
			auto const& dice_results = result.dice().results();
			ASSERT_EQ(dice_results.size(), 4);
			EXPECT_EQ(dice_results[0].team0(), 1);
			EXPECT_EQ(dice_results[0].team1(), 1);
			EXPECT_EQ(dice_results[1].team0(), 6);
			EXPECT_EQ(dice_results[1].team1(), 6);
			EXPECT_EQ(dice_results[2].team0(), 4);
			EXPECT_EQ(dice_results[2].team1(), 4);
			EXPECT_EQ(dice_results[3].team0(), 1);
			EXPECT_EQ(dice_results[3].team1(), 2);
			EXPECT_EQ(result.team_going_first(), 1);
		});
	c2.expect(EvDFTCase::kDecideFirstTurn);
}

TEST_F(ServerRoomDecidingFirstTurn, CoinTossWorks)
{
	RandomNumberGenerator coin_toss({1});
	YGOpen::Proto::Room::Options options;
	options.set_ftdm(YGOpen::Proto::Room::FTDM_COIN);
	static_cast<void>(emplace_room(coin_toss, options));
	wrap(c1, c2).expect(
		[](EvRef e)
		{
			ASSERT_TRUE(e.has_deciding_first_turn());
			ASSERT_TRUE(e.deciding_first_turn().has_result());
			auto const& result = e.deciding_first_turn().result();
			ASSERT_TRUE(result.has_coin_result());
			auto const coin_result = result.coin_result();
			EXPECT_EQ(coin_result,
		              Ev::DecidingFirstTurn::Result::COIN_RESULT_TAILS);
			EXPECT_EQ(result.team_going_first(), 1);
		});
	c2.expect(EvDFTCase::kDecideFirstTurn);
}

} // namespace
