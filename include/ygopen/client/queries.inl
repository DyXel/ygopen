// https://en.wikipedia.org/wiki/X_macro
// Prototype: X(NAME, Name, name, value)
//  NAME: SCREAMING_SNAKE_CASE name the query has in the core minus `QUERY_`
//  Name: CamelCase name of this query (based on protobuf message name)
//  name: snake_case name the query has in protobuf message
//  value: flag value the query has in the core
// Optional expansions:
//  EXPAND_ARRAY_LIKE_QUERIES
//  EXPAND_OLD_LINK_QUERY
//  EXPAND_END_MARKER_QUERY
//  EXPAND_SEPARATE_LINK_DATA_QUERIES
// Values from https://github.com/edo9300/ygopro-core/blob/master/common.h
X(CODE, Code, code, 0x1)
X(POSITION, Position, position, 0x2)
X(ALIAS, Alias, alias, 0x4)
X(TYPE, Type, type, 0x8)
X(LEVEL, Level, level, 0x10)
X(RANK, XyzRank, xyz_rank, 0x20)
X(ATTRIBUTE, Attribute, attribute, 0x40)
X(RACE, Race, race, 0x80)
X(ATTACK, Atk, atk, 0x100)
X(DEFENSE, Def, def, 0x200)
X(BASE_ATTACK, BaseAtk, base_atk, 0x400)
X(BASE_DEFENSE, BaseDef, base_def, 0x800)
// X(REASON, Reason, reason, 0x1000) // NOTE: Unused
// X(REASON_CARD, ReasonCard, reason_card, 0x2000) // NOTE: Unused
X(EQUIP_CARD, EquippedTo, equipped_to, 0x4000)
#ifdef EXPAND_ARRAY_LIKE_QUERIES
X(TARGET_CARD, Targets, targets, 0x8000)
// X(OVERLAY_CARD, OverlayCard, overlay_card, 0x10000) // NOTE: Special case
X(COUNTERS, Counters, counters, 0x20000)
#endif // EXPAND_ARRAY_LIKE_QUERIES
X(OWNER, Owner, owner, 0x40000)
X(STATUS, Status, status, 0x80000)
X(IS_PUBLIC, IsPublic, is_public, 0x100000)
X(LSCALE, PendLScale, pend_l_scale, 0x200000)
X(RSCALE, PendRScale, pend_r_scale, 0x400000)
#ifdef EXPAND_OLD_LINK_QUERY
X(LINK, Link, link, 0x800000)
#endif // EXPAND_OLD_LINK_QUERY
X(IS_HIDDEN, IsHidden, is_hidden, 0x1000000)
X(COVER, Cover, cover, 0x2000000)
#ifdef EXPAND_END_MARKER_QUERY
X(END, End, end, 0x80000000)
#endif // EXPAND_END_MARKER_QUERY
#ifdef EXPAND_SEPARATE_LINK_DATA_QUERIES
X(LINK_RATE, LinkRate, link_rate, 0x100000000)
X(LINK_ARROW, LinkArrow, link_arrow, 0x200000000)
#endif // EXPAND_SEPARATE_LINK_DATA_QUERIES
