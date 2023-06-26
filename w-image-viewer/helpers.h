#pragma once

#include "pch.h"

#ifdef NDEBUG
#define wiv_assert(keep, discard_ifndebug) (keep)
#else
#define wiv_assert(keep, discard_ifndebug) (assert(keep discard_ifndebug))
#endif

//safe floating comparations
//

inline constexpr double WIV_DBL_EPS{ 1e-15 };
inline constexpr float WIV_FLT_EPS{ 1e-6f };

inline bool is_equal(double a, double b) noexcept
{
	return std::abs(a - b) < WIV_DBL_EPS;
}

inline bool is_equal(float a, float b) noexcept
{
	return std::abs(a - b) < WIV_FLT_EPS;
}

inline bool is_zero(double a) noexcept
{
	return std::abs(a) < WIV_DBL_EPS;
}

inline bool is_zero(float a) noexcept
{
	return std::abs(a) < WIV_FLT_EPS;
}

inline bool is_not_zero(double a) noexcept
{
	return std::abs(a) >= WIV_DBL_EPS;
}

inline bool is_not_zero(float a) noexcept
{
	return std::abs(a) >= WIV_FLT_EPS;
}

//

template<typename T>
constexpr T get_ratio(auto a, auto b) noexcept
{
	return static_cast<T>(a) / static_cast<T>(b);
}
