/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_VALUE_WRAPPERS_HPP
#define YGOPEN_CLIENT_VALUE_WRAPPERS_HPP
#include <list>
#include <memory_resource>

namespace YGOpen::Client
{

// Simple zero-cost abstraction over a single trivial type.
template<typename T>
class Value
{
public:
	using ValueType = T;

	explicit constexpr Value() noexcept : value_() {}

	[[nodiscard]] constexpr auto get() const noexcept -> T const&
	{
		return value_;
	}

	constexpr auto set(T const& v) noexcept -> void { value_ = v; }

private:
	T value_;
};

// Abstraction that keeps a record of added values and is able to go back
// to them for inspection only. Recommended to use with a monotonic resource.
template<typename T>
class Sequential
{
public:
	using ValueType = T;

	explicit constexpr Sequential() noexcept
		: values_(1U, std::pmr::get_default_resource())
		, current_value_(values_.cbegin())
	{}

	explicit constexpr Sequential(std::pmr::memory_resource* alloc) noexcept
		: values_(1U, alloc), current_value_(values_.cbegin())
	{}

	[[nodiscard]] constexpr auto get() const noexcept -> T const&
	{
		return *current_value_;
	}

	constexpr auto set(T const& v) noexcept -> void
	{
		if(current_value_ == --values_.cend())
			values_.emplace_back(v);
		current_value_++;
	}

	constexpr auto dec() noexcept -> void { current_value_--; }

private:
	typename std::pmr::list<T> values_;
	typename std::pmr::list<T>::const_iterator current_value_;
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_VALUE_WRAPPERS_HPP
