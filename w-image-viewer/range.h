#pragma once

#include "helpers.h"

template<typename T>
struct Right_open_range
{
	constexpr bool is_inrange(auto val) const noexcept
	{
		return lower <= static_cast<T>(val) && static_cast<T>(val) < upper;
	}

	constexpr bool is_valid() const noexcept
	{
		if (lower <= upper)
			return true;
		return false;
	}

	constexpr bool is_overlapping(Right_open_range<T>& other) const noexcept
	{
		return (lower < other.upper && upper > other.lower) || (lower == other.lower);
	}

	T lower;
	T upper;
};