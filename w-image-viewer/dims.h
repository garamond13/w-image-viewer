#pragma once

struct Dims
{
	template<typename T>
	constexpr T get_width() const noexcept
	{
		return static_cast<T>(width);
	}

	template<typename T>
	constexpr T get_height() const noexcept
	{
		return static_cast<T>(height);
	}

	int width;
	int height;
};
