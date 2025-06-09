/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_CARD_HPP
#define YGOPEN_CLIENT_CARD_HPP
#include <ygopen/detail/config.hpp>

namespace YGOpen::Client
{

#ifdef YGOPEN_HAS_CONCEPTS
template<typename T>
concept CardTraits = requires(T){
#define X(NAME, Name, name, value) \
	requires std::default_initializable<typename T::Name##Type>;
#define EXPAND_ARRAY_LIKE_QUERIES
#define EXPAND_SEPARATE_LINK_DATA_QUERIES
#include <ygopen/client/queries.inl>
#undef EXPAND_SEPARATE_LINK_DATA_QUERIES
#undef EXPAND_ARRAY_LIKE_QUERIES
#undef X
};
#endif // YGOPEN_HAS_CONCEPTS

template<YGOPEN_CONCEPT(CardTraits)>
class BasicCard
{
#define X(NAME, Name, name, value)                                        \
public:                                                                   \
	using Name##Type = typename CardTraits::Name##Type;                   \
	[[nodiscard]] auto name() const noexcept -> Name##Type const&         \
	{                                                                     \
		return name##_;                                                   \
	}                                                                     \
	[[nodiscard]] auto name() noexcept -> Name##Type& { return name##_; } \
                                                                          \
private:                                                                  \
	Name##Type name##_{};
#define EXPAND_ARRAY_LIKE_QUERIES
#define EXPAND_SEPARATE_LINK_DATA_QUERIES
#include <ygopen/client/queries.inl>
#undef EXPAND_SEPARATE_LINK_DATA_QUERIES
#undef EXPAND_ARRAY_LIKE_QUERIES
#undef X

public:
	explicit constexpr BasicCard() noexcept = default;

	inline ~BasicCard() noexcept = default;
	BasicCard(BasicCard const&) = delete;
	constexpr BasicCard(BasicCard&&) noexcept = default;
	auto operator=(BasicCard const&) -> BasicCard& = delete;
	constexpr auto operator=(BasicCard&&) noexcept -> BasicCard& = default;
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_CARD_HPP
