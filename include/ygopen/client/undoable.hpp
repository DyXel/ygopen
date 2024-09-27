/*
 * Copyright (c) 2024, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#ifndef YGOPEN_CLIENT_UNDOABLE_HPP
#define YGOPEN_CLIENT_UNDOABLE_HPP
#include <list>

namespace YGOpen::Client
{

// Abstraction over a regular value that keeps a record of added ones and is
// able to go back to them for inspection.
template<typename T, typename Container = std::list<T>>
class BasicUndoable
{
public:
	using ValueType = T;
	using ContainerType = Container;

	explicit constexpr BasicUndoable() noexcept
		: values_(1U), current_(values_.cbegin())
	{}

	[[nodiscard]] constexpr explicit operator ValueType() const noexcept
	{
		return *current_;
	}

	template<typename U = ValueType>
	constexpr auto operator=(U&& v) noexcept -> BasicUndoable&
	{
		if(current_ == --values_.cend())
			values_.emplace_back(std::forward<U>(v));
		current_++;
		return *this;
	};

	constexpr auto undo() noexcept -> void
	{
		assert(current_ != values_.cbegin());
		current_--;
	}

	// FIXME: Too lazy to do something else, should probably be optimized
	//        and/or be its own template type or something...

	template<typename ...Args>
	constexpr auto assign(Args&&... args) noexcept -> auto
	{
		if(current_ == --values_.cend())
			values_.emplace_back().assign(std::forward<Args>(args)...);
		current_++;
		return *this;
	};

	template<typename ...Args>
	constexpr auto push_back(Args&&... args) noexcept -> auto
	{
		// TODO
	};

	constexpr auto pop_back() noexcept -> auto
	{
		// TODO
	};

	template<typename ...Args>
	constexpr auto undo_assign(Args&&... args) noexcept -> auto
	{
		// TODO
	};

	template<typename ...Args>
	constexpr auto undo_push_back(Args&&... args) noexcept -> auto
	{
		// TODO
	};

	constexpr auto undo_pop_back() noexcept -> auto
	{
		// TODO
	};

private:
	ContainerType values_;
	typename ContainerType::const_iterator current_;
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_UNDOABLE_HPP
