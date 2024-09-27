/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#error "This file is for documentation only, do not use for compilation!"
// NOTE: Stuff within Detail namespaces is not included, this shows only PUBLIC
//       interfaces, classes and APIs. [[nodiscard]], constexpr and noexcept are
//       skipped because they are mostly an eye-sore.

namespace YGOpen
{

namespace Bit
{

// from #include <ygopen/bit.hpp>
template<typename T>
auto popcnt(T x) -> unsigned;
template<typename T>
auto choosel(unsigned count, T x) -> T;
template<typename T>
auto chooser(unsigned count, T x) -> T;

} // namespace Bit

namespace Client
{

// from #include <ygopen/client/board.hpp>
template<typename Frame, template<typename Wrapped> typename Wrapper>
class BasicBoard;

// from #include <ygopen/client/card.hpp>
template<template<typename Wrapped> typename Wrapper>
struct BasicCard;

// from #include <ygopen/client/frame.hpp>
auto zone_seq_lim(YGOpen::Duel::Location loc) -> size_t;
template<typename Card, typename CardBuilder = Detail::DefaultBuilder<Card>>
class BasicFrame;

// from #include <ygopen/client/frame_limbo.hpp>
template<typename Card, typename CardBuilder = Detail::DefaultBuilder<Card>>
class LimboFrame : public BasicFrame<Card, CardBuilder>;

// from <ygopen/client/parse_event.hpp>
template<typename Board>
auto parse_event(Board& board, Proto::Duel::Msg::Event const& event) -> void;

// from <ygopen/client/parse_query.hpp>
enum class QueryCacheHit : unsigned;
auto operator&(QueryCacheHit lhs, QueryCacheHit rhs) -> QueryCacheHit;
auto operator~(QueryCacheHit rhs) -> QueryCacheHit;
auto operator|=(QueryCacheHit& lhs, QueryCacheHit rhs) -> QueryCacheHit&;
auto operator!(QueryCacheHit rhs) -> bool;
template<bool use_cache = false, typename Frame>
auto parse_query(Frame& frame, Proto::Duel::Msg::Query const& query)
	-> QueryCacheHit;

// from #include <ygopen/client/undo_parse_event.hpp>
template<typename Board>
auto undo_parse_event(Board& board, Proto::Duel::Msg const& msg);

// from #include <ygopen/client/value_wrappers.hpp>
template<typename T>
class Value;
template<typename T>
class Sequential;

} // namespace Client

namespace Duel
{

// from #include <ygopen/duel/constants_fwd.hpp>
// NOTE: Concrete types defined in <ygopen/duel/constants/*.hpp>
enum Attribute : uint32_t;
enum Controller : int;
enum LinkArrow : uint32_t;
enum Location : uint32_t;
enum Phase : uint32_t;
enum Position : uint32_t;
enum Race : uint64_t;
enum Reason : uint64_t;
enum RockPaperScissors : int;
enum Status : uint32_t;
enum Type : uint32_t;

// from #include <ygopen/duel/constants/location.hpp>
auto is_empty(Location loc) -> bool;
auto is_pile(Location loc) -> bool;
auto is_zone(Location loc) -> bool;

} // namespace Duel

namespace Server
{

// from #include <ygopen/server/room.hpp>
template<typename Client, typename DeckValidator, typename CoreDuelFactory>
class BasicRoom;

} // namespace Server

namespace Codec
{

// from #include <ygopen/codec/encode_common.hpp>
struct EncodeOneQueryResult;
struct EncodeOneResult;
class IEncodeContext;

namespace Edo9300::OCGCore
{

// from #include <ygopen/codec/edo9300_ocgcore_decode.hpp>
auto decode_one_answer(Proto::Duel::Msg_Request const& request,
                       Proto::Duel::Answer const& answer,
                       std::vector<uint8_t>& out) -> void;

// from #include <ygopen/codec/edo9300_ocgcore_encode.hpp>
auto encode_one_query(uint8_t const* data, Proto::Duel::Msg_Query_Data& q)
	-> EncodeOneQueryResult;
auto encode_one(google::protobuf::Arena& arena, IEncodeContext& context,
                uint8_t const* data) -> EncodeOneResult;

} // namespace Edo9300::OCGCore

} // namespace Codec

namespace Proto
{

// Generated from "ygopen/proto/banlist.proto"
class Banlist;
class BanlistError;
// from #include <ygopen/proto/banlist.hpp>
auto validate_banlist(Banlist const& banlist, BanlistError& error) -> bool;

// Generated from "ygopen/proto/deck.proto"
enum DeckType;
class Deck;
class DeckLimits;
class DeckError;
// from #include <ygopen/proto/deck.hpp>
auto validate_deck_excessive_copies(Deck const& deck,
                                    DeckError::ExcessiveCopies& error) -> bool;
auto validate_deck_limits(Deck const& deck, DeckLimits const& limits,
                          DeckError::OutOfLimit& error);

// Generated from "ygopen/proto/replay.proto"
class Replay;

namespace Duel
{

// Generated from "ygopen/proto/duel_answer.proto"
class Answer;

// Generated from "ygopen/proto/duel_data.proto"
enum ChainStatus;
class Chain;
class Counter;
class Effect;
class Place;
// from #include <ygopen/proto/duel/data.hpp>
int OSEQ_INVALID;
struct PlaceLess;
inline auto get_con(Place const& place) -> auto;
inline auto get_loc(Place const& place) -> auto;
inline auto is_empty(Place const& place) -> bool;
inline auto is_pile(Place const& place) -> bool;
inline auto is_zone(Place const& place) -> bool;

// Generated from "ygopen/proto/duel_msg.proto"
class Msg;

} // namespace Duel

namespace Room
{

// Generated from "ygopen/proto/room.proto"
enum State;
class Duelist;
class Options;
class Event;
class Signal;

} // namespace Room

// Generated from "ygopen/proto/server_msg.proto"
class ServerMsg;

// Generated from "ygopen/proto/client_msg.proto"
class ClientMsg;

// Generated from "ygopen/proto/user.proto"
class User;
// from #include <ygopen/proto/user.hpp>
auto validate_user_id(char const* data, size_t size) -> bool;
auto validate_user_name(std::string_view name) -> bool;

} // namespace Proto

} // namespace YGOpen
