#pragma once

#include "helpers.h"

template<typename T>
struct Range
{
	bool operator==(const Range<T>& other) const noexcept
	{
		if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
			return is_equal(lower, other.lower) && is_equal(upper, other.upper);
		else
			return lower == other.lower && upper == other.upper;
	}

	constexpr bool is_inrange(auto val) const noexcept
	{
		return lower <= static_cast<T>(val) && static_cast<T>(val) <= upper;
	}

	constexpr void clamp() noexcept
	{
		if (lower > upper)
			lower = upper;
	}

	T lower;
	T upper;
};