/*
 * Copyright (c) 2022, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_BIT_HPP
#define YGOPEN_BIT_HPP
#ifdef __cpp_lib_bitops
#include <bit>
#endif
#include <type_traits>

namespace YGOpen::Bit
{

// Counts the number of 1 bits in `x`, also called "Hamming weight".
template<typename T>
constexpr auto popcnt(T x) noexcept -> unsigned
{
	static_assert(std::is_unsigned_v<T>);
#if defined(__cpp_lib_bitops)
	return std::popcount(x);
#elif defined(__GNUC__) || defined(__clang__)
	if constexpr(std::numeric_limits<T>::digits > 32)
		return __builtin_popcountll(x);
	else
		return __builtin_popcount(x);
#else
	unsigned count = 0;
	while(x)
	{
		count++;
		x &= x - 1;
	}
	return count;
#endif
}

// Chooses at most `count` 1 bits from `x`, starting from the most significant
// bit.
template<typename T>
constexpr auto choosel(unsigned count, T x) noexcept -> T
{
	static_assert(std::is_unsigned_v<T>);
	T chosen = 0U;
	unsigned i = std::numeric_limits<T>::digits;
	while(i != 0U)
	{
		chosen |= x & (1U << (i - 1U));
		if(popcnt(chosen) >= count)
			break;
		i--;
	}
	return chosen;
}

// Chooses at most `count` 1 bits from `x`, starting from the least significant
// bit.
template<typename T>
constexpr auto chooser(unsigned count, T x) noexcept -> T
{
	static_assert(std::is_unsigned_v<T>);
	T chosen = 0U;
	unsigned i = 0;
	while(i <= std::numeric_limits<T>::digits && popcnt(chosen) < count)
	{
		chosen |= x & (1U << i);
		i++;
	}
	return chosen;
}

} // namespace YGOpen::Bit

#endif // YGOPEN_BIT_HPP
