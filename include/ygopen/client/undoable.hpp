/*
 * Copyright (c) 2025, Dylam De La Torre <dyxel04@gmail.com>
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

	[[nodiscard]] constexpr operator ValueType const&() const noexcept
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
	}

	template<typename U = ValueType>
	constexpr auto operator==(U&& v) const noexcept -> bool
	{
		return *current_ == v;
	}

	constexpr auto undo() noexcept -> void
	{
		assert(current_ != values_.cbegin());
		current_--;
	}

	constexpr auto erase_history() noexcept -> void
	{
		assert(false); // TODO
	}

	// FIXME: Too lazy to do something else, should probably be optimized
	//        and/or be its own template type or something...

	template<typename... Args>
	constexpr auto assign(Args&&... args) noexcept -> void
	{
		if(current_ == --values_.cend())
			values_.emplace_back().assign(std::forward<Args>(args)...);
		current_++;
	}

	template<typename... Args>
	constexpr auto push_back(Args&&... args) noexcept -> void
	{
		assert(false); // TODO
	}

	constexpr auto pop_back() noexcept -> void
	{
		assert(false); // TODO
	}

	template<typename... Args>
	constexpr auto undo_push_back(Args&&... args) noexcept -> void
	{
		assert(false); // TODO
	}

	constexpr auto undo_pop_back() noexcept -> void
	{
		assert(false); // TODO
	}

private:
	ContainerType values_;
	typename ContainerType::const_iterator current_;
};

} // namespace YGOpen::Client

#endif // YGOPEN_CLIENT_UNDOABLE_HPP
