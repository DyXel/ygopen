/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_VALUE_WRAPPERS_HPP
#define YGOPEN_CLIENT_VALUE_WRAPPERS_HPP
#include <list>

namespace YGOpen::Client
{

// Simple zero-cost abstraction over a single trivial type.
template<typename T>
class Value
{
public:
	using ValueType = T;

	explicit constexpr Value() noexcept : value_() {}

	constexpr auto get() const noexcept -> const T& { return value_; }

	constexpr auto set(T v) noexcept -> void { value_ = std::move(v); }

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

	template<typename Allocator>
	explicit constexpr Sequential(Allocator* alloc) noexcept
		: values_(1U, alloc), current_value_(values_.cbegin())
	{}

	constexpr auto get() const noexcept -> const T& { return *current_value_; }

	constexpr auto set(T v) noexcept -> void
	{
		if(current_value_ == --values_.cend())
			values_.emplace_back(std::move(v));
		current_value_++;
	}

	constexpr auto dec() noexcept -> void { current_value_--; }

private:
	typename std::pmr::list<T> values_;
	typename std::pmr::list<T>::const_iterator current_value_;
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_VALUE_WRAPPERS_HPP
