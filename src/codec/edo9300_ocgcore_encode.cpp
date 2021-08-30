/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include "ygopen/codec/edo9300_ocgcore_encode.hpp"

#include <bitset>  // std::bitset
#include <cstring> // std::memcpy
#include <google/protobuf/arena.h>
#include <ygopen/duel/constants/controller.hpp>
#include <ygopen/duel/constants/location.hpp>
#include <ygopen/duel/constants/phase.hpp>
#include <ygopen/duel/constants/reason.hpp>
#include <ygopen/proto/duel/msg.hpp>

// #define YGOPEN_ENCODER_DEBUG

namespace YGOpen::Codec::Edo9300::OCGCore
{

namespace
{

// Types used to read the right values size from core messages:
using CPlayer = uint8_t;   // Used for controller too.
using CCount = uint32_t;   // Count type, used for arrays read.
using CCode = uint32_t;    // Card code type.
using CLoc = uint32_t;     // Location type.
using CSeq = uint32_t;     // Sequence type.
using CPos = uint32_t;     // Position type.
using CEffect = uint64_t;  // Effect descriptor type.
using CCounter = uint32_t; // Counter type.
using CField = uint32_t;   // Type used to store zone bits.
using CReason = uint32_t;  // Core reason on why something happened.
using CQSize = uint16_t;   // Query size.
using CQFlag = uint32_t;   // Query flag.

// Smaller, irregular versions of the above types. These should be changed to
// the above values in the future.
using CSCount = uint8_t; // Count type, used for arrays read.
using CSLoc = uint8_t;   // Location type.
using CSSeq = uint8_t;   // Sequence type.
using CSPos = uint8_t;   // Position type.

constexpr auto CORE_LOC_INFO_SIZE =
	sizeof(CPlayer) + sizeof(CSLoc) + sizeof(CSeq) + sizeof(CPos);

constexpr int OSEQ_INVALID = -1;

#include "ocgcore_messages.inl"

constexpr auto is_special_msg(OCGCoreMsgValue core_msg) noexcept -> bool
{
	switch(core_msg)
	{
	case MSG_WIN:
	case MSG_HINT:
	case MSG_SWAP_GRAVE_DECK:
	case MSG_REVERSE_DECK:
	case MSG_DECK_TOP:
	case MSG_MOVE:
	case MSG_DRAW:
	case MSG_CARD_HINT:
	case MSG_MATCH_KILL:
		return true;
	default:
		return false;
	}
}

#ifndef YGOPEN_ENCODER_DEBUG

template<typename... Args>
constexpr auto log([[maybe_unused]] Args&&... args) noexcept -> void
{}

#else

constexpr char const* const YGOPEN_ENCODER_BASE_LOG = "@ygopen_encoder: ";

template<typename... Args>
constexpr auto log(Args&&... args) noexcept -> void
{
	if constexpr(sizeof...(Args) != 0U)
	{
		std::cout << YGOPEN_ENCODER_BASE_LOG;
		(std::cout << ... << args);
		std::cout << '\n';
	}
}

#endif // YGOPEN_ENCODER_DEBUG

template<typename T, typename... Args>
constexpr auto read(uint8_t const*& ptr, Args&&... args) noexcept -> T
{
	log(std::forward<Args>(args)...);
	T value{};
	std::memcpy(&value, ptr, sizeof(T));
	ptr += sizeof(T); // NOLINT: No alignment issues since type is uint8_t.
	return value;
}

constexpr auto read_attribute(uint8_t const*& ptr) noexcept -> auto
{
	return read<uint32_t>(ptr, "attribute");
}

template<typename... Args>
constexpr auto read_bool(uint8_t const*& ptr, Args&&... args) noexcept -> bool
{
	return read<uint8_t>(ptr, std::forward<Args>(args)...) != 0U;
}

constexpr auto read_con(uint8_t const*& ptr) noexcept -> auto
{
	return static_cast<Duel::Controller>(read<CPlayer>(ptr, "player"));
}

constexpr auto read_link_arrow(uint8_t const*& ptr) noexcept -> auto
{
	return read<uint32_t>(ptr, "link arrow");
}

template<typename T = CLoc>
constexpr auto read_loc(uint8_t const*& ptr) noexcept -> auto
{
	return static_cast<Duel::Location>(read<T>(ptr, "location"));
}

template<typename T = CPos>
constexpr auto read_pos(uint8_t const*& ptr) noexcept -> auto
{
	return static_cast<uint32_t>(read<T>(ptr, "position"));
}

constexpr auto read_race(uint8_t const*& ptr) noexcept -> auto
{
	return static_cast<uint64_t>(read<uint32_t>(ptr, "race"));
}

constexpr auto read_reason(uint8_t const*& ptr) noexcept -> auto
{
	return static_cast<uint64_t>(read<uint32_t>(ptr, "reason"));
}

constexpr auto read_status(uint8_t const*& ptr) noexcept -> auto
{
	return read<uint32_t>(ptr, "status");
}

constexpr auto read_type(uint8_t const*& ptr) noexcept -> auto
{
	return read<uint32_t>(ptr, "type");
}

inline auto read_counter(uint8_t const*& ptr,
                         YGOpen::Proto::Duel::Counter& counter) noexcept -> void
{
	auto const c = read<CCounter>(ptr, "counter");
	counter.set_type(c & 0xFFFFU); // NOLINT
	counter.set_count(c >> 16U);   // NOLINT
}

inline auto read_effect(uint8_t const*& ptr,
                        YGOpen::Proto::Duel::Effect& effect) noexcept -> void
{
	auto const ed = read<CEffect>(ptr, "effect desc");
	effect.set_code(ed >> 20U);      // NOLINT: From utility.lua's Stringid.
	effect.set_index(ed & 0xFFFFFU); // NOLINT: From utility.lua's Stringid.
}

template<typename Loc, typename Seq>
constexpr auto fix_spell_loc_seq(Loc loc, Seq seq) noexcept
	-> std::pair<Loc, Seq>
{
	using namespace YGOpen::Duel;
	constexpr Seq SPELL_ZONE_LIMIT = 4U;
	constexpr Seq FIELD_ZONE_SEQUENCE = 5U;
	if((loc & LOCATION_SPELL_ZONE) && seq > SPELL_ZONE_LIMIT)
	{
		if(seq == FIELD_ZONE_SEQUENCE)
		{
			loc = LOCATION_FIELD_ZONE;
		}
		else // Pendulum zone.
		{
			loc = LOCATION_PENDULUM_ZONE;
			seq--; // Skip field zone.
		}
		seq -= SPELL_ZONE_LIMIT;
	}
	return {loc, seq};
}

template<typename Loc = CSLoc, typename Seq = CSeq, typename Pos = CPos>
inline auto read_loc_info(uint8_t const*& ptr,
                          YGOpen::Proto::Duel::Place& place) noexcept -> void
{
	using namespace YGOpen::Duel;
	constexpr Loc LOCATION_OVERLAY = 0x80U;
	place.set_con(read_con(ptr));
	auto const loc = read_loc<Loc>(ptr);
	place.set_loc(loc & (~LOCATION_OVERLAY));
	if constexpr(!std::is_void<Seq>())
	{
		auto const loc_seq =
			fix_spell_loc_seq(place.loc(), read<Seq>(ptr, "sequence"));
		place.set_loc(loc_seq.first);
		place.set_seq(loc_seq.second);
		if constexpr(!std::is_void<Pos>())
		{
			auto const pos = read_pos<Pos>(ptr);
			if(loc & LOCATION_OVERLAY)
				place.set_oseq(pos);
			else
				place.set_oseq(OSEQ_INVALID);
		}
		else
		{
			place.set_oseq(OSEQ_INVALID);
		}
	}
}

template<typename Count, typename Loc, typename Seq, typename Pos,
         typename Next>
auto read_card_list(uint8_t const*& ptr, Next next) noexcept -> void
{
	auto const count = read<Count>(ptr, ".size()");
	log("total size of card list: ", static_cast<int>(count));
	for(Count i = 0; i < count; i++)
	{
		log("reading card number ", static_cast<int>(i));
		auto* place = next();
		// Skip card code.
		read<CCode>(ptr);
		// Location information.
		read_loc_info<Loc, Seq, Pos>(ptr, *place);
	}
}

template<typename Count, typename Loc, typename Seq, typename Pos,
         typename Next, typename Post>
auto read_card_list(uint8_t const*& ptr, Next next, Post post) noexcept -> void
{
	auto const count = read<Count>(ptr, ".size()");
	log("total size of card list: ", static_cast<int>(count));
	for(Count i = 0; i < count; i++)
	{
		log("reading card number ", static_cast<int>(i));
		auto* place = next();
		// Skip card code.
		read<CCode>(ptr, "skipped card code");
		// Location information.
		read_loc_info<Loc, Seq, Pos>(ptr, *place);
		// Post read.
		post(*place);
	}
}

template<typename... Args>
constexpr auto skip(uint8_t const*& ptr, size_t bytes, Args&&... args) noexcept
	-> void
{
	log("skipping ", bytes, " bytes. ", std::forward<Args>(args)...);
	ptr += bytes; // NOLINT
}

template<typename... Args>
constexpr auto back(uint8_t const*& ptr, size_t bytes, Args&&... args) noexcept
	-> void
{
	log("going back ", bytes, " bytes. ", std::forward<Args>(args)...);
	ptr -= bytes; // NOLINT
}

template<typename Next>
inline auto unpack_zones(CField zones, CPlayer invert, Next next) noexcept
	-> void
{
	static constexpr CField FIELD_HALF = 16U;
	static constexpr CField FIELD_MZONE_COUNT = 7U; // 5 MMZ + 2 EMZ.
	static constexpr CField FIELD_SZONE_COUNT = 5U;
	static constexpr CField FIELD_PZONE_COUNT = 2U;
	using namespace YGOpen::Duel;
	auto add_place = [&](CPlayer con, Location loc, CSeq seq)
	{
		auto* place = next();
		place->set_con(static_cast<Controller>(con));
		place->set_loc(loc);
		place->set_seq(seq);
		place->set_oseq(OSEQ_INVALID);
	};
	auto iterate_half = [&](CPlayer con, CField offset)
	{
		std::bitset<FIELD_HALF> const half(zones >> offset);
		CField i = 0U;
		// Monster Zones.
		for(CSeq seq = 0U; seq < FIELD_MZONE_COUNT; i++, seq++)
			if(!half[i])
				add_place(con, LOCATION_MONSTER_ZONE, seq);
		i++; // NOTE: Unused bit.
		// Spell Zones.
		for(CSeq seq = 0U; seq < FIELD_SZONE_COUNT; i++, seq++)
			if(!half[i])
				add_place(con, LOCATION_SPELL_ZONE, seq);
		// Field Spell Zone.
		if(!half[i])
			add_place(con, LOCATION_FIELD_ZONE, 0U);
		i++;
		// Pendulum Zones.
		for(CSeq seq = 0U; seq < FIELD_PZONE_COUNT; i++, seq++)
			if(!half[i])
				add_place(con, LOCATION_PENDULUM_ZONE, seq);
	};
	iterate_half(invert, 0U);
	iterate_half(1U - invert, FIELD_HALF);
}

} // namespace

auto encode_one_query(uint8_t const* data,
                      Proto::Duel::Msg::Query::Data& q) noexcept
	-> EncodeOneQueryResult
{
	enum CoreQuery : CQFlag
	{
		QUERY_CODE = 0x1U,
		QUERY_POSITION = 0x2U,
		QUERY_ALIAS = 0x4U,
		QUERY_TYPE = 0x8U,
		QUERY_LEVEL = 0x10U,
		QUERY_RANK = 0x20U,
		QUERY_ATTRIBUTE = 0x40U,
		QUERY_RACE = 0x80U,
		QUERY_ATTACK = 0x100U,
		QUERY_DEFENSE = 0x200U,
		QUERY_BASE_ATTACK = 0x400U,
		QUERY_BASE_DEFENSE = 0x800U,
		// 	QUERY_REASON = 0x1000U,
		QUERY_REASON_CARD = 0x2000U,
		QUERY_EQUIP_CARD = 0x4000U,
		QUERY_TARGET_CARD = 0x8000U,
		QUERY_OVERLAY_CARD = 0x10000U,
		QUERY_COUNTERS = 0x20000U,
		QUERY_OWNER = 0x40000U,
		QUERY_STATUS = 0x80000U,
		QUERY_IS_PUBLIC = 0x100000U,
		QUERY_LSCALE = 0x200000U,
		QUERY_RSCALE = 0x400000U,
		QUERY_LINK = 0x800000U,
		QUERY_IS_HIDDEN = 0x1000000U,
		QUERY_COVER = 0x2000000U,
		QUERY_END = 0x80000000U,
	};
	EncodeOneQueryResult result{};
	decltype(data) const sentry = data;
	for(;;)
	{
		auto const size = read<CQSize>(data, "query size");
		auto const flag = read<CQFlag>(data, "query flag");
		switch(flag)
		{
		case QUERY_END:
		{
			log("finished parsing query");
			result.bytes_read = static_cast<size_t>(data - sentry);
			return result;
		}
// NOLINTNEXTLINE: No reflection :(
#define X(qtype, v, type)                                 \
	case qtype:                                           \
	{                                                     \
		q.mutable_##v()->set_value(read<type>(data, #v)); \
		break;                                            \
	}
		case QUERY_OWNER:
		{
			q.mutable_owner()->set_value(read_con(data));
			break;
		}
		case QUERY_IS_PUBLIC:
		{
			q.mutable_is_public()->set_value(read_bool(data, "is_public"));
			break;
		}
		case QUERY_IS_HIDDEN:
		{
			q.mutable_is_hidden()->set_value(read_bool(data, "is_hidden"));
			break;
		}
		case QUERY_POSITION:
		{
			q.mutable_position()->set_value(read_pos(data));
			break;
		}
			X(QUERY_COVER, cover, CCode)
		case QUERY_STATUS:
		{
			q.mutable_status()->set_value(read_status(data));
			break;
		}
			X(QUERY_CODE, code, CCode)
			X(QUERY_ALIAS, alias, CCode)
		case QUERY_TYPE:
		{
			q.mutable_type()->set_value(read_type(data));
			break;
		}
			X(QUERY_LEVEL, level, uint32_t)
			X(QUERY_RANK, xyz_rank, uint32_t)
		case QUERY_ATTRIBUTE:
		{
			q.mutable_attribute()->set_value(read_attribute(data));
			break;
		}
		case QUERY_RACE:
		{
			q.mutable_race()->set_value(read_race(data));
			break;
		}
			X(QUERY_BASE_ATTACK, base_atk, int32_t)
			X(QUERY_ATTACK, atk, int32_t)
			X(QUERY_BASE_DEFENSE, base_def, int32_t)
			X(QUERY_DEFENSE, def, int32_t)
			X(QUERY_LSCALE, pend_l_scale, uint32_t)
			X(QUERY_RSCALE, pend_r_scale, uint32_t)
#undef X
		case QUERY_LINK:
		{
			q.mutable_link_rate()->set_value(read<int32_t>(data, "link_rate"));
			q.mutable_link_arrow()->set_value(read_link_arrow(data));
			break;
		}
		case QUERY_COUNTERS:
		{
			auto* counters = q.mutable_counters();
			auto const count = read<CCount>(data, "counter count");
			for(CCount i = 0; i < count; i++)
				read_counter(data, *counters->add_values());
			break;
		}
		case QUERY_EQUIP_CARD:
		{
			read_loc_info(data, *q.mutable_equipped()->mutable_value());
			break;
		}
		case QUERY_TARGET_CARD:
		{
			auto* relations = q.mutable_relations();
			auto const count = read<CCount>(data, "target card count");
			for(CCount i = 0; i < count; i++)
				read_loc_info(data, *relations->add_values());
			break;
		}
		case QUERY_OVERLAY_CARD:
		{
			result.overlays_count = read<CCount>(data, "overlay count");
			result.overlays_ptr = data;
			skip(data, result.overlays_count * sizeof(uint32_t), "' codes");
			break;
		}
		default:
		{
			skip(data, size - sizeof(CQFlag), "unknown query type: ", flag);
			break;
		}
		}
	}
}

auto encode_one(google::protobuf::Arena& arena, IEncodeContext& context,
                uint8_t const* data) noexcept -> EncodeOneResult
{
	decltype(data) const sentry = data;
	auto const core_msg = read<OCGCoreMsgValue>(data);
	EncodeOneResult result{};
	log("core_msg: ", static_cast<int>(core_msg));
	using namespace google::protobuf;
	using namespace YGOpen::Duel;
	using namespace YGOpen::Proto::Duel;
	auto create_event = [&]() -> Msg::Event*
	{
		result.state = EncodeOneResult::State::OK;
		result.msg = Arena::CreateMessage<Msg>(&arena);
		return result.msg->mutable_event();
	};
	auto create_queries = [&]() -> RepeatedPtrField<Msg::Query>*
	{
		result.state = EncodeOneResult::State::OK;
		result.msg = Arena::CreateMessage<Msg>(&arena);
		return result.msg->mutable_queries();
	};
	auto create_request = [&]() -> Msg::Request*
	{
		result.state = EncodeOneResult::State::OK;
		result.msg = Arena::CreateMessage<Msg>(&arena);
		auto* request = result.msg->mutable_request();
		request->set_replier(read_con(data));
		return request;
	};
	auto set_state_swallowed = [&result]()
	{
		result.state = EncodeOneResult::State::SWALLOWED;
	};
	// for: MSG_SUMMONING, MSG_SPSUMMONING and MSG_FLIPSUMMONING.
	auto add_summoning = [&](Reason reason)
	{
		auto* update = create_event()->mutable_meta()->mutable_update();
		read<CCode>(data, "skipped card code");
		read_loc_info(data, *update->add_places());
		update->set_reason(reason);
	};
	// for: MSG_RANDOM_SELECTED, MSG_CARD_SELECTED and MSG_BECOME_TARGET.
	auto add_selected = [&]()
	{
		auto* selection = create_event()->mutable_meta()->mutable_selection();
		const auto count = read<CCount>(data, ".size()");
		for(CCount i = 0; i < count; i++)
			read_loc_info(data, *selection->add_selected_places());
	};
	//
	switch(core_msg)
	{
		/*
		 * Event messages.
		 */
	case MSG_CONFIRM_DECKTOP:
	case MSG_CONFIRM_EXTRATOP:
	{
		auto* confirm = create_event()->mutable_meta()->mutable_confirm();
		read_con(data);
		read_card_list<CCount, CSLoc, CSeq, void>(
			data, [&]() { return confirm->add_places(); });
		break;
	}
	case MSG_SHUFFLE_DECK:
	{
		auto* meta = create_event()->mutable_meta();
		auto* place = meta->mutable_shuffle_pile()->add_places();
		place->set_con(read_con(data));
		place->set_loc(LOCATION_MAIN_DECK);
		break;
	}
	case MSG_SHUFFLE_HAND:
	{
		auto* meta = create_event()->mutable_meta();
		auto* place = meta->mutable_shuffle_pile()->add_places();
		place->set_con(read_con(data));
		place->set_loc(LOCATION_HAND);
		const auto count = read<CCount>(data, ".size()");
		for(CCount i = 0; i < count; i++)
			read<CCode>(data, "skipped card code ", static_cast<int>(i));
		break;
	}
	case MSG_CONFIRM_CARDS:
	{
		// TODO: Specialize case where location is 0.
		auto* confirm = create_event()->mutable_meta()->mutable_confirm();
		read<CPlayer>(data, "skipped player");
		read_card_list<CCount, CSLoc, CSeq, void>(
			data, [&]() { return confirm->add_places(); });
		break;
	}
	case MSG_SHUFFLE_SET_CARD:
	{
		auto* shuffle = create_event()->mutable_card()->mutable_shuffle();
		read<CSLoc>(data, "location");
		auto const count = read<CSCount>(data, ".size()");
		for(CSCount i = 0; i < count; i++)
			read_loc_info(data, *shuffle->add_previous_places());
		for(CSCount i = 0; i < count; i++)
			read_loc_info(data, *shuffle->add_current_places());
		break;
	}
	case MSG_SHUFFLE_EXTRA:
	{
		auto* meta = create_event()->mutable_meta();
		auto* place = meta->mutable_shuffle_pile()->add_places();
		place->set_con(read_con(data));
		place->set_loc(LOCATION_EXTRA_DECK);
		auto const count = read<CCount>(data, ".size()");
		for(CCount i = 0; i < count; i++)
			read<CCode>(data, "skipped card code ", static_cast<int>(i));
		break;
	}
	case MSG_NEW_TURN:
	{
		create_event()->set_next_turn(read_con(data));
		break;
	}
	case MSG_NEW_PHASE:
	{
		create_event()->set_next_phase(
			static_cast<Phase>(read<uint16_t>(data)));
		break;
	}
	case MSG_POS_CHANGE:
	{
		auto* update = create_event()->mutable_meta()->mutable_update();
		read<CCode>(data, "skipped card code");
		read_loc_info<CSLoc, CSSeq, void>(data, *update->add_places());
		read<CSPos>(data, "skipped position (previous)");
		read<CSPos>(data, "skipped position (current)");
		break;
	}
	case MSG_SET:
	{
		auto* update = create_event()->mutable_meta()->mutable_update();
		read<CCode>(data, "skipped card code");
		read_loc_info<CSLoc, CSeq, void>(data, *update->add_places());
		read<CPos>(data, "skipped position (current)");
		break;
	}
	case MSG_SWAP:
	{
		auto* op =
			create_event()->mutable_card()->mutable_exchange()->add_ops();
		read<CCode>(data, "skipped card code");
		read_loc_info(data, *op->mutable_place_a());
		read<CCode>(data, "skipped card code");
		read_loc_info(data, *op->mutable_place_b());
		break;
	}
	case MSG_FIELD_DISABLED:
	{
		auto* zone_block = create_event()->mutable_zone_block();
		unpack_zones(read<CField>(data, "zones"), 0U,
		             [&]() { return zone_block->add_zones(); });
		break;
	}
	case MSG_SUMMONING:
	{
		add_summoning(REASON_SUMMON);
		break;
	}
	case MSG_SPSUMMONING:
	{
		add_summoning(REASON_SPSUMMON);
		break;
	}
	case MSG_FLIPSUMMONING:
	{
		add_summoning(REASON_FLIP);
		break;
	}
	case MSG_CHAINING:
	{
		auto* push = create_event()->mutable_chain_stack()->mutable_push();
		read<CCode>(data, "skipped card code");
		read_loc_info(data, *push->mutable_card_place());
		read_loc_info<CSLoc, CSeq, void>(data, *push->mutable_place());
		read_effect(data, *push->mutable_effect());
		read<uint32_t>(data, "chain num");
		break;
	}
	case MSG_CHAIN_SOLVED:
	{
		read<uint8_t>(data, "chain num");
		create_event()->mutable_chain_stack()->set_pop(true);
		break;
	}
// NOLINTNEXTLINE: No reflection :(
#define LP(t)                                           \
	do                                                  \
	{                                                   \
		auto* lp = create_event()->mutable_lp();        \
		lp->set_controller(read_con(data));             \
		lp->set_##t(read<uint32_t>(data, "lp amount")); \
	} while(0)
	case MSG_DAMAGE:
	{
		LP(damage);
		break;
	}
	case MSG_RECOVER:
	{
		LP(recover);
		break;
	}
	case MSG_LPUPDATE:
	{
		LP(become);
		break;
	}
	case MSG_PAY_LPCOST:
	{
		LP(pay);
		break;
	}
#undef LP
	case MSG_RANDOM_SELECTED:
	{
		read<CPlayer>(data, "skipped player");
		add_selected();
		break;
	}
	case MSG_CARD_SELECTED:
	case MSG_BECOME_TARGET:
	{
		add_selected();
		break;
	}
	case MSG_CARD_TARGET:
	case MSG_CANCEL_TARGET: // TODO: Maybe ignore?
	{
		auto* selection = create_event()->mutable_meta()->mutable_selection();
		read_loc_info(data, *selection->mutable_selector());
		read_loc_info(data, *selection->add_selected_places());
		break;
	}
	case MSG_ADD_COUNTER:
	case MSG_REMOVE_COUNTER:
	{
		auto* update = create_event()->mutable_meta()->mutable_update();
		read<uint16_t>(data, "skipped counter type info");
		read_loc_info<CSLoc, CSSeq, void>(data, *update->add_places());
		read<uint16_t>(data, "skipped counter count info");
		break;
	}
	case MSG_ATTACK:
	{
		auto* attack = create_event()->mutable_meta()->mutable_attack();
		read_loc_info(data, *attack->mutable_attacker());
		auto* attack_target = attack->mutable_attack_target();
		read_loc_info(data, *attack_target);
		if(attack_target->loc() == 0U)
			attack->clear_attack_target();
		break;
	}
// NOLINTNEXTLINE: No reflection :(
#define RESULT(r, t)                                                        \
	do                                                                      \
	{                                                                       \
		auto* res = create_event()->mutable_result()->mutable_##r();        \
		res->set_actor(read_con(data));                                     \
		const auto count = read<CSCount>(data, "count");                    \
		for(CSCount i = 0; i < count; i++)                                  \
			res->add_values(static_cast<t>(read<uint8_t>(data, "result"))); \
	} while(0)
	case MSG_TOSS_COIN:
	{
		RESULT(coin, bool);
		break;
	}
	case MSG_TOSS_DICE:
	{
		RESULT(dice, uint32_t);
		break;
	}
#undef RESULT
	case MSG_HAND_RES:
	{
		// TODO: make sure these are properly mapped to the enum.
		auto* rps = create_event()->mutable_result()->mutable_rps();
		const auto hands_results = read<uint8_t>(data, "hand results");
		auto to_rps = [](uint8_t v)
		{
			return static_cast<RockPaperScissors>(v);
		};
		rps->add_values(to_rps((hands_results & 0x3) - 1));
		rps->add_values(to_rps(((hands_results >> 2) & 0x3) - 1));
		break;
	}
	case MSG_START:
	{
		auto* state = create_event()->mutable_board()->mutable_state();
		auto* resize = state->mutable_resize();
		read<uint8_t>(data, "compatibility padding (duel_rule)");
		state->set_core_flags(0U);
		state->set_turn_counter(0);
		for(int i = 0; i < 2; i++)
			state->add_lps(read<uint32_t>(data, "LP"));
		auto add_pile = [&](int con, Location loc, uint16_t count)
		{
			auto* op = resize->add_ops();
			auto* place = op->mutable_place();
			place->set_con(static_cast<Controller>(con));
			place->set_loc(loc);
			place->set_seq(0U);
			place->set_oseq(OSEQ_INVALID);
			op->set_count(count);
		};
		for(int i = 0; i < 2; i++)
		{
			add_pile(i, LOCATION_MAIN_DECK, read<uint16_t>(data, "MD"));
			add_pile(i, LOCATION_EXTRA_DECK, read<uint16_t>(data, "ED"));
		}
		break;
	}
	case MSG_TAG_SWAP:
	{
		auto* exchange = create_event()->mutable_board()->mutable_exchange();
		auto* resize = exchange->mutable_resize();
		const auto controller = read_con(data);
		exchange->set_con(controller);
		auto add_pile = [&](Location loc, CCount count)
		{
			auto* op = resize->add_ops();
			auto* place = op->mutable_place();
			place->set_con(controller);
			place->set_loc(loc);
			place->set_seq(0U);
			place->set_oseq(OSEQ_INVALID);
			op->set_count(count);
		};
		const auto main_size = read<CCount>(data, "main");
		const auto extra_size = read<CCount>(data, "extra");
		read<CCount>(data, "skipped extra_p_count");
		const auto hand_size = read<CCount>(data, "hand");
		add_pile(LOCATION_MAIN_DECK, main_size);
		add_pile(LOCATION_EXTRA_DECK, extra_size);
		add_pile(LOCATION_HAND, hand_size);
		read<CCode>(data, "skipped deck_reversed card code");
		for(size_t i = 0U; i < (extra_size + hand_size); i++)
		{
			read<CCode>(data, "skipped card code");
			read<CPos>(data, "skipped position (current)");
		}
		break;
	}
	case MSG_RELOAD_FIELD:
	{
		auto* state = create_event()->mutable_board()->mutable_state();
		auto* add = state->mutable_add();
		auto* resize = state->mutable_resize();
		state->set_core_flags(read<uint32_t>(data, "core flags"));
		state->set_turn_counter(-1);
		auto read_controller_state = [&](Controller con)
		{
			auto add_one = [&](Location loc, CSeq seq, int oseq)
			{
				auto* place = add->add_places();
				place->set_con(con);
				place->set_loc(loc);
				place->set_seq(seq);
				place->set_oseq(oseq);
			};
			auto parse_card_zone_array = [&](CSeq max_seq, Location loc)
			{
				for(CSeq seq = 0U; seq < max_seq; seq++)
				{
					if(read<uint8_t>(data, "has card") == 0U)
						continue;
					read<CSPos>(data, "skipped position (current)");
					add_one(loc, seq, OSEQ_INVALID);
					const auto mats = read<CCount>(data, "xyz mats count");
					for(CCount oseq = 0U; oseq < mats; oseq++)
						add_one(loc, seq, oseq);
				}
			};
			auto add_pile = [&](Location loc, CCount count)
			{
				auto* op = resize->add_ops();
				auto* place = op->mutable_place();
				place->set_con(con);
				place->set_loc(loc);
				place->set_seq(0U);
				place->set_oseq(OSEQ_INVALID);
				op->set_count(count);
			};
			log("reading controller ", con);
			state->add_lps(read<uint32_t>(data, "LP"));
			parse_card_zone_array(7U, LOCATION_MONSTER_ZONE);
			parse_card_zone_array(8U, LOCATION_SPELL_ZONE);
			add_pile(LOCATION_MAIN_DECK, read<CCount>(data, "main"));
			add_pile(LOCATION_HAND, read<CCount>(data, "hand"));
			add_pile(LOCATION_GRAVEYARD, read<CCount>(data, "graveyard"));
			add_pile(LOCATION_BANISHED, read<CCount>(data, "banished"));
			add_pile(LOCATION_EXTRA_DECK, read<CCount>(data, "extra"));
			read<CCount>(data, "skipped extra_p_count");
		};
		read_controller_state(CONTROLLER_0);
		read_controller_state(CONTROLLER_1);
		const auto chain_count = read<CCount>(data, "chain count");
		for(CCount i = 0U; i < chain_count; i++)
		{
			auto* chain = state->add_chains();
			read<CCode>(data, "skipped card code");
			read_loc_info(data, *chain->mutable_card_place());
			read_loc_info<CSLoc, CSeq, void>(data, *chain->mutable_place());
			read_effect(data, *chain->mutable_effect());
		}
		break;
	}
	case MSG_PLAYER_HINT:
	{
		enum PlayerHintType : uint8_t
		{
			PLAYER_HINT_TYPE_DESC_ADD = 1U,
			PLAYER_HINT_TYPE_DESC_REMOVE = 2U,
		};
		auto* description =
			create_event()->mutable_meta()->mutable_description();
		description->set_con(read_con(data));
		auto type = read<PlayerHintType>(data, "player hint type");
		if(type == PLAYER_HINT_TYPE_DESC_ADD)
			read_effect(data, *description->mutable_add());
		else // type == PLAYER_HINT_TYPE_DESC_REMOVE
			read_effect(data, *description->mutable_remove());
		break;
	}
		/*
		 * Queries "containers".
		 */
	case MSG_UPDATE_DATA:
	{
		auto* queries = create_queries();
		auto const controller = read_con(data);
		auto const location = read_loc<CSLoc>(data);
		auto const total_size = read<uint32_t>(data, "total query size");
		uint32_t i = 0U;
		for(uint32_t read_size = 0U; read_size < total_size; i++)
		{
			if(read<CQSize>(data, "query size (initial)") == 0U)
			{
				read_size += sizeof(CQSize);
				continue;
			}
			back(data, sizeof(CQSize), "align for encode_one_query");
			auto* query = queries->Add();
			auto* place = query->mutable_place();
			place->set_con(controller);
			auto const loc_seq = fix_spell_loc_seq(location, i);
			place->set_loc(loc_seq.first);
			place->set_seq(loc_seq.second);
			place->set_oseq(OSEQ_INVALID);
			auto eqr = encode_one_query(data, *query->mutable_data());
			for(uint32_t i = 0; i < eqr.overlays_count; i++)
			{
				auto const code = read<CCode>(eqr.overlays_ptr, "overlay code");
				auto* overlay_query = queries->Add();
				auto* overlay_place = overlay_query->mutable_place();
				auto* overlay_data = overlay_query->mutable_data();
				overlay_place->set_con(controller);
				overlay_place->set_loc(loc_seq.first);
				overlay_place->set_seq(loc_seq.second);
				overlay_place->set_oseq(i);
				overlay_data->mutable_code()->set_value(code);
			}
			skip(data, eqr.bytes_read);
			read_size += eqr.bytes_read;
		}
		break;
	}
	case MSG_UPDATE_CARD:
	{
		auto* queries = create_queries();
		auto* query = queries->Add();
		auto& place = *query->mutable_place();
		read_loc_info<CSLoc, CSSeq, void>(data, place);
		auto eqr = encode_one_query(data, *query->mutable_data());
		for(uint32_t i = 0; i < eqr.overlays_count; i++)
		{
			auto const code = read<CCode>(eqr.overlays_ptr, "overlay code");
			auto* overlay_query = queries->Add();
			auto* overlay_place = overlay_query->mutable_place();
			auto* overlay_data = overlay_query->mutable_data();
			overlay_place->set_con(place.con());
			overlay_place->set_loc(place.loc());
			overlay_place->set_seq(place.seq());
			overlay_place->set_oseq(i);
			overlay_data->mutable_code()->set_value(code);
		}
		skip(data, eqr.bytes_read);
		break;
	}
		/*
		 * Request messages.
		 */
	case MSG_ANNOUNCE_ATTRIB:
	{
		auto* select_attribute = create_request()->mutable_select_attribute();
		select_attribute->set_count(read<CSCount>(data, "count"));
		select_attribute->set_attribute(read_attribute(data));
		break;
	}
	case MSG_SELECT_CARD:
	case MSG_SELECT_TRIBUTE:
	case MSG_SELECT_COUNTER:
	case MSG_SELECT_SUM:
	case MSG_SELECT_UNSELECT_CARD:
	{
		// TODO: Spare my soul!
		break;
	}
	case MSG_ANNOUNCE_CARD:
	case MSG_ANNOUNCE_CARD_FILTER:
	{
		// TODO
		break;
	}
	case MSG_SELECT_OPTION:
	{
		auto* select_effect = create_request()->mutable_select_effect();
		select_effect->set_count(1);
		auto const count = read<CSCount>(data, "count");
		for(CSCount i = 0; i < count; i++)
			read_effect(data, *select_effect->add_effects());
		break;
	}
	case MSG_SELECT_BATTLECMD:
	{
		auto* select_idle = create_request()->mutable_select_idle();
		select_idle->set_is_battle_cmd(true);
		{ // Activable cards
			auto const count = read<CCount>(data, "number of cards");
			for(CCount i = 0; i < count; i++)
			{
				auto& card = *select_idle->add_activable_cards();
				read<CCode>(data, "skipped card code");
				read_loc_info<CSLoc, CSeq, void>(data, *card.mutable_place());
				read_effect(data, *card.mutable_effect());
				skip(data, 1U, "normal_resolve_reset");
			}
		}
		{ // Can attack cards
			auto const count = read<CCount>(data, "number of cards");
			for(CCount i = 0; i < count; i++)
			{
				auto& card = *select_idle->add_can_attack_cards();
				read<CCode>(data, "skipped card code");
				read_loc_info<CSLoc, CSSeq, void>(data, *card.mutable_place());
				card.set_can_attack_directly(read<uint8_t>(data) != 0U);
			}
		}
		uint32_t phases = PHASE_UNSPECIFIED;
		if(read<uint8_t>(data, "to_mp2") != 0U)
			phases |= PHASE_MAIN_2;
		if(read<uint8_t>(data, "to_ep") != 0U)
			phases |= PHASE_END;
		select_idle->set_available_phase(static_cast<Phase>(phases));
		select_idle->set_can_shuffle(false);
		break;
	}
	case MSG_SELECT_IDLECMD:
	{
		auto* select_idle = create_request()->mutable_select_idle();
		select_idle->set_is_battle_cmd(false);
		{ // Summonable cards
			auto const count = read<CCount>(data, "number of cards");
			for(CCount i = 0; i < count; i++)
			{
				auto& place = *select_idle->add_summonable_cards();
				read<CCode>(data, "skipped card code");
				read_loc_info<CSLoc, CSeq, void>(data, place);
			}
		}
		{ // Special Summonable cards
			auto const count = read<CCount>(data, "number of cards");
			for(CCount i = 0; i < count; i++)
			{
				auto& place = *select_idle->add_spsummonable_cards();
				read<CCode>(data, "skipped card code");
				read_loc_info<CSLoc, CSeq, void>(data, place);
			}
		}
		{ // Repositionable cards
			auto const count = read<CCount>(data, "number of cards");
			for(CCount i = 0; i < count; i++)
			{
				auto& place = *select_idle->add_repositionable_cards();
				read<CCode>(data, "skipped card code");
				read_loc_info<CSLoc, CSSeq, void>(data, place);
			}
		}
		{ // Msetable cards
			auto const count = read<CCount>(data, "number of cards");
			for(CCount i = 0; i < count; i++)
			{
				auto& place = *select_idle->add_msetable_cards();
				read<CCode>(data, "skipped card code");
				read_loc_info<CSLoc, CSeq, void>(data, place);
			}
		}
		{ // Ssetable cards
			auto const count = read<CCount>(data, "number of cards");
			for(CCount i = 0; i < count; i++)
			{
				auto& place = *select_idle->add_ssetable_cards();
				read<CCode>(data, "skipped card code");
				read_loc_info<CSLoc, CSeq, void>(data, place);
			}
		}
		{ // Activable cards
			auto const count = read<CCount>(data, "number of cards");
			for(CCount i = 0; i < count; i++)
			{
				auto& card = *select_idle->add_activable_cards();
				read<CCode>(data, "skipped card code");
				read_loc_info<CSLoc, CSeq, void>(data, *card.mutable_place());
				read_effect(data, *card.mutable_effect());
				skip(data, 1U, "normal_resolve_reset");
			}
		}
		uint32_t phases = PHASE_UNSPECIFIED;
		if(read<uint8_t>(data, "to_bp") != 0U)
			phases |= PHASE_BATTLE;
		if(read<uint8_t>(data, "to_ep") != 0U)
			phases |= PHASE_END;
		select_idle->set_available_phase(static_cast<Phase>(phases));
		select_idle->set_can_shuffle(read<uint8_t>(data, "can shuffle") != 0U);
		break;
	}
	case MSG_ANNOUNCE_NUMBER:
	{
		auto* select_number = create_request()->mutable_select_number();
		select_number->set_count(1);
		auto const count = read<CSCount>(data, "count");
		for(CSCount i = 0; i < count; i++)
			select_number->add_numbers(read<uint64_t>(data));
		break;
	}
	case MSG_SELECT_POSITION:
	{
		auto* select_position = create_request()->mutable_select_position();
		select_position->set_count(1);
		select_position->set_code(read<CCode>(data, "card code"));
		select_position->set_position(read_pos<CSPos>(data));
		break;
	}
	case MSG_ANNOUNCE_RACE:
	{
		auto* select_race = create_request()->mutable_select_race();
		select_race->set_count(read<CSCount>(data, "count"));
		select_race->set_race(read_race(data));
		break;
	}
	case MSG_ROCK_PAPER_SCISSORS:
	{
		create_request()->set_select_rock_paper_scissors(true);
		break;
	}
	case MSG_SELECT_CHAIN:
	{
		auto* select_to_chain = create_request()->mutable_select_to_chain();
		const bool triggering = (read<uint8_t>(data, "spe_count") & 0x7FU) > 0U;
		select_to_chain->set_triggering(triggering);
		select_to_chain->set_forced(read<uint8_t>(data, "forced") != 0U);
		skip(data, 8U, "timing hints");
		{ // Activable cards
			auto const count = read<CCount>(data, "number of cards");
			for(CCount i = 0; i < count; i++)
			{
				auto& card = *select_to_chain->add_activable_cards();
				read<CCode>(data, "skipped card code");
				read_loc_info(data, *card.mutable_place());
				read_effect(data, *card.mutable_effect());
				skip(data, 1U, "normal_resolve_reset");
			}
		}
		break;
	}
	case MSG_SELECT_EFFECTYN:
	{
		auto* select_yes_no = create_request()->mutable_select_yes_no();
		select_yes_no->set_code(read<CCode>(data, "card code"));
		read_loc_info(data, *select_yes_no->mutable_place());
		read_effect(data, *select_yes_no->mutable_effect());
		break;
	}
	case MSG_SELECT_YESNO:
	{
		auto* select_yes_no = create_request()->mutable_select_yes_no();
		read_effect(data, *select_yes_no->mutable_effect());
		break;
	}
	case MSG_SELECT_PLACE:
	case MSG_SELECT_DISFIELD:
	{
		auto* request = create_request();
		auto* select_zone = request->mutable_select_zone();
		select_zone->set_blocking(core_msg == MSG_SELECT_DISFIELD);
		select_zone->set_count(read<CSCount>(data, "count"));
		auto const replier = static_cast<CPlayer>(request->replier());
		unpack_zones(read<CField>(data, "zones"), replier,
		             [&]() { return select_zone->add_places(); });
		break;
	}
		/*
		 * Special messages.
		 */
	case MSG_WIN:
	{
		// Core Mitigation: See MSG_MATCH_KILL handling.
		auto* finish = create_event()->mutable_finish();
		const auto winner = read<CPlayer>(data, "who won");
		finish->set_win_reason(read<uint8_t>(data, "win reason"));
		finish->set_match_win_reason(context.get_match_win_reason());
		if(winner != 2U)
			finish->set_winner(static_cast<Controller>(winner));
		else
			finish->set_draw(true);
		result.state = EncodeOneResult::State::OK;
		break;
	}
	case MSG_HINT:
	{
		// Core Mitigation: This message does too much, it should be split into
		// smaller messages, also, in the case of select hints, the info should
		// be appended directly to the respective request message, instead.
		// TODO: properly handle this.
		read<uint8_t>(data, "skipped hint type");
		read_con(data);
		read<uint64_t>(data, "skipped hint data");
		result.state = EncodeOneResult::State::SWALLOWED;
		// 	result.state = EncodeOneResult::State::OK;
		break;
	}
	case MSG_DECK_TOP:
	{
		// Core Mitigation: Don't write reversed sequences for no reason.
		auto* place =
			create_event()->mutable_meta()->mutable_update()->add_places();
		place->set_con(read_con(data));
		place->set_loc(LOCATION_MAIN_DECK);
		auto const size = context.pile_size(get_con(*place), get_loc(*place));
		place->set_seq(size - 1U - read<CSeq>(data, "sequence"));
		place->set_oseq(OSEQ_INVALID);
		read<CCode>(data, "skipped card code");
		read<CPos>(data, "skipped position (current)");
		result.state = EncodeOneResult::State::OK;
		break;
	}
	case MSG_MOVE:
	{
		// Core Mitigation: This message does too much, it should be split into
		// smaller messages:
		// * A message that only perform multiple card movements.
		// * A message that is able to add multiple cards to the field.
		// * A message that is able to remove multiple cards from the field.
		// These new messages should not lose their sequencing properties.
		// Aditionally, the way overlays are handled should be reworked, since
		// right now it works on the assumption that materials are attached to
		// cards outside the field and the client is responsible for moving
		// those back and forth with the actual card who "owns them".
		Place prev{};
		Place curr{};
		read<CCode>(data, "skipped card code");
		read_loc_info(data, prev);
		read_loc_info(data, curr);
		auto const reason = read_reason(data);
		bool const is_prev_limbo = prev.loc() == LOCATION_UNSPECIFIED;
		bool const is_curr_limbo = curr.loc() == LOCATION_UNSPECIFIED;
		bool const is_prev_not_material = prev.oseq() < 0;
		bool const is_curr_not_material = curr.oseq() < 0;
		if(is_prev_limbo && is_curr_limbo)
		{
			// Corrupted move (core bug). Only handled because of older replays.
			// NOTE: fix
			// https://github.com/edo9300/ygopro-core/commit/36f2139582b
			result.state = EncodeOneResult::State::SWALLOWED;
		}
		else if(is_prev_limbo)
		{
			// Adding a card.
			auto* card = create_event()->mutable_card();
			card->set_reason(reason);
			*card->mutable_add()->add_places() = curr;
			result.state = EncodeOneResult::State::OK;
		}
		else if(is_curr_limbo)
		{
			// Removing a card.
			if(!is_pile(prev) && context.has_xyz_mat(prev))
				context.xyz_left(prev, Place{});
			auto* card = create_event()->mutable_card();
			card->set_reason(reason);
			*card->mutable_remove()->add_places() = prev;
			result.state = EncodeOneResult::State::OK;
		}
		else if(!is_curr_not_material && is_pile(curr))
		{
			// Attaching outside field.
			context.xyz_mat_defer(prev);
			result.state = EncodeOneResult::State::SWALLOWED;
		}
		else if(is_curr_not_material && !is_pile(curr))
		{
			// Moving to field.
			auto* card = create_event()->mutable_card();
			card->set_reason(reason);
			// Add operations for deferred attachments (if any).
			int32_t i = 0U;
			for(auto const& def : context.take_deferred_xyz_mat())
			{
				auto const oseq = i++;
				// Skip no-ops.
				if(def.con() == curr.con() && def.loc() == curr.loc() &&
				   def.seq() == curr.seq() && def.oseq() == oseq)
					continue;
				auto* op = card->mutable_move()->add_ops();
				auto* old_place = op->mutable_old_place();
				auto* new_place = op->mutable_new_place();
				*old_place = def;
				*new_place = curr;
				new_place->set_oseq(oseq);
			}
			// Add actual card move.
			auto* op = card->mutable_move()->add_ops();
			*op->mutable_old_place() = prev;
			*op->mutable_new_place() = curr;
			result.state = EncodeOneResult::State::OK;
		}
		else if(!is_prev_not_material && is_pile(prev))
		{
			// Deattaching outside field.
			auto const oseq = prev.oseq();
			auto* card = create_event()->mutable_card();
			auto* op = card->mutable_move()->add_ops();
			card->set_reason(reason);
			auto* old_place = op->mutable_old_place();
			prev.set_oseq(OSEQ_INVALID);
			*old_place = context.get_xyz_left(prev);
			old_place->set_oseq(oseq);
			*op->mutable_new_place() = curr;
			result.state = EncodeOneResult::State::OK;
		}
		else
		{
			// Every other "regular" case.
			auto* card = create_event()->mutable_card();
			auto* op = card->mutable_move()->add_ops();
			card->set_reason(reason);
			*op->mutable_old_place() = prev;
			*op->mutable_new_place() = curr;
			// Card with xyz materials left the field.
			if(!is_pile(prev) && is_pile(curr) && context.has_xyz_mat(prev))
				context.xyz_left(curr, prev);
			result.state = EncodeOneResult::State::OK;
		}
		break;
	}
	case MSG_SWAP_GRAVE_DECK:
	{
		// Core Mitigation: Implement by using pile splices.
		auto const con = read_con(data);
		auto const extra_deck_insert_pos = read<CCount>(data, "extra_p_count");
		auto const buffer_size = read<CCount>(data, "extra_buffer_size");
		auto const gy_size = context.pile_size(con, LOCATION_GRAVEYARD);
		auto* splice = create_event()->mutable_pile()->mutable_splice();
		// Splice graveyard to the bottom of the main deck.
		if(gy_size > 0U)
		{
			auto* op = splice->add_ops();
			auto* from = op->mutable_from();
			from->set_con(con);
			from->set_loc(LOCATION_GRAVEYARD);
			from->set_seq(0U);
			from->set_oseq(OSEQ_INVALID);
			op->set_count(gy_size);
			auto* to = op->mutable_to();
			to->set_con(con);
			to->set_loc(LOCATION_MAIN_DECK);
			to->set_seq(0U);
			to->set_oseq(OSEQ_INVALID);
		}
		// Splice deck minus newly added graveyard cards to the graveyard.
		auto const deck_size = context.pile_size(con, LOCATION_MAIN_DECK);
		if(deck_size > 0U)
		{
			auto* op = splice->add_ops();
			auto* from = op->mutable_from();
			from->set_con(con);
			from->set_loc(LOCATION_MAIN_DECK);
			from->set_seq(gy_size);
			from->set_oseq(OSEQ_INVALID);
			op->set_count(deck_size);
			auto* to = op->mutable_to();
			to->set_con(con);
			to->set_loc(LOCATION_GRAVEYARD);
			to->set_seq(0U);
			to->set_oseq(OSEQ_INVALID);
		}
		// Splice extra deck cards from the bottom of the main deck to the top
		// of the extra deck BUT before the face-up pendulums.
		CCount splice_seq = 0U;
		CCount splice_size = 0U;
		CCount sent_to_extra = 0U;
		auto add_op = [&]()
		{
			auto* op = splice->add_ops();
			auto* from = op->mutable_from();
			from->set_con(con);
			from->set_loc(LOCATION_MAIN_DECK);
			from->set_seq(splice_seq - sent_to_extra);
			from->set_oseq(OSEQ_INVALID);
			op->set_count(splice_size);
			auto* to = op->mutable_to();
			to->set_con(con);
			to->set_loc(LOCATION_EXTRA_DECK);
			to->set_seq(extra_deck_insert_pos + sent_to_extra);
			to->set_oseq(OSEQ_INVALID);
			op->set_reverse(false);
		};
		for(CCount i = 0U; i < gy_size; i++)
		{
			// NOLINTNEXTLINE: Check if the nth bit of the buffer is set (true).
			if(!!(data[i / 8U] & (1U << (i % 8U))))
			{
				if(splice_size++ == 0U)
					splice_seq = i;
			}
			else if(splice_size > 0U)
			{
				add_op();
				// Reset state.
				sent_to_extra += splice_size;
				splice_size = 0U;
			}
		}
		// In case the last loop iteration ended with bit checking condition
		// being true.
		if(splice_size > 0U)
			add_op();
		skip(data, buffer_size, "extra deck buffer bit's length");
		result.state = EncodeOneResult::State::OK;
		break;
	}
	case MSG_DRAW:
	{
		// Core Mitigation: Implement as splices OR write required pile sizes.
		auto const con = read_con(data);
		auto const count = read<CCount>(data, "number of draws");
		for(CCount i = 0; i < count; i++)
		{
			read<CCode>(data, "skipped card code");
			read<CPos>(data, "skipped position (current)");
		}
		auto* pile = create_event()->mutable_pile();
		pile->set_reason(REASON_DRAW);
		auto* op = pile->mutable_splice()->add_ops();
		{
			auto* from = op->mutable_from();
			from->set_con(con);
			from->set_loc(LOCATION_MAIN_DECK);
			from->set_seq(context.pile_size(con, get_loc(*from)) - count);
			from->set_oseq(OSEQ_INVALID);
		}
		op->set_count(count);
		{
			auto* to = op->mutable_to();
			to->set_con(con);
			to->set_loc(LOCATION_HAND);
			to->set_seq(context.pile_size(con, get_loc(*to)));
			to->set_oseq(OSEQ_INVALID);
		}
		op->set_reverse(true);
		result.state = EncodeOneResult::State::OK;
		break;
	}
	case MSG_REVERSE_DECK:
	{
		// Core Mitigation: Implement splicing.
		auto* pile = create_event()->mutable_pile();
		pile->set_reason(REASON_EFFECT);
		auto* splice = pile->mutable_splice();
		auto add_op = [&](Controller con)
		{
			auto* op = splice->add_ops();
			{
				auto* from = op->mutable_from();
				from->set_con(con);
				from->set_loc(LOCATION_MAIN_DECK);
				from->set_seq(0U);
				from->set_oseq(OSEQ_INVALID);
			}
			op->set_count(context.pile_size(con, LOCATION_MAIN_DECK));
			{
				auto* to = op->mutable_to();
				to->set_con(con);
				to->set_loc(LOCATION_MAIN_DECK);
				to->set_seq(0U);
				to->set_oseq(OSEQ_INVALID);
			}
			op->set_reverse(true);
		};
		add_op(CONTROLLER_0);
		add_op(CONTROLLER_1);
		result.state = EncodeOneResult::State::OK;
		break;
	}
	case MSG_MATCH_KILL:
	{
		// Core Mitigation: Just write this with MSG_WIN directly.
		context.match_win_reason(read<uint32_t>(data, "match win reason"));
		result.state = EncodeOneResult::State::SWALLOWED;
		break;
	}
	case MSG_CARD_HINT:
	{
		// Core Mitigation: These should be queries.
		// TODO: properly handle this.
		enum CardHintType : uint8_t
		{
			CARD_HINT_TYPE_TURN = 1U,
			CARD_HINT_TYPE_CARD = 2U,
			CARD_HINT_TYPE_RACE = 3U,
			CARD_HINT_TYPE_ATTRIBUTE = 4U,
			CARD_HINT_TYPE_NUMBER = 5U,
			CARD_HINT_TYPE_DESC_ADD = 6U,
			CARD_HINT_TYPE_DESC_REMOVE = 7U,
		};
		data += CORE_LOC_INFO_SIZE;
		/*auto type = */ read<CardHintType>(data, "card hint type");
		/*auto value = */ read<uint64_t>(data, "card hint value");
		result.state = EncodeOneResult::State::SWALLOWED;
		break;
	}
		/*
		 * Swallowed messages.
		 */
	case MSG_RETRY:
	{
		set_state_swallowed();
		break;
	}
	case MSG_SUMMONED:
	case MSG_SPSUMMONED:
	case MSG_FLIPSUMMONED:
	{
		set_state_swallowed();
		break;
	}
	case MSG_CHAIN_SOLVING:
	case MSG_CHAINED:
	case MSG_CHAIN_NEGATED:
	case MSG_CHAIN_DISABLED: // TODO: Put these into Event.Meta.ChainStatus.
	{
		set_state_swallowed();
		read<uint8_t>(data, "chain num");
		break;
	}
	case MSG_CHAIN_END:
	{
		set_state_swallowed();
		break;
	}
	case MSG_BATTLE:
	{
		set_state_swallowed();
		// atk + def + "flag"
		constexpr size_t EXTRA_INFO =
			sizeof(int32_t) + sizeof(int32_t) + sizeof(uint8_t);
		skip(data, CORE_LOC_INFO_SIZE, "attacker place");
		skip(data, EXTRA_INFO, "attacker extra info");
		skip(data, CORE_LOC_INFO_SIZE, "attack_target place");
		skip(data, EXTRA_INFO, "attack_target extra info");
		break;
	}
	case MSG_EQUIP:
	{
		set_state_swallowed();
		skip(data, CORE_LOC_INFO_SIZE, "equip card");
		skip(data, CORE_LOC_INFO_SIZE, "equip target");
		break;
	}
	case MSG_ATTACK_DISABLED:
	{
		set_state_swallowed();
		break;
	}
	case MSG_DAMAGE_STEP_START:
	case MSG_DAMAGE_STEP_END:
	{
		set_state_swallowed();
		break;
	}
	case MSG_MISSED_EFFECT:
	{
		set_state_swallowed();
		// TODO: Improve message in the core.
		skip(data, CORE_LOC_INFO_SIZE, "who missed");
		read<CCode>(data, "skipped card code");
		break;
	}
	case MSG_AI_NAME:
	{
		set_state_swallowed();
		skip(data, read<uint16_t>(data, "name length") + 1U, "ai name");
		break;
	}
	/*
	 * Unknown / Invalid core message.
	 */
	default:
		log("unknown");
		result.state = EncodeOneResult::State::UNKNOWN;
	}
	result.bytes_read = static_cast<size_t>(data - sentry);
	log("bytes read: ", result.bytes_read);
	return result;
}

} // namespace YGOpen::Codec::Edo9300::OCGCore
